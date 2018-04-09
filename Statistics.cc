#include "Statistics.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

void Statistics::AddRel(const char *relName, int numTuples) {
  string name(relName);
  RelationInfo relation;
  relation.num_tuples = numTuples;
  relations_[name] = relation;
}

void Statistics::AddAtt(const char *relName, const char *attName,
                        int numDistincts) {
  string name(relName);
  string att_name(attName);
  relations_[name].attribute_info[att_name] =
      numDistincts == -1 ? relations_[name].num_tuples : numDistincts;
  attr_to_relation_name_[att_name] = name;
}

void Statistics::CopyRel(const char *oldName, const char *newName) {
  string old_name(oldName);
  string new_name(newName);
  AddRel(newName, relations_[old_name].num_tuples);
  for (auto &attr : relations_[old_name].attribute_info) {
    string new_attr_name = new_name + "." + attr.first;
    AddAtt(newName, new_attr_name.c_str(), attr.second);
  }
}

void Statistics::Write(const char *fileName) {
  string file_name(fileName);
  ofstream out(file_name);
  out << relations_.size() << "\n";
  for (auto &relation : relations_) {
    out << relation.first << " " << relation.second.num_tuples << "\n";
    out << relation.second.attribute_info.size() << "\n";
    for (auto &attr : relation.second.attribute_info) {
      out << attr.first << " " << attr.second << "\n";
    }
  }
  out.close();
}

void Statistics::Read(const char *fileName) {
  string file_name(fileName);
  ifstream in(fileName);
  if (!in.good())
    return;
  int num_relations;
  in >> num_relations;
  while (num_relations--) {
    RelationInfo info;
    string relation_name;
    in >> relation_name >> info.num_tuples;
    int num_attributes;
    in >> num_attributes;
    while (num_attributes--) {
      string attr_name;
      int num_distinct;
      in >> attr_name >> num_distinct;
      info.attribute_info[attr_name] = num_distinct;
      attr_to_relation_name_[attr_name] = relation_name;
    }
    relations_[relation_name] = info;
  }
  in.close();
}

double Statistics::Estimate(struct AndList *parseTree, char **relNames,
                            int numToJoin, bool persist) {
  double result = 0.0, factor = 1.0;
  bool is_first_join = true;
  struct AndList *and_list = parseTree;
  unordered_set<string> relation_names;

  // If AndList is null, there should only be 1 relation.
  if (and_list == NULL) {
    if (numToJoin > 1)
      return -1;
    return relations_[relNames[0]].num_tuples;
  }

  // Check if given attribute exists in any given relations.
  auto err_check = [this, &relation_names](string att_name) -> bool {
    if (attr_to_relation_name_.find(att_name) == attr_to_relation_name_.end()) {
      cout << att_name << " does not belong to any relation.\n";
      return false;
    }
    return true;
  };
  vector<string> joined_relations;

  while (and_list != NULL) {
    if (and_list->left != NULL) {
      struct OrList *or_list = and_list->left;
      double factor_or = 0.0;
      string prev_attr;

      while (or_list != NULL) {
        Operand *left_operand = or_list->left->left;
        Operand *right_operand = or_list->left->right;
        int op = or_list->left->code;

        if (left_operand->code != NAME) {
          cout << "Invalid Query: left operand should type NAME.\n";
          return -1;
        }

        string attr_left = left_operand->value;
        if (!err_check(attr_left)) {
          return -1;
        }
        RelationInfo &relation_left =
            relations_[attr_to_relation_name_[attr_left]];

        // Check if this is a join operation
        if (right_operand->code == NAME) {
          string attr_right = right_operand->value;
          if (!err_check(attr_right)) {
            return -1;
          }
          RelationInfo &relation_right =
              relations_[attr_to_relation_name_[attr_right]];

          double max_distinct = max(relation_left.attribute_info[attr_left],
                                    relation_right.attribute_info[attr_right]);

          if (is_first_join) {
            result = relation_left.num_tuples * relation_right.num_tuples /
                     max_distinct;
            joined_relations.push_back(attr_to_relation_name_[attr_left]);
            is_first_join = false;
          } else {
            result = result * relation_right.num_tuples / max_distinct;
          }

          joined_relations.push_back(attr_to_relation_name_[attr_right]);
        } else {
          // this is selection filter: equal, greater than or less than
          if (result == 0.0) {
            result = relation_left.num_tuples;
          }
          double distinct_factor = 1.0;
          // If the operator is equal, divide by the num_distinct of attribute
          // else divide by 3.
          distinct_factor /=
              (op == EQUALS) ? relation_left.attribute_info[attr_left] : 3.0;

          factor_or += (prev_attr == attr_left)
                           ? distinct_factor
                           : (distinct_factor - (distinct_factor * factor_or));
          prev_attr = attr_left;
        }
        or_list = or_list->rightOr;
      }
      if (factor_or != 0.0) {
        factor *= factor_or;
      }
    }
    and_list = and_list->rightAnd;
  }
  result = result * factor;

  // If this is a call via Apply, then persist the joined relation set
  // and remove the old sets.
  if (persist) {
    JoinRel(joined_relations, result);
  }
  return result;
}

void Statistics::JoinRel(vector<string> relation_names, double num_tuples) {
  string new_relation_name = "";
  for (string s : relation_names) {
    new_relation_name += s + ":";
  }
  new_relation_name.pop_back();
  AddRel(new_relation_name.c_str(), num_tuples);
  for (string s : relation_names) {
    for (auto &attr : relations_[s].attribute_info) {
      AddAtt(new_relation_name.c_str(), attr.first.c_str(), attr.second);
    }
    relations_.erase(s);
  }
}