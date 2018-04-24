#ifndef PARSE_UTIL_H
#define PARSE_UTIL_H

#include "Defs.h"
#include "ParseTree.h"
#include "Schema.h"
#include <bitset>
#include <unordered_set>
#include <vector>

using namespace std;

class ParseUtil {
public:
  // Converts the AndList into a vector<vector<ComparisonOp>
  // Useful to avoid the old list style syntax.
  static vector<vector<NewComparisonOp>>
  AndListToVector(struct AndList *parse_tree) {
    vector<vector<NewComparisonOp>> parse_vector;
    while (parse_tree != NULL) {
      if (parse_tree->left != NULL) {
        struct OrList *or_list = parse_tree->left;
        vector<NewComparisonOp> or_vector;
        while (or_list != NULL) {
          NewComparisonOp op;
          op.code = or_list->left->code;

          op.left.code = or_list->left->left->code;
          op.left.value = or_list->left->left->value;

          op.right.code = or_list->left->right->code;
          op.right.value = or_list->left->right->value;

          or_vector.push_back(op);

          or_list = or_list->rightOr;
        }
        parse_vector.push_back(or_vector);
        parse_tree = parse_tree->rightAnd;
      }
    }
    return parse_vector;
  }

  // returns a new BitSet with bits set corresponding to or_list having
  // attributes from the given schema, and not already present in
  // already_selected. Also modified the alread_selected bitset to include the
  // new ones found in the result.
  static BitSet FilterParseVector(ParseVector &parse_vector, Schema &schema,
                                  BitSet &already_selected) {
    BitSet answer;
    for (int i = 0; i < parse_vector.size(); i++) {
      if (!already_selected.test(i)) {
        auto &or_list = parse_vector[i];
        bool is_valid = true;
        for (auto &cop : or_list) {
          if (cop.left.code == NAME &&
              schema.Find(cop.left.value.c_str()) == -1) {
            is_valid = false;
            break;
          }
          if (cop.right.code == NAME &&
              schema.Find(cop.right.value.c_str()) == -1) {
            is_valid = false;
            break;
          }
        }
        if (is_valid) {
          answer.set(i);
        }
      }
    }
    already_selected = already_selected | answer;
    return answer;
  }

  // Join relation names with an underscore in between.
  static string joinVector(vector<string> &names) {
    string answer = "";
    for (auto name : names) {
      answer += name + "_";
    }
    if (answer != "")
      answer.pop_back();
    return answer;
  }
};

#endif