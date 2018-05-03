#include "QueryNode.h"
#include "Defs.h"
#include "ParseUtil.h"
#include "ParseTree.h"
#include "QueryPlan.h"
#include "TableOperation.h"
#include <cstdio>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>
#include <ctime>

using namespace std;

extern "C" struct YY_BUFFER_STATE *yy_scan_string(const char *);
extern "C" int yyparse(void);
extern struct AndList *boolean;
extern struct FuncOperator *finalFunction;
extern int query_type;
extern string mytable;
extern vector<pair<string, int>> modern_schema;
extern int db_file_type;
extern string insert_file_name;
extern string output_file;
extern int output_mode;
extern vector<string> sort_atts;
extern vector<pair<string, string>> tables;
extern vector<string> groupingAtts;
extern vector<string> attsToSelect;

void ClearFinalFunc(FuncOperator *func) {
  if(func == NULL) return;
  delete func->leftOperand;
  ClearFinalFunc(func->leftOperator);
  ClearFinalFunc(func->right);
}

void Clear() {
  modern_schema.clear();
  sort_atts.clear();
  tables.clear();
  groupingAtts.clear();
  attsToSelect.clear();

  AndList *temp = boolean;
  while(temp != NULL) {
    OrList *or_list = temp->left;
    while(or_list != NULL) {
      auto *cop = or_list->left;
      delete cop->left;
      delete cop->right;
      delete cop;
      auto *prev = or_list;
      or_list = or_list->rightOr;
      delete prev;
    }
    auto *prev = temp;
    temp = temp->rightAnd;
    delete prev;
  }
  boolean = NULL;

  ClearFinalFunc(finalFunction);
  finalFunction = NULL;
}

int main() {
  TableOperation table_operation(statistics, catalog);
  bool is_exit = false;
  cout << "Welcome to the S&A (Simple & Awesome) database engine!! Enter your query "
          "below: \n";

  while (!is_exit) {
    cout << "\nSADB>> ";
    if (yyparse() != 0) {
      cout << "It seems you typed in something wrong. Try again! \n";
      continue;
    }
    double begin = time(NULL);
    if (query_type == 0) {
      table_operation.Query();
    } else if (query_type == 1) {
      table_operation.Create();
    } else if (query_type == 2) {
      table_operation.Insert();
    } else if (query_type == 3) {
      table_operation.Drop();
    } else if (query_type == 4) {
      cout << "Exiting!\n";
      is_exit = true;
    } else if (query_type == 5) {
      cout << "Setting output mode: " << output_mode << "\n";
      if (output_mode == 2) {
        cout << "Output file name: " << output_file << "\n";
      }
    } else {
      cout << "Invalid command. Try Again!\n";
    }
    double end = time(NULL);
    cout << "Time Taken: " << (end - begin) << " secs\n";
    Clear();
  }
}