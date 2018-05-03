#ifndef BIG_Q_DISTINCT_H
#define BIG_Q_DISTINCT_H

#include "BigQ.h"

// This class is used to get distinct values for aggregate
// functions. Deployed when we have SUM DISTINCT in the query.
// It only accepts single value records of type Int or Double.
// Internally it uses a BigQ to sort these records.
// When users calls GetDistinctSum after shutting down the
// input pipe, it adds all the distinct records and returns
// the result.
class BigQDistinct {
public:
  BigQDistinct(Pipe &in_pipe, OrderMaker &o, int run_len) {
      Q_ = make_unique<BigQ>(in_pipe, out_pipe_, o, run_len);
  }
  template <typename T>
  T GetDistinctSum() {
    T prev = 0;
    T distinct_sum = 0;
    Record rec;
    while (out_pipe_.Remove(&rec)) {
      T curr = rec.GetFirstValue<T>();
      if (curr != prev) {
        distinct_sum += curr;
        prev = curr;
      }
    }
    Q_->End();
    return distinct_sum;
  }

private:
  unique_ptr<BigQ> Q_;
  Pipe out_pipe_;
};

#endif