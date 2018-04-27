#ifndef QUERY_NODE_H
#define QUERY_NODE_H

#include "Comparison.h"
#include "Function.h"
#include "Record.h"
#include "Schema.h"
#include "RelOp.h"
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <unordered_map>
using namespace std;

#define QueryNodeP unique_ptr<QueryNode>

class QueryNode {
public:
  QueryNode(int out_pipe_id, unique_ptr<Schema> schema)
      : schema_(move(schema)), out_pipe_id_(out_pipe_id) {}

  QueryNode(int out_pipe_id) : out_pipe_id_(out_pipe_id) {}

  ~QueryNode() = default;

  // Get Out pipe id.
  int OutPipeId() { return out_pipe_id_; }

  // Get Output Schema.
  Schema *GetSchema() { return schema_.get(); }

  // Recursively print children nodes first.
  void PrintNode() {
    for (auto &child : children_) {
      child->PrintNode();
    }
    cout << "\n-------------------------------------\n";
    Print();
    cout << "\n-------------------------------------\n";
  }

  void ExecuteNode() {
    for (auto &child : children_) {
      child->ExecuteNode();
    }
    Execute();
  }

  void FinishNode() {
    Finish();
    int len = children_.size();
    while(len--) {
      children_[len]->FinishNode();
    }
  }

  // Overridden by every derived class.
  virtual void Print() = 0;

  void Execute() {
    rel_op_->Run();
  };

  void Finish() {
    rel_op_->WaitUntilDone();
  }

protected:
  // smart pointer to store schema
  unique_ptr<Schema> schema_;

  unique_ptr<RelationalOp> rel_op_;

  // output pipe_id.
  int out_pipe_id_;

  // Variable number of children.
  vector<QueryNodeP> children_;
};

class SelectPipeNode : public QueryNode {
private:
  CNF cnf_;
  int in_pipe_id_;

public:
  SelectPipeNode(ParseVector &parse_vector, BitSet &selector,
                 unique_ptr<Schema> schema, QueryNodeP input_node, int pipe_id, PMap<Pipe> &pipes);
  void Print();
  void Execute();
};

class ProjectNode : public QueryNode {
private:
  int in_pipe_id_;

public:
  ProjectNode(QueryNodeP input_node, unordered_set<string> &att_names,
              int pipe_id, PMap<Pipe> &pipes);
  void Print();
  void Execute();
};

class DuplicateRemovalNode : public QueryNode {
private:
  int in_pipe_id_;

public:
  DuplicateRemovalNode(QueryNodeP input_node, int pipe_id, PMap<Pipe> &pipes);
  void Print();
  void Execute();
};

class SelectFileNode : public QueryNode {
private:
  CNF cnf_;
  DBFile db_file_;

public:
  SelectFileNode(ParseVector &parse_vector, BitSet &selector, string table_name,
                 unique_ptr<Schema> schema, int pipe_id, PMap<Pipe> &pipes);
  void Print();
  void Execute();

  ~SelectFileNode() {
    db_file_.Close();
  }
};

class JoinNode : public QueryNode {
private:
  CNF cnf_;
  int left_pipe_id_;
  int right_pipe_id_;

public:
  JoinNode(ParseVector &parse_vector, BitSet &selector,
           unique_ptr<Schema> schema, QueryNodeP left, QueryNodeP right,
           int out_pipe_id, PMap<Pipe> &pipes);
  void Print();
  void Execute();
};

class GroupByNode : public QueryNode {
private:
  OrderMaker order_maker_;
  Function compute_;
  int in_pipe_id_;

public:
  GroupByNode(QueryNodeP input_node, unordered_set<string> &grouping_atts,
              FuncOperator *func, int pipe_id, PMap<Pipe> &pipes);
  void Print();
  void Execute();
};

class SumNode : public QueryNode {
private:
  Function compute_;
  int in_pipe_id_;

public:
  SumNode(QueryNodeP input_node, FuncOperator *func, int pipe_id, PMap<Pipe> &pipes);
  void Print();
  void Execute();
};

#endif