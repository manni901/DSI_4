#ifndef RELATIONAL_OP_H
#define RELATIONAL_OP_H

#include <thread>

#include "BigQ.h"
#include "Comparison.h"
#include "DBFile.h"
#include "Function.h"
#include "Pipe.h"
#include "Record.h"

class RelationalOp {

public:
  // blocks the caller until the particular relational operator has
  // run to completion
  void WaitUntilDone() { thread_.join(); }

  // tells how much internal memory the operation can use
  virtual void Use_n_Pages(int n) { run_len_ = n; }

  virtual void Run() = 0;

protected:
  std::thread thread_;

  int run_len_ = 1;
};

class SelectPipe : public RelationalOp {
public:
  SelectPipe(Pipe &in_pipe, Pipe &out_pipe, CNF &sel_op, Record &literal)
      : in_pipe(in_pipe), out_pipe(out_pipe), sel_op(sel_op), literal(literal) {
  }

  void Run();

private:
  Pipe &in_pipe;
  Pipe &out_pipe;
  CNF &sel_op;
  Record literal;
};

class SelectFile : public RelationalOp {
public:
  SelectFile(DBFile &in_file, Pipe &out_pipe, CNF &sel_op, Record literal)
      : in_file(in_file), out_pipe(out_pipe), sel_op(sel_op), literal(literal) {
  }

  void Run();

private:
  DBFile &in_file;
  Pipe &out_pipe;
  CNF &sel_op;
  Record literal;
};

class Project : public RelationalOp {
public:
  Project(Pipe &in_pipe, Pipe &out_pipe, vector<int>& keep_me, int num_atts_input,
          int num_atts_output)
      : in_pipe(in_pipe), out_pipe(out_pipe), keep_me(keep_me),
        num_atts_input(num_atts_input), num_atts_output(num_atts_output) {}

  void Run();

private:
  Pipe &in_pipe;
  Pipe &out_pipe;
  vector<int> keep_me;
  int num_atts_input;
  int num_atts_output;
};

class WriteOut : public RelationalOp {
public:
  WriteOut(Pipe &in_pipe, FILE *out_file, Schema &my_schema)
      : in_pipe(in_pipe), out_file(out_file), my_schema(my_schema) {}

  void Run();

private:
  Pipe &in_pipe;
  FILE *out_file;
  Schema &my_schema;
};

class DuplicateRemoval : public RelationalOp {
public:
  DuplicateRemoval(Pipe &in_pipe, Pipe &out_pipe, Schema &my_schema)
      : in_pipe(in_pipe), out_pipe(out_pipe), my_schema(my_schema) {}

  void Run();

private:
  Pipe &in_pipe;
  Pipe &out_pipe;
  Schema &my_schema;
};

class Sum : public RelationalOp {
public:
  Sum(Pipe &in_pipe, Pipe &out_pipe, Function &compute_me)
      : in_pipe(in_pipe), out_pipe(out_pipe), compute_me(compute_me) {}

  void Run();

private:
  Pipe &in_pipe;
  Pipe &out_pipe;
  Function &compute_me;
};

class GroupBy : public RelationalOp {
public:
  GroupBy(Pipe &in_pipe, Pipe &out_pipe, OrderMaker &group_atts,
          Function &compute_me)
      : in_pipe(in_pipe), out_pipe(out_pipe), group_atts(group_atts),
        compute_me(compute_me) {}

  void Run();

private:
  Pipe &in_pipe;
  Pipe &out_pipe;
  OrderMaker &group_atts;
  Function &compute_me;
};

class Join : public RelationalOp {
public:
  Join(Pipe &in_pipe_L, Pipe &in_pipe_R, Pipe &out_pipe, CNF &sel_op,
       Record literal)
      : in_pipe_L(in_pipe_L), in_pipe_R(in_pipe_R), out_pipe(out_pipe),
        sel_op(sel_op), literal(literal) {}

  void Run();

private:
  Pipe &in_pipe_L;
  Pipe &in_pipe_R;
  Pipe &out_pipe;
  CNF &sel_op;
  Record literal;
};

#endif