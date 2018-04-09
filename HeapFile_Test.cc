#include "HeapFile.h"
#include "DBFile.h"
#include "gtest/gtest.h"
#include "cstdio"

extern "C" {
  int yyparse(void);   // defined in y.tab.c
  struct yy_buffer_state* yy_scan_string (const char *yy_str  );
  void yy_delete_buffer (struct yy_buffer_state* b  );
}

extern struct AndList *final;

namespace {

// Tests number of records in lineitem.tbl
// Check if the correct number of records 
// are loaded or not.
TEST(HeapFileTest, LoadCountTest) {
  const char* file_path = "10M/lineitem.tbl";
  const char* table_bin_path = "lineitem-test.bin";
  int expected_records = 60175;

  DBFile dbfile;
  dbfile.Create (table_bin_path, heap, NULL);

  // suck up the schema from the file
  Schema lineitem ("catalog", "lineitem");

  dbfile.Load (lineitem, file_path);
  dbfile.Close ();

  dbfile.Open(table_bin_path);
  dbfile.MoveFirst();

  int actual_records = 0;
  Record temp;
  while (dbfile.GetNext (temp) == 1) {
    actual_records++;
  }
  dbfile.Close();

  EXPECT_EQ(actual_records, expected_records);
  ASSERT_EQ(std::remove(table_bin_path), 0);
}

// Tests cnf in nation.tbl
// Also checks the actual content of the result
// in addition to the result count.
TEST(HeapFileTest, CNFTest) {
  const char* file_path = "10M/nation.tbl";
  const char* table_bin_path = "nation-test.bin";
  // suck up the schema from the file
  Schema nation ("catalog", "nation");
  
  // To parse from string instead of stdin
  // (https://lists.gnu.org/archive/html/help-bison/2006-01/msg00054.html)
  auto string_buffer = yy_scan_string ("(n_name = 'INDIA')");
  yyparse();
  yy_delete_buffer (string_buffer);

  // grow the CNF expression from the parse tree 
  CNF myComparison;
  Record literal;
  myComparison.GrowFromParseTree (final, &nation, literal);

  int expected_records = 1;
  std::string expected_record_string = 
    "n_nationkey: [8], n_name: [INDIA], n_regionkey: [2], n_comment: [ss excuses cajole slyly across the packages. deposits print aroun]";

  DBFile dbfile;
  dbfile.Create (table_bin_path, heap, NULL);

  dbfile.Load (nation, file_path);
  dbfile.Close ();

  dbfile.Open(table_bin_path);
  dbfile.MoveFirst();

  int actual_records = 0;
  std::string actual_record_string = "";
  Record temp;
  while (dbfile.GetNext (temp, myComparison, literal) == 1) {
    actual_records++;
    actual_record_string = temp.ToString(&nation);
  }
  dbfile.Close();

  EXPECT_EQ(actual_records, expected_records);
  EXPECT_EQ(actual_record_string, expected_record_string);
  ASSERT_EQ(std::remove(table_bin_path), 0);
}

} // namespace 
