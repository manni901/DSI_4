#ifndef DBFILE_H
#define DBFILE_H

#include <iostream>
#include <memory>
#include <fstream>

#include "Comparison.h"
#include "ComparisonEngine.h"
#include "File.h"
#include "Record.h"
#include "Schema.h"
#include "TwoWayList.h"
#include "DBFileInternal.h"

typedef enum { heap, sorted, tree } fType;

// stub DBFile header..replace it with your own DBFile.h

class DBFile {

public:
  DBFile() = default;
  ~DBFile() = default;

  int Create(const char *fpath, fType file_type, void *startup);
  int Open(const char *fpath);
  int Close();

  void Load(Schema &myschema, const char *loadpath);

  void MoveFirst();
  void Add(Record &addme);
  int GetNext(Record &fetchme);
  int GetNext(Record &fetchme, CNF &cnf, Record &literal);

private:
  std::unique_ptr<DBFileInternal> db_;
};
#endif
