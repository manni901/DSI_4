#include "HeapFile.h"
#include "File.h"

int HeapFile::Close() {
  // If page_ has any records on it, add them to file_.
  if (page_.NumRecs() > 0) {
    file_.AddPage(&page_, page_index_++);
    page_.EmptyItOut();
  } 
  file_.Close();
  return 1;
}

void HeapFile::MoveFirst() {
  DBFileInternal::MoveFirst();
}

void HeapFile::Add(Record &addme) {
  if (!page_.Append(&addme)) {
    file_.AddPage(&page_, page_index_++);
    page_.EmptyItOut();
    page_.Append(&addme);
  }
}

int HeapFile::GetNext(Record &fetchme) {
  return DBFileInternal::GetNext(fetchme);
}

int HeapFile::GetNext(Record &fetchme, CNF &cnf, Record &literal) {
  ComparisonEngine cmp;
  while (GetNext(fetchme)) {
    if (cmp.Compare(&fetchme, &literal, &cnf)) {
      return 1;
    }
  }
  return 0;
}