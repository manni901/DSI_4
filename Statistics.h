#ifndef STATISTICS_H
#define STATISTICS_H

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "ParseTree.h"

using namespace std;

struct RelationInfo {
  double num_tuples;
  unordered_map<string, double> attribute_info;
};

class Statistics {
private:
  unordered_map<string, RelationInfo> relations_;

  unordered_map<string, string> attr_to_relation_name_;

  void JoinRel(vector<string> relation_names, double num_tuples);

public:
  void AddRel(const char *relName, int numTuples);

  void AddAtt(const char *relName, const char *attName, int numDistincts);

  void CopyRel(const char *oldName, const char *newName);

  void Write(const char *fileName);

  void Read(const char *fileName);

  double Estimate(struct AndList *parseTree, char **relNames, int numToJoin, bool persist = false);

  void Apply(struct AndList *parseTree, char **relNames, int numToJoin) {
    double res = Estimate(parseTree, relNames, numToJoin, true);
  }
};

#endif