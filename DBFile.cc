#include "DBFile.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "Defs.h"
#include "File.h"
#include "HeapFile.h"
#include "SortedFile.h"
#include "Record.h"
#include "Schema.h"
#include "TwoWayList.h"

static std::unique_ptr<DBFileInternal> GetDBFile(fType f_type) {
  switch (f_type) {
  case heap:
    return std::make_unique<HeapFile>();
  case sorted:
    return std::make_unique<SortedFile>();
  default:
    cout << "Error: Unknown db file type";
    exit(1);
  }
}

int DBFile::Create(const char *f_path, fType f_type, void *startup) {
  if (db_) {
    Close();
  }
  // TODO: Add support for creating metadata file for table
  // storing atleast the file type information.
  db_ = GetDBFile(f_type);
  return db_->Open(0, f_path, startup);
}

void DBFile::Load(Schema &f_schema, const char *loadpath) {
  FILE *file = fopen(loadpath, "r");
  if (!file) {
    cout << "Error: Cannot open load file.";
    exit(1);
  }

  Record rec;
  while (rec.SuckNextRecord(&f_schema, file)) {
    Add(rec);
  }
}

int DBFile::Open(const char *f_path) {
  if (db_) {
    Close();
  }

  int file_type = heap;

  string meta_file(f_path);
  meta_file += ".meta";
  ifstream ifs(meta_file);
  if (ifs) {
    ifs >> file_type;
    ifs.close();
  }
  db_ = GetDBFile(static_cast<fType>(file_type));
  return db_->Open(1, f_path, /* startup */ nullptr);
}

void DBFile::MoveFirst() { db_->MoveFirst(); }

int DBFile::Close() {
  int close_status = db_->Close();
  if (close_status != 1) {
    cout << "Error in closing file.";
  }
  //db_.reset(nullptr);
  return 1;
}

void DBFile::Add(Record &rec) { db_->Add(rec); }

int DBFile::GetNext(Record &fetchme) { return db_->GetNext(fetchme); }

int DBFile::GetNext(Record &fetchme, CNF &cnf, Record &literal) {
  return db_->GetNext(fetchme, cnf, literal);
}
