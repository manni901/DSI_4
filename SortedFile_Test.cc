#include "HeapFile.h"
#include "DBFile.h"
#include "gtest/gtest.h"
#include "cstdio"
#include "ParseUtil.h"

extern "C" {
  int yyparse(void);   // defined in y.tab.c
  struct yy_buffer_state* yy_scan_string (const char *yy_str  );
  void yy_delete_buffer (struct yy_buffer_state* b  );
}

extern struct AndList *boolean;

namespace {

// Tests number of records in lineitem.tbl
// Check if the correct number of records 
// are loaded or not.
TEST(SortedFileTest, LoadCountTest) {
  const char* file_path = "10M/lineitem.tbl";
  string table_bin_path = "lineitem-test.bin";
  int expected_records = 60175;

  // suck up the schema from the file
  Schema lineitem ("catalog_old", "lineitem");
  string sort_att = "l_orderkey";
  OrderMaker o;
  o.AddAtt(lineitem.Find(sort_att), lineitem.FindType(sort_att));

  struct {
      OrderMaker *o;
      int l;
    } startup = {&o, RUN_LEN};

  DBFile dbfile;
  dbfile.Create (table_bin_path.c_str(), sorted, &startup);

  dbfile.Load (lineitem, file_path);
  dbfile.Close ();

  dbfile.Open(table_bin_path.c_str());
  dbfile.MoveFirst();

  int actual_records = 0;
  Record temp;
  while (dbfile.GetNext (temp) == 1) {
    actual_records++;
  }
  dbfile.Close();

  EXPECT_EQ(actual_records, expected_records);
  ASSERT_EQ(std::remove(table_bin_path.c_str()), 0);
  string meta_file = table_bin_path + ".meta";
  ASSERT_EQ(std::remove(meta_file.c_str()), 0);
}

// Tests cnf in lineitem.tbl
// Also checks the number of results returned by sorting
// in addition to the result count.
TEST(SortedFileTest, CNFTest) {
  const char* file_path = "10M/lineitem.tbl";
  string table_bin_path = "lineitem-test.bin";
  
  // Subtracting 13 to account for our search criteria (l_orderkey > 3)
  int expected_records = 60175 - 13;

  // suck up the schema from the file
  Schema lineitem ("catalog_old", "lineitem");
  string sort_att = "l_orderkey";
  OrderMaker o;
  o.AddAtt(lineitem.Find(sort_att), lineitem.FindType(sort_att));

  struct {
      OrderMaker *o;
      int l;
    } startup = {&o, RUN_LEN};

  DBFile dbfile;
  dbfile.Create (table_bin_path.c_str(), sorted, &startup);

  dbfile.Load (lineitem, file_path);
  dbfile.Close ();

  dbfile.Open(table_bin_path.c_str());
  dbfile.MoveFirst();

  auto string_buffer = yy_scan_string("SELECT ll FROM lineitem AS l WHERE (l_orderkey > 3);");
  yyparse();
  yy_delete_buffer(string_buffer);

  auto parse_vector = ParseUtil::AndListToVector(boolean);
  BitSet bitset;
  bitset.set(0);

  CNF myComparison;
  Record literal;
  myComparison.GrowFromParseTree(parse_vector, bitset, &lineitem, literal);

  int actual_records = 0;
  Record temp;
  while (dbfile.GetNext (temp, myComparison, literal) == 1) {
    actual_records++;
  }
  dbfile.Close();

  EXPECT_EQ(actual_records, expected_records);
  ASSERT_EQ(std::remove(table_bin_path.c_str()), 0);
  string meta_file = table_bin_path + ".meta";
  ASSERT_EQ(std::remove(meta_file.c_str()), 0);
}

} // namespace 
