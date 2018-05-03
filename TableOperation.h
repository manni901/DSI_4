#ifndef TABLE_OPERATION_H
#define TABLE_OPERATION_H

#include "DBFile.h"
#include "Defs.h"
#include "QueryPlan.h"
#include "Schema.h"
#include <iostream>
#include <ostream>
#include <unordered_set>
#include <vector>
using namespace std;

extern string mytable;
extern vector<pair<string, int>> modern_schema;
extern int db_file_type;
extern vector<string> sort_atts;
extern string insert_file_name;
extern int output_mode;

class TableOperation {
public:
  TableOperation(string stat_file, string catalog_file)
      : stat_file_(stat_file), catalog_file_(catalog_file) {}

  ~TableOperation();

  bool Create();
  bool Insert();
  bool Drop();
  void Query();

private:
  string stat_file_;
  string catalog_file_;

  bool IsExistingTable(string table_name);

  string GetDBFileName(string table_name) {
    return db_file_prefix + mytable + ".bin";
  }

  // Stores dropped tables in current session.
  //(TODO) On closing the db engine, remove these
  // tables from catalog.
  unordered_set<string> dropped_tables_;

  void DeleteDroppedSchema();

  bool GetNextTableSchema(ifstream &in, Schema &schema, string &table_name);
};

#endif