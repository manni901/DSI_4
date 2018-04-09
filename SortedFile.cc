#include "SortedFile.h"
#include "HeapFile.h"
#include "DBFile.h"

struct SortInfo {
  OrderMaker *myOrder;
  int runLength;
};

int SortedFile::Open(int length, const char *fpath, void *startup) {
  string meta_file(fpath);
  file_name_ = meta_file;
  meta_file += ".meta";
  if (startup) {
    SortInfo *sort_info = (SortInfo *)startup;
    sortorder_ = *sort_info->myOrder;
    run_length_ = sort_info->runLength;
  } else {
    int file_type;
    ifstream ifs(meta_file);
    if (!ifs) {
      exit(1);
    }
    ifs >> file_type >> run_length_ >> sortorder_;
    ifs.close();
  }
  return DBFileInternal::Open(length, fpath, startup);
}

void SortedFile::MoveFirst() {
  PrepareIO(READ);
  DBFileInternal::MoveFirst();
}

int SortedFile::Close() {
  PrepareIO(READ);

  string meta_file_name = file_name_ + ".meta";
  ofstream ofs(meta_file_name);
  ofs << (int)fType::sorted << "\n" << run_length_ << "\n" << sortorder_;
  ofs.close();

  file_.Close();
  return 1;
}

void SortedFile::PrepareIO(Mode mode) {
  // Only proceed if there's a change in mode.
  if (mode != mode_) {
    mode_ = mode;
    switch (mode) {
    case WRITE:
      in_pipe_.reset(new Pipe(100));
      out_pipe_.reset(new Pipe(100));
      bigq_.reset(
          new BigQ(*in_pipe_.get(), *out_pipe_.get(), sortorder_, run_length_));
      break;
    case READ:
      Merge();
      bigq_.reset(nullptr);
      in_pipe_.reset(nullptr);
      out_pipe_.reset(nullptr);
      break;
    default:
      break;
    }
  }
}

void SortedFile::Merge() {
  in_pipe_->ShutDown();
  string new_file_name = file_name_ + "_new";
  HeapFile new_file;
  new_file.Open(0, new_file_name.c_str(), nullptr);

  MoveFirst();
  ComparisonEngine ce;
  Record A, B;
  int a = GetNext(A);
  int b = out_pipe_->Remove(&B);

  int count = 0, pipe_count = 0;
  while (a && b) {
    if (ce.Compare(&A, &B, &sortorder_) < 0) {
      new_file.Add(A);
      a = GetNext(A);
    } else {
      new_file.Add(B);
      b = out_pipe_->Remove(&B);
    }
  }
  while (a) {
    new_file.Add(A);
    a = GetNext(A);
  }
  while (b) {
    new_file.Add(B);
    b = out_pipe_->Remove(&B);
  }
  new_file.Close();
  rename(new_file_name.c_str(), file_name_.c_str());
  page_index_ = 0;
  file_.Open(1, file_name_.c_str());
}

void SortedFile::Add(Record &addme) {
  PrepareIO(WRITE);
  in_pipe_->Insert(&addme);
}

int SortedFile::GetNext(Record &fetchme) {
  PrepareIO(READ);
  return DBFileInternal::GetNext(fetchme);
}

int SortedFile::GetNext(Record &fetchme, CNF &cnf, Record &literal) {
  PrepareIO(READ);
  OrderMaker query_order, cnf_order;
  sortorder_.BuildQueryOrder(query_order, cnf_order, cnf);
  ComparisonEngine ce;

  if(!GetNext(fetchme)) {
    return 0;
  }
  int query_match = ce.Compare(&fetchme, &query_order, &literal, &cnf_order);
  if(query_match > 0) {
    return 0;
  }
  if(query_match < 0) {
    BinarySearch(query_order, literal, cnf_order, ce);
    if(!GetNext(fetchme)) {
      return 0;
    }
  }

  do {
    int query_match = ce.Compare(&fetchme, &query_order, &literal, &cnf_order);
    if (query_match > 0) {
      return 0;
    } else if(query_match < 0) {
      continue;
    }
    if (ce.Compare(&fetchme, &literal, &cnf)) {
      return 1;
    }
  } while(GetNext(fetchme));
  return 0;
}

void SortedFile::BinarySearch(OrderMaker &query_order,
                              Record &literal, OrderMaker &cnf_order,
                              ComparisonEngine &ce) {
  Record fetchme;
  int query_match = 0;
  off_t left = page_index_;
  off_t right = file_.GetLength() - 2;
  while (left < right - 1) {
    off_t mid = (left + right) / 2;
    page_index_ = mid;
    file_.GetPage(&page_, page_index_++);
    if (!GetNext(fetchme)) {
      return;
    }
    query_match = ce.Compare(&fetchme, &query_order, &literal, &cnf_order);
    if (query_match < 0) {
      left = mid;
    } else if (query_match > 0) {
      right = mid - 1;
    } else {
      right = mid;
    }
  }

  page_index_ = left;
  file_.GetPage(&page_, page_index_++);
}