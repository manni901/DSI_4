#include "TableOperation.h"

bool TableOperation::IsExistingTable(string table_name) {
  if(dropped_tables_.find(table_name) != dropped_tables_.end()) {
      return false;
  }
  ifstream in(catalog_file_);
  if (!in.good()) {
    cout << "Cannot open catalog file! Exiting!! \n";
    exit(1);
  }
  string prev = "", curr = "";
  while (in >> curr) {
    if (prev == "BEGIN" && curr == table_name) {
      return true;
    }
    prev = curr;
  }
  return false;
}

bool TableOperation::Create() {
  fType file_type = fType(db_file_type);
  if (IsExistingTable(mytable)) {
    cout << "Table: " << mytable << " already exists!";
    return false;
  }
  DBFile db_file;
  vector<Attribute> attributes;
  for (auto &p : modern_schema) {
    attributes.push_back({p.first, Type(p.second)});
  }
  Schema schema(mytable, attributes.size(), attributes);
  string db_file_name = GetDBFileName(mytable);
  int create_status = 0;
  switch (file_type) {
  case heap:
    create_status = db_file.Create(db_file_name.c_str(), heap);
    break;

  case sorted: {
    OrderMaker o;
    for (auto &att : sort_atts) {
      int index = schema.Find(att);
      if (index == -1) {
        cout << "Invalid sort order!";
        return false;
      }
      o.AddAtt(index, schema.FindType(att));
    }
    struct {
      OrderMaker *o;
      int l;
    } startup = {&o, RUN_LEN};
    create_status = db_file.Create(db_file_name.c_str(), sorted, &startup);
    break;
  }

  default:
    break;
  }
  if (create_status == 0) {
    cout << "Error in creating table!";
    return false;
  }
  schema.WriteToCatalog(catalog_file_);
  return true;
}

bool TableOperation::Insert() {
  if (!IsExistingTable(mytable)) {
    cout << "Table: " << mytable << " does not exist!";
    return false;
  }
  string db_file_name = GetDBFileName(mytable);
  DBFile db_file;
  db_file.Open(db_file_name.c_str());
  Schema schema(catalog_file_, mytable);
  db_file.Load(schema, insert_file_name.c_str());
  db_file.Close();
  return true;
}

bool TableOperation::Drop() {
  if (!IsExistingTable(mytable)) {
    cout << "Table: " << mytable << " does not exist!";
    return false;
  }
  string db_file_name = GetDBFileName(mytable);
  remove(db_file_name.c_str());
  db_file_name += ".meta";
  remove(db_file_name.c_str());
  dropped_tables_.insert(mytable);
  return true;
}

void TableOperation::Query() {
  QueryPlan plan(stat_file_);
  plan.Print();
  if (output_mode != 0) {
    plan.Execute();
    plan.Finish();
  }
}