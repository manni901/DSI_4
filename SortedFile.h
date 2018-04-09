#ifndef SORTED_FILE_H
#define SORTED_FILE_H

#include "BigQ.h"
#include "DBFileInternal.h"
#include "Pipe.h"
#include <memory>

class SortedFile : public DBFileInternal {
public:
  SortedFile() = default;
  ~SortedFile() = default;

  int Open(int length, const char *fpath, void *startup);

  int Close();

  void MoveFirst();

  void Add(Record &addme);

  int GetNext(Record &fetchme);
  int GetNext(Record &fetchme, CNF &cnf, Record &literal);

private:
  OrderMaker sortorder_;
  int run_length_ = 0;

  // BigQ for external sorting of incoming records.
  std::unique_ptr<BigQ> bigq_;

  // Pipe to insert new records into.
  std::unique_ptr<Pipe> in_pipe_;

  // Pipe to which BigQ inserts all sorted records.
  std::unique_ptr<Pipe> out_pipe_;
  Mode mode_ = READ;
  string file_name_;

  void PrepareIO(Mode mode);

  // Method to perform a 2-way merge between records in existing file
  // and new records from out_pipe_.
  void Merge();

  // Performs a binary search in terms of pages to get to the least
  // possible record according to the supplied sorting criteria.
  void BinarySearch(OrderMaker &query_order, Record &literal,
                    OrderMaker &cnf_order, ComparisonEngine &ce);
};

#endif