#ifndef DEFS_H
#define DEFS_H

#include <iostream>
#include <vector>
#include <bitset>
using namespace std;

#define MAX_ANDS 20
#define MAX_ORS 20

#define PAGE_SIZE 131072


enum Target {Left, Right, Literal};
enum CompOperator {LessThan, GreaterThan, Equals};
enum Type {Int, Double, String};


unsigned int Random_Generate();

#define MAX_EXPRESSION_SIZE 1000

#define ParseVector vector<vector<NewComparisonOp>>

#define BitSet bitset<MAX_EXPRESSION_SIZE>

#define FOREACH_BS(v, vSet)                                                    \
  for (size_t v = vSet._Find_first(); v != vSet.size(); v = vSet._Find_next(v))

#define CATALOG "catalog"

struct NewOperand {
  int code;
  std::string value;
};

struct NewComparisonOp {
  int code;

  struct NewOperand left;
  struct NewOperand right;
};


#endif

