#ifndef DB_FILE_INTERNAL_H
#define DB_FILE_INTERNAL_H

#include "Comparison.h"
#include "ComparisonEngine.h"
#include "File.h"
#include "Record.h"
#include "Schema.h"
#include "TwoWayList.h"
#include <iostream>

typedef enum { READ, WRITE } Mode;

// Base class for three different types of db files:
// Heap, Sorted and Tree.
class DBFileInternal {
public:
  DBFileInternal() = default;
  ~DBFileInternal() = default;

  // Opens the file by calling file_.Open.
  virtual int Open(int length, const char *fpath, void *startup) {
    file_.Open(length, fpath);
    if (file_.GetLength() >= 2) {
      page_index_ = file_.GetLength() - 2;
      file_.GetPage(&page_, page_index_);
    }
    return 1;
  }

  // Closes the file.
  // Additionally writes the current page to disk if
  // it has any records left on it.
  virtual int Close() = 0;

  // Modifies page_ to point to the first page in file_.
  virtual void MoveFirst() {
    page_index_ = 0;
    if (page_index_ > file_.GetLength() - 2) {
      return;
    }
    file_.GetPage(&page_, page_index_++);
  }

  // Adds given record to page_ and consumes it.
  // If page_ is already full, adds new page to file_
  // and then appends the record to this new page.
  virtual void Add(Record &addme) = 0;

  // Get next record in page_ or move to next page
  // if page_ is empty.
  // Returns: 1 if successful, 0 if not.
  virtual int GetNext(Record &fetchme) {
    while (!page_.GetFirst(&fetchme)) {
      if (page_index_ > file_.GetLength() - 2) {
        // Reached end of file, no more pages left.
        return 0;
      }
      file_.GetPage(&page_, page_index_++);
    }
    return 1;
  };

  // Keeps calling GetNext(fetchme) until the returned
  // record meets the cnf condition.
  // Return: 1 if successful, 0 if not.
  virtual int GetNext(Record &fetchme, CNF &cnf, Record &literal) = 0;

protected:
  // File to read pages from the disk.
  File file_;

  // Hold the current page in file_
  Page page_;

  // Offset to keep track of the number of pages inserted in file.
  off_t page_index_ = 0;
};
#endif
