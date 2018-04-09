#ifndef CROSS_JOIN_H
#define CROSS_JOIN_H

#include "Comparison.h"
#include "ComparisonEngine.h"
#include "Defs.h"
#include "Pipe.h"
#include "Record.h"
#include <vector>

class CrossJoin {
public:
  CrossJoin(int run_len) : left_size_(PAGE_SIZE * run_len) {}
  ~CrossJoin() = default;

  bool AddLeft(Record &rec) {
    int rec_size = rec.GetSize();
    if (left_size_used_ + rec_size < left_size_) {
      left_.push_back(rec);
      left_size_used_ += rec_size;
      return true;
    }
    return false;
  }

  void ClearLeft() { left_.clear(); }

  void SetRight(Record &rec) { right_ = rec; }

  void Exectute(Pipe &out_pipe, CNF &sel_op, Record &literal) {
    if (left_.size() == 0) {
      return;
    }
    std::vector<int> atts;
    int num_left = left_[0].NumAtts();
    int num_right = right_.NumAtts();
    for (int i = 0; i < num_left; i++) {
      atts.push_back(i);
    }
    for (int i = 0; i < num_right; i++) {
      atts.push_back(i);
    }
    Record rec;
    Record temp;
    ComparisonEngine ce;
    for (auto &rec_left : left_) {
      temp = right_;
      if (ce.Compare(&rec_left, &temp, &literal, &sel_op)) {
        rec.MergeRecords(&rec_left, &temp, num_left, num_right, atts.data(),
                         num_left + num_right, num_left);
      }
    }
    out_pipe.Insert(&rec);
  }

private:
  std::vector<Record> left_;

  Record right_;

  long long int left_size_;
  long long int left_size_used_ = 0;
};
#endif