#ifndef QUERY_NODE_H
#define QUERY_NODE_H

#include "Comparison.h"
#include "Function.h"
#include "Record.h"
#include "Schema.h"
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
using namespace std;

#define QueryNodeP unique_ptr<QueryNode>

class QueryNode {
public:
  QueryNode(int out_pipe_id, unique_ptr<Schema> schema)
      : schema_(move(schema)), out_pipe_id_(out_pipe_id) {}

  QueryNode(int out_pipe_id) : out_pipe_id_(out_pipe_id) {}

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

  // Overridden by every derived class.
  virtual void Print() = 0;

protected:
  // smart pointer to store schema
  unique_ptr<Schema> schema_;

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
                 unique_ptr<Schema> schema, QueryNodeP input_node, int pipe_id);
  void Print();
};

class ProjectNode : public QueryNode {
private:
  int in_pipe_id_;

public:
  ProjectNode(QueryNodeP input_node, unordered_set<string> &att_names,
              int pipe_id);
  void Print();
};

class DuplicateRemovalNode : public QueryNode {
private:
  int in_pipe_id_;

public:
  DuplicateRemovalNode(QueryNodeP input_node, int pipe_id);
  void Print();
};

class SelectFileNode : public QueryNode {
private:
  CNF cnf_;

public:
  SelectFileNode(ParseVector &parse_vector, BitSet &selector,
                 unique_ptr<Schema> schema, int pipe_id);
  void Print();
};

class JoinNode : public QueryNode {
private:
  CNF cnf_;
  int left_pipe_id_;
  int right_pipe_id_;

public:
  JoinNode(ParseVector &parse_vector, BitSet &selector,
           unique_ptr<Schema> schema, QueryNodeP left, QueryNodeP right,
           int out_pipe_id);
  void Print();
};

class GroupByNode : public QueryNode {
private:
  OrderMaker order_maker_;
  Function compute_;
  int in_pipe_id_;

public:
  GroupByNode(QueryNodeP input_node, unordered_set<string> &grouping_atts,
              FuncOperator *func, int pipe_id);
  void Print();
};

class SumNode : public QueryNode {
private:
  Function compute_;
  int in_pipe_id_;

public:
  SumNode(QueryNodeP input_node, FuncOperator *func, int pipe_id);
  void Print();
};

#endif