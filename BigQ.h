#ifndef BIGQ_H
#define BIGQ_H
#include "File.h"
#include "Pipe.h"
#include "Record.h"
#include <algorithm>
#include <climits>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <queue>
#include <thread>
#include <unordered_set>
#include <vector>
#include "TempFileGen.h"

using namespace std;

// Class to hold information about a particular Run.
// current_page_index_ and last_page_index_ specify the range
// of file pages for the Run, and thus the list of sorted records
// for the Run.
class Run {
public:
  Run(off_t current_page_index, off_t last_page_index)
      : current_page_index_(current_page_index),
        last_page_index_(last_page_index) {}

  // Keeps getting records in sorted order till the end
  // of page index: last_page_index_.
  bool getNext(Record &record, File &file) {
    while (!page_.GetFirst(&record)) {
      if (current_page_index_ > last_page_index_) {
        return false;
      }
      file.GetPage(&page_, current_page_index_++);
    }
    return true;
  }

private:
  off_t current_page_index_;
  off_t last_page_index_;
  Page page_;
};

class BigQ {

public:
  // Starts an std::thread worker_, which will perform the external
  // sort and merge for all records coming from pipe 'in'.
  BigQ(Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen);

  // Will close the thread worker_.
  void End() {
    worker_.join();
  }

  ~BigQ();

private:

  // Worker thread.
  thread worker_;

  // Vector to hold records for in memory sort and
  // subsequent copy to disk.
  vector<unique_ptr<Record>> record_run_;

  // Comparison engine to compare records based on sort order.
  ComparisonEngine ce_;

  // Vector of runs to hold page range and iterate over sorted records.
  vector<unique_ptr<Run>> runs_;

  // Temporary file to save all the sorted chunks.
  File file_;

  // page index to keep track of the number of pages used to save records
  // in temporary file_.
  off_t curr_page_index_ = 0;

  // Method which sorts all records currently in record_run_ and
  // saves them on disk to file_.
  void SortNSave(OrderMaker &sortorder);

  // Method to sort all the runs in runs_ using a priority_queue
  // and stream the sorted records to out pipe.
  void MergeRuns(OrderMaker &sortorder, Pipe &out);
};

#endif
