#ifndef STATISTICS_H
#define STATISTICS_H

#include <bitset>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ParseTree.h"
#include "ParseUtil.h"

using namespace std;

struct RelationInfo {
  double num_tuples;
  unordered_map<string, double> attribute_info;
};

class Statistics {
private:
  unordered_map<string, RelationInfo> relations_;

  unordered_map<string, string> attr_to_relation_name_;

  unordered_map<string, string> set_name_;

  void JoinRel(vector<string> relation_names, double num_tuples);

public:
  void AddRel(const char *relName, int numTuples);

  void AddAtt(const char *relName, const char *attName, int numDistincts);

  void CopyRel(const char *oldName, const char *newName);

  void Write(const char *fileName);

  void Read(const char *fileName);

  /*double Estimate(struct AndList *parseTree, char **relNames, int numToJoin,
                  bool persist = false);*/

  /*double Estimate(ParseVector &parseTree, char **relNames, int numToJoin,
                  bool persist = false);*/

  double Apply(ParseVector &parseTree, vector<string> &table_names,
             BitSet &selector) {
    double res = Estimate(parseTree, table_names, selector, true);
    return res;
  }

  double Estimate(ParseVector &parseTree, vector<string> &table_names,
                  BitSet &selector, bool persist = false);

  /*void Apply(struct AndList *parseTree, char **relNames, int numToJoin) {
    ParseVector parse_vector = ParseUtil::AndListToVector(parseTree);
    double res = Estimate(parse_vector, relNames, numToJoin, true);
  }*/

  double GetEstimate(string table_name) {
    return relations_[table_name].num_tuples;
  }
};

#endif