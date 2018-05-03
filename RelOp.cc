#include "RelOp.h"
#include "BigQDistinct.h"
#include "CrossJoin.h"
#include "TempFileGen.h"

unordered_set<long long int> TempFileGen::filenames_;

mutex TempFileGen::mutex_;

void SelectPipe::Run() {
  thread_ = std::thread([this]() {
    ComparisonEngine ce;
    Record rec;
    while (in_pipe.Remove(&rec)) {
      if (ce.Compare(&rec, &literal, &sel_op)) {
        out_pipe.Insert(&rec);
      }
    }
    out_pipe.ShutDown();
  });
}

void SelectFile::Run() {
  thread_ = std::thread([this]() {
    ComparisonEngine ce;
    Record rec;
    while (in_file.GetNext(rec)) {
      if (ce.Compare(&rec, &literal, &sel_op)) {
        out_pipe.Insert(&rec);
      }
    }
    out_pipe.ShutDown();
  });
}

void Project::Run() {
  thread_ = std::thread([this]() {
    ComparisonEngine ce;
    Record rec;
    while (in_pipe.Remove(&rec)) {
      rec.Project(keep_me.data(), num_atts_output, num_atts_input);
      out_pipe.Insert(&rec);
    }
    out_pipe.ShutDown();
  });
}

void WriteOut::Run() {
  thread_ = std::thread([this]() {
    int num_records = 0;
    Record rec;
    while (in_pipe.Remove(&rec)) {
      out_ << rec.ToString(&my_schema) << "\n";
      num_records++;
    }
    out_ << "\nTotal Records: " << num_records << "\n";
  });
}

void DuplicateRemoval::Run() {
  thread_ = std::thread([this]() {
    OrderMaker order(&my_schema);
    Pipe bigq_out_pipe(1000);
    Q_ = make_unique<BigQ>(in_pipe, bigq_out_pipe, order, run_len_);

    ComparisonEngine ce;
    Record prev_rec;
    Record curr_rec;
    int res = bigq_out_pipe.Remove(&prev_rec);
    if (res == 1) {
      Record temp = prev_rec;
      out_pipe.Insert(&temp);
      while (bigq_out_pipe.Remove(&curr_rec)) {
        if (ce.Compare(&prev_rec, &curr_rec, &order) != 0) {
          prev_rec = curr_rec;
          out_pipe.Insert(&curr_rec);
        }
      }
    }
    out_pipe.ShutDown();
  });
}

void Sum::Run() {
  thread_ = std::thread([this]() {
    int int_result = 0, int_temp;
    double double_result = 0.0, double_temp;
    Record rec;
    Type result_type;

    auto set_single_record = [&result_type](Record &rec, int int_result,
                                            double double_result) {
      if (result_type == Int) {
        rec.SetSingleValue(int_result);
      } else if (result_type == Double) {
        rec.SetSingleValue(double_result);
      }
    };

    if (distinct_) {
      Record cop;
      int res = in_pipe.Remove(&cop);
      if (!res)
        return;
      result_type = compute_me.Apply(cop, int_temp, double_temp);
      set_single_record(cop, int_temp, double_temp);
      OrderMaker o;
      o.AddAtt(0, result_type);
      Pipe distinct_pipe;
      BigQDistinct distinct_q(distinct_pipe, o, run_len_);
      distinct_pipe.Insert(&cop);

      Record copy;
      while (in_pipe.Remove(&copy)) {
        Record topy;
        result_type = compute_me.Apply(copy, int_temp, double_temp);
        set_single_record(topy, int_temp, double_temp);
        distinct_pipe.Insert(&topy);
      }
      distinct_pipe.ShutDown();
      if (result_type == Int) {
        int_result = distinct_q.GetDistinctSum<int>();
      } else {
        double_result = distinct_q.GetDistinctSum<double>();
      }
    } else {
      while (in_pipe.Remove(&rec)) {
        result_type = compute_me.Apply(rec, int_temp, double_temp);
        int_result += int_temp;
        double_result += double_temp;
      }
    }
    set_single_record(rec, int_result, double_result);
    out_pipe.Insert(&rec);
    out_pipe.ShutDown();
  });
}

void GroupBy::Run() {
  thread_ = std::thread([this]() {
    Pipe bigq_out_pipe(1000);
    Q_ = make_unique<BigQ>(in_pipe, bigq_out_pipe, group_atts, run_len_);

    int int_result = 0, int_temp = 0;
    double double_result = 0.0, double_temp = 0.0;
    Type result_type = String;

    ComparisonEngine ce;
    Record prev_rec;
    Record curr_rec;

    auto add_to_pipe = [this, &result_type](Record &curr_r, Pipe &pipe) {
      int int_temp = 0;
      double double_temp = 0.0;
      result_type = compute_me.Apply(curr_r, int_temp, double_temp);
      Record rec;
      if (result_type == Int) {
        rec.SetSingleValue<int>(int_temp);
        pipe.Insert(&rec);
      } else {
        rec.SetSingleValue<double>(double_temp);
        pipe.Insert(&rec);
      }
    };

    // Inline method to apply function to curr_rec and
    // store result in int_result or double_result.
    auto add_sum = [this, &int_result, &double_result,
                    &result_type](Record &curr_r) {
      int int_temp = 0;
      double double_temp = 0.0;
      result_type = compute_me.Apply(curr_r, int_temp, double_temp);
      int_result += int_temp;
      double_result += double_temp;
    };

    int res = bigq_out_pipe.Remove(&prev_rec);
    if (res == 1) {
      add_sum(prev_rec);
    }

    std::vector<int> atts_to_keep({0});
    group_atts.GetAtts(atts_to_keep);

    // func creates a single valued record rec (Int or Double) as the
    // aggregate
    // for the group.
    // Projects the curr_rec using the given OrderMaker and merges the two
    // records together.
    auto func = [&result_type, &int_result, &double_result, &atts_to_keep,
                 this](Record &curr_r) {
      Record rec;
      if (result_type == Int) {
        rec.SetSingleValue(int_result);
      } else if (result_type == Double) {
        rec.SetSingleValue(double_result);
      }

      Record temp;
      temp.MergeRecords(&rec, &curr_r, 1, curr_r.NumAtts(), atts_to_keep.data(),
                        atts_to_keep.size(), 1);
      out_pipe.Insert(&temp);
      int_result = 0;
      double_result = 0.0;
    };

    // If distinct sum, we need to remove duplicate sum value within a group
    // This is done by creating one BigQ per group and calculating their
    // distinct sum using the BigQDistinct class.
    if (distinct_) {

      OrderMaker o;
      o.AddAtt(0, result_type);
      unique_ptr<Pipe> distinct_pipe = make_unique<Pipe>();
      unique_ptr<BigQDistinct> distinct_q =
          make_unique<BigQDistinct>(*distinct_pipe.get(), o, run_len_);
      add_to_pipe(prev_rec, *distinct_pipe.get());
      while (bigq_out_pipe.Remove(&curr_rec)) {
        if (ce.Compare(&prev_rec, &curr_rec, &group_atts) != 0) {
          distinct_pipe->ShutDown();
          if (result_type == Int) {
            int_result = distinct_q->GetDistinctSum<int>();
          } else {
            double_result = distinct_q->GetDistinctSum<double>();
          }
          func(prev_rec);
          distinct_pipe.reset(new Pipe());
          distinct_q.reset(new BigQDistinct(*distinct_pipe.get(), o, run_len_));
        }
        add_to_pipe(curr_rec, *distinct_pipe.get());
        prev_rec = curr_rec;
      }
      if (result_type == Int || result_type == Double) {
        distinct_pipe->ShutDown();
        if (result_type == Int) {
          int_result = distinct_q->GetDistinctSum<int>();
        } else {
          double_result = distinct_q->GetDistinctSum<double>();
        }
        func(prev_rec);
        distinct_pipe.reset(nullptr);
        distinct_q.reset(nullptr);
      }
    } else {

      while (bigq_out_pipe.Remove(&curr_rec)) {
        if (ce.Compare(&prev_rec, &curr_rec, &group_atts) != 0) {
          func(prev_rec);
        }
        add_sum(curr_rec);
        prev_rec = curr_rec;
      }
      if (result_type == Int || result_type == Double) {
        func(prev_rec);
      }
    }
    out_pipe.ShutDown();
  });
}

void JoinSorted(Pipe &in_pipe_L, Pipe &in_pipe_R, Pipe &out_pipe,
                OrderMaker &left_order, OrderMaker &right_order, CNF &sel_op,
                Record &literal, int run_len) {
  Record left, right;
  ComparisonEngine ce;
  CrossJoin c_join(run_len);
  int is_left = in_pipe_L.Remove(&left);
  int is_right = in_pipe_R.Remove(&right);

  while (is_left == 1 && is_right == 1) {
    int is_equal;
    while ((is_equal = ce.Compare(&left, &left_order, &right, &right_order)) &&
           is_equal != 0) {
      if (is_equal < 0) {
        if (!in_pipe_L.Remove(&left))
          return;
      } else {
        if (!in_pipe_R.Remove(&right))
          return;
      }
    }
    c_join.AddLeft(left, false /* consume */);
    Record temp;
    is_left = in_pipe_L.Remove(&temp);
    while (is_left == 1 && ce.Compare(&left, &temp, &left_order) == 0) {
      if (!c_join.AddLeft(temp)) {
        cout << "Not enough space in join buffer\n";
        exit(1);
      }
      is_left = in_pipe_L.Remove(&temp);
    }
    left = temp;
    temp = right;
    do {
      c_join.SetRight(temp);
      c_join.Exectute(out_pipe, sel_op, literal);
      is_right = in_pipe_R.Remove(&temp);
    } while (is_right == 1 && ce.Compare(&right, &temp, &right_order) == 0);

    c_join.ClearLeft();
    right = temp;
  }
}

void JoinUnsorted(Pipe &in_pipe_L, Pipe &in_pipe_R, Pipe &out_pipe, CNF &sel_op,
                  Record &literal, int run_len) {
  DBFile db_file;
  long long int filename = TempFileGen::GenFileName();
  db_file.Create(to_string(filename).c_str(), heap, nullptr);
  Record temp;
  while (in_pipe_R.Remove(&temp)) {
    db_file.Add(temp);
  }
  db_file.Close();
  db_file.Open(to_string(filename).c_str());

  CrossJoin c_join(run_len);
  while (in_pipe_L.Remove(&temp)) {
    if (!c_join.AddLeft(temp)) {
      db_file.MoveFirst();
      Record rec;
      while (db_file.GetNext(rec)) {
        c_join.SetRight(rec);
        c_join.Exectute(out_pipe, sel_op, literal);
      }
      c_join.ClearLeft();
      c_join.AddLeft(temp);
    }
  }
  c_join.Exectute(out_pipe, sel_op, literal);
  db_file.Close();
  TempFileGen::RemoveFile(filename);
}

void Join::Run() {
  thread_ = std::thread([this]() {
    OrderMaker left_order, right_order;
    int is_sorting_possible = sel_op.GetSortOrders(left_order, right_order);
    if (is_sorting_possible) {
      Pipe out_pipe_L(1000);
      Pipe out_pipe_R(1000);
      LQ_ = make_unique<BigQ>(in_pipe_L, out_pipe_L, left_order, run_len_);
      RQ_ = make_unique<BigQ>(in_pipe_R, out_pipe_R, right_order, run_len_);
      JoinSorted(out_pipe_L, out_pipe_R, out_pipe, left_order, right_order,
                 sel_op, literal, run_len_);
      Record rec;

      // Remove unused records.
      while (out_pipe_L.Remove(&rec)) {
      }
      while (out_pipe_R.Remove(&rec)) {
      }
    } else {
      JoinUnsorted(in_pipe_L, in_pipe_R, out_pipe, sel_op, literal, run_len_);
    }
    out_pipe.ShutDown();
  });
}
