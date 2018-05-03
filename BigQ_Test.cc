#include "BigQ.h"
#include "DBFile.h"
#include "HeapFile.h"
#include "cstdio"
#include "gtest/gtest.h"
#include "ParseUtil.h"

extern "C" {
int yyparse(void); // defined in y.tab.c
struct yy_buffer_state *yy_scan_string(const char *yy_str);
void yy_delete_buffer(struct yy_buffer_state *b);
}

extern struct AndList * boolean;

namespace {

// Tests that the same number of records are output by BigQ
// and that the records are sorted by the given sort order.
TEST(BigQTest, SimpleSortTest) {
  const char *file_path = "10M/lineitem.tbl";
  const char *table_bin_path = "lineitem-test.bin";
  // suck up the schema from the file
  Schema lineitem("catalog_old", "lineitem");

  // To parse from string instead of stdin
  // (https://lists.gnu.org/archive/html/help-bison/2006-01/msg00054.html)
  auto string_buffer = yy_scan_string("SELECT ll FROM lineitem AS l WHERE (l_partkey > 5);");
  yyparse();
  yy_delete_buffer(string_buffer);

  // grow the CNF expression from the parse tree
  OrderMaker sortorder;
  CNF myComparison;
  Record literal;
  auto parse_vector = ParseUtil::AndListToVector(boolean);
  BitSet bitset;
  bitset.set(0);
  myComparison.GrowFromParseTree(parse_vector, bitset, &lineitem, literal);
  OrderMaker dummy;
  myComparison.GetSortOrders(sortorder, dummy);

  DBFile dbfile;
  dbfile.Create(table_bin_path, heap, NULL);

  dbfile.Load(lineitem, file_path);
  dbfile.Close();

  dbfile.Open(table_bin_path);
  dbfile.MoveFirst();

  int buffsz = 100; // pipe cache size
  Pipe input(buffsz);
  Pipe output(buffsz);

  int produce_records = 0;
  thread producer = thread([&dbfile, &input, &produce_records]() {
    Record rec;
    while (dbfile.GetNext(rec) == 1) {
      input.Insert(&rec);
      produce_records++;
    }
    input.ShutDown();
  });

  int consume_records = 0;
  int sort_errors = 0;
  thread consumer =
      thread([&sortorder, &output, &consume_records, &sort_errors]() {
        ComparisonEngine ce;
        Record prev_rec;
        Record curr_rec;
        output.Remove(&prev_rec);
        consume_records++;
        while (output.Remove(&curr_rec)) {
          if (ce.Compare(&prev_rec, &curr_rec, &sortorder) == 1) {
            sort_errors++;
          }
          prev_rec = curr_rec;
          consume_records++;
        }
      });

  int runlen = 5;
  BigQ bigQ(input, output, sortorder, runlen);

  bigQ.End();
  consumer.join();
  producer.join();
  dbfile.Close();

  // Check that all records are processed by BigQ.
  std::cout << consume_records << " " << produce_records << " " << sort_errors << "\n";
  EXPECT_EQ(consume_records, produce_records);
  // Check that there are not sorting errors.
  EXPECT_EQ(sort_errors, 0);
  ASSERT_EQ(std::remove(table_bin_path), 0);
}

} // namespace
