#ifndef HEAP_FILE_H
#define HEAP_FILE_H

#include "DBFileInternal.h"

class HeapFile : public DBFileInternal {
public:
  HeapFile() = default;
  ~HeapFile() = default;

  int Close();

  void MoveFirst();

  void Add(Record &addme);

  int GetNext(Record &fetchme);
  int GetNext(Record &fetchme, CNF &cnf, Record &literal);
};

#endif