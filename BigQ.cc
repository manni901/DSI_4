#include "BigQ.h"
#include "time.h"

unordered_set<long long int> BigQ::filenames_;

mutex BigQ::mutex_;

BigQ::BigQ(Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen) {
  worker_ = thread([this, &in, &out, &sortorder, runlen]() {
    // read data from in pipe sort them into runlen pages
    srand (time(NULL));
    long long int file_name = rand() % LONG_MAX;
    mutex_.lock();
    while (!filenames_.insert(rand() % LONG_MAX).second) {
      file_name = rand() % LONG_MAX;
    }
    const char *filename = to_string(file_name).c_str();
    file_.Open(0, filename);
    mutex_.unlock();

    Record rec;
    // Record rec_copy;
    Page curr_page;
    int run_length = 0;
    while (in.Remove(&rec)) {
      auto rec_copy = make_unique<Record>(rec);
      if (!curr_page.Append(&rec)) {
        curr_page.EmptyItOut();
        curr_page.Append(&rec);
        run_length++;
      }

      if (run_length == runlen) {
        SortNSave(sortorder);
        run_length = 0;
      }
      record_run_.push_back(move(rec_copy));
    }

    SortNSave(sortorder);
    run_length = 0;

    // Merge the sorted runs.
    MergeRuns(sortorder, out);

    file_.Close();

    mutex_.lock();
    remove(filename);
    filenames_.erase(file_name);
    mutex_.unlock();

    // finally shut down the out pipe
    out.ShutDown();
  });
}

BigQ::~BigQ() { worker_.join(); }

void BigQ::SortNSave(OrderMaker &sortorder) {
  Page copy_page;
  // In memory sort for records in current run.
  sort(record_run_.begin(), record_run_.end(),
       [this, &sortorder](unique_ptr<Record> &rec1, unique_ptr<Record> &rec2) {
         return ce_.Compare(rec1.get(), rec2.get(), &sortorder) < 0;
       });

  // Save records to disk in sorted order.
  off_t page_start = curr_page_index_;
  for (auto &rec : record_run_) {
    if (!copy_page.Append(rec.get())) {
      file_.AddPage(&copy_page, curr_page_index_++);
      copy_page.EmptyItOut();
      copy_page.Append(rec.get());
    }
  }
  if (copy_page.NumRecs() > 0) {
    file_.AddPage(&copy_page, curr_page_index_++);
    copy_page.EmptyItOut();
  }

  // Add run information for merging step.
  auto run = make_unique<Run>(page_start, curr_page_index_ - 1);
  runs_.push_back(move(run));
  record_run_.clear();
}

void BigQ::MergeRuns(OrderMaker &sortorder, Pipe &out) {
  // Vector to hold current record for each run.
  vector<Record> current_run_record(runs_.size());

  // lambda comparator for sorting pair<Record, int> where the int denotes
  // the index of the Run that this Record comes from.
  auto cmp = [this, &sortorder, &current_run_record](int &P1, int &P2) {
    return ce_.Compare(&current_run_record[P1], &current_run_record[P2],
                       &sortorder) >= 0;
  };

  // construct priority queue over sorted runs and dump sorted data
  // into the out pipe
  priority_queue<int, vector<int>, decltype(cmp)> Q(cmp);

  // Fill the priority queue with the first record from each Run.
  for (int run_index = 0; run_index < runs_.size(); run_index++) {
    if (runs_[run_index]->getNext(current_run_record[run_index], file_)) {
      Q.push(run_index);
    }
  }

  // Repeatedly pop from the Q and add the next record from the corresponding
  // run.
  while (!Q.empty()) {
    int P = Q.top();
    Q.pop();
    out.Insert(&current_run_record[P]);
    if (runs_[P]->getNext(current_run_record[P], file_)) {
      Q.push(P);
    }
  }
}
