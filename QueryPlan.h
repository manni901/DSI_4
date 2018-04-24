#ifndef QUERY_PLAN_H
#define QUERY_PLAN_H

#include "ParseTree.h"
#include "QueryNode.h"
#include "Statistics.h"
#include <algorithm>
#include <climits>
#include <memory>
#include <vector>
using namespace std;

// Following variables are defined in Parser.y
extern struct FuncOperator *finalFunction;
extern struct TableList *tables;
extern struct AndList *boolean;
extern struct NameList *groupingAtts;
extern struct NameList *attsToSelect;
extern int distinctAtts;
extern int distinctFunc;

class QueryPlan {
public:
  QueryPlan(string stat_file);

  // Prints the entire query plan recursively in in-order fashion.
  void Print() { root_node_->PrintNode(); }

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
};

#endif