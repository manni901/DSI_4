#ifndef QUERY_PLAN_H
#define QUERY_PLAN_H

#include "ParseTree.h"
#include "QueryNode.h"
#include "Statistics.h"
#include <ostream>
#include "Pipe.h"
#include "Defs.h"
#include <algorithm>
#include <climits>
#include <memory>
#include <vector>
#include <unordered_set>
#include <thread>
using namespace std;

// Following variables are defined in Parser.y
extern struct FuncOperator *finalFunction;
extern vector<pair<string, string>> tables;
extern struct AndList *boolean;
extern vector<string> groupingAtts;
extern vector<string> attsToSelect;
extern int distinctAtts;
extern int distinctFunc;
extern string output_file;
extern int output_mode;

class QueryPlan {
public:
  QueryPlan(string stat_file);
  ~QueryPlan() = default;

  // Prints the entire query plan recursively in in-order fashion.
  void Print() { root_node_->PrintNode(); }

  void Execute() { 
    root_node_->ExecuteNode();
  }

  // Calls WaitUntilDone on all the QueryNodes in top down fashion.
  // This is so that we join threads in reverse of the order in which
  // they were created.
  void Finish() { 
    root_node_->FinishNode();
    if(out_fstream_.is_open()) {
      out_fstream_.close();
    }
  }

private:

  // Statistics object for evaluating different query orders.
  Statistics stat_;

  // A vector based variant for AndList (see Defs.h).
  ParseVector parse_vector_;

  // table_names_ and alias_ store information from TableList.
  vector<string> table_names_;
  unordered_map<string, string> alias_;

  // Root Node for the QueryPlan.
  QueryNodeP root_node_;

  // Pipe id to increment and assign from.
  int curr_pipe_id_ = 0;

  PMap<Pipe> pipes_;

  // Returns the minimum cost join order.
  void UpdateJoinOrder();

  // Builds SelectFile and JoinNodes.
  void BuildJoinNodes();

  // Builds GroupBy or Sum Nodes.
  void BuildGroupByNode();

  // Builds Project Nodes.
  void BuildProjectNode();

  // Builds Distinct Nodes.
  void BuildDistinctNode();

  ofstream out_fstream_;

  ostream& GetOutMode() {
    if(output_mode == 1) {
      return std::cout;
    } else {
      out_fstream_.open(output_file);
      return out_fstream_;
    }
  }
};

#endif