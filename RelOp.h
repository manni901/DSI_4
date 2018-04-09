#ifndef RELATIONAL_OP_H
#define RELATIONAL_OP_H

#include <thread>

#include "BigQ.h"
#include "Comparison.h"
#include "DBFile.h"
#include "Pipe.h"
#include "Record.h"
#include "Function.h"

class RelationalOp {

public:
  // blocks the caller until the particular relational operator has
  // run to completion
  void WaitUntilDone() { thread_.join(); }

  // tells how much internal memory the operation can use
  virtual void Use_n_Pages(int n) { run_len_ = n; }

protected:
  std::thread thread_;

  int run_len_ = 1;
};

class SelectPipe : public RelationalOp {
public:
  void Run(Pipe &in_pipe, Pipe &out_pipe, CNF &sel_op, Record &literal);
};

class SelectFile : public RelationalOp {
public:
  void Run(DBFile &in_file, Pipe &out_pipe, CNF &sel_op, Record &literal);
};

class Project : public RelationalOp {
public:
  void Run(Pipe &in_pipe, Pipe &out_pipe, int *keep_me, int num_atts_input,
           int num_atts_output);
};

class WriteOut : public RelationalOp {
public:
  void Run(Pipe &in_pipe, FILE *out_file, Schema &my_schema);
};

class DuplicateRemoval : public RelationalOp {
public:
  void Run(Pipe &in_pipe, Pipe &out_pipe, Schema &my_schema);
};

class Sum : public RelationalOp {
    public:
    void Run(Pipe &in_pipe, Pipe &out_pipe, Function &compute_me);
};

class GroupBy : public RelationalOp {
    public:
    void Run (Pipe &in_pipe, Pipe &out_pipe, OrderMaker &group_atts, Function &compute_me);
};

class Join : public RelationalOp {
    public:
    void Run (Pipe &in_pipe_L, Pipe &in_pipe_R, Pipe &out_pipe, CNF &sel_op, Record &literal);
};

#endif