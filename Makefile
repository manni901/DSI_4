CC = g++-5 -Wno-deprecated -ggdb -std=c++14 

# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file.
GTEST_DIR = googletest

# Where to find user code.
USER_DIR = .

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread

# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
TESTS = HeapFile_Test BigQ_Test

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h


# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CC) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CC) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^


tag = -i

ifdef linux
tag = -n
endif

a41.out: Statistics.o y.tab.o lex.yy.o test4-1.o
	$(CC) -o a41.out Statistics.o y.tab.o lex.yy.o test4-1.o -ll

a3.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o HeapFile.o SortedFile.o RelOp.o Function.o Pipe.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o test3.o
	$(CC) -o a3.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o HeapFile.o SortedFile.o RelOp.o Function.o Pipe.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o test3.o -ll -lpthread

a22.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o HeapFile.o SortedFile.o Pipe.o y.tab.o lex.yy.o test2-2.o
	$(CC) -o a22.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o HeapFile.o SortedFile.o Pipe.o y.tab.o lex.yy.o test2-2.o -lfl -lpthread

a21.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o HeapFile.o SortedFile.o Pipe.o y.tab.o lex.yy.o test2-1.o
	$(CC) -o a21.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o HeapFile.o SortedFile.o Pipe.o y.tab.o lex.yy.o test2-1.o -lfl -lpthread

test.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o HeapFile.o SortedFile.o Pipe.o y.tab.o lex.yy.o test.o
	$(CC) -o test.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o HeapFile.o SortedFile.o Pipe.o y.tab.o lex.yy.o test.o -lfl -lpthread
	
main: Record.o Comparison.o ComparisonEngine.o Schema.o File.o y.tab.o lex.yy.o main.o
	$(CC) -o main Record.o Comparison.o ComparisonEngine.o Schema.o File.o y.tab.o lex.yy.o main.o -lfl

HeapFile_Test : Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o HeapFile.o y.tab.o lex.yy.o HeapFile.o SortedFile.o Pipe.o HeapFile_Test.o gtest_main.a
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@

BigQ_Test : Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o HeapFile.o y.tab.o lex.yy.o HeapFile.o SortedFile.o Pipe.o BigQ_Test.o gtest_main.a
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@		

test4-1.o: test4-1.cc
	$(CC) -g -c test4-1.cc

test3.o: test3.cc
	$(CC) -g -c test3.cc

test2-2.o: test2-2.cc
	$(CC) -g -c test2-2.cc

test2-1.o: test2-1.cc
	$(CC) -g -c test2-1.cc

test.o: test.cc
	$(CC) -g -c test.cc

main.o: main.cc
	$(CC) -g -c main.cc
	
Comparison.o: Comparison.cc
	$(CC) -g -c Comparison.cc
	
ComparisonEngine.o: ComparisonEngine.cc
	$(CC) -g -c ComparisonEngine.cc
	
DBFile.o: DBFile.cc
	$(CC) -g -c DBFile.cc

HeapFile.o: HeapFile.cc
	$(CC) -g -c HeapFile.cc

SortedFile.o: SortedFile.cc
	$(CC) -g -c SortedFile.cc

RelOp.o: RelOp.cc
	$(CC) -g -c RelOp.cc

Function.o: Function.cc
	$(CC) -g -c Function.cc

HeapFile_Test.o : HeapFile_Test.cc
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c HeapFile_Test.cc

BigQ_Test.o : BigQ_Test.cc
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c BigQ_Test.cc

File.o: File.cc
	$(CC) -g -c File.cc

Record.o: Record.cc
	$(CC) -g -c Record.cc

Schema.o: Schema.cc
	$(CC) -g -c Schema.cc

Pipe.o: Pipe.cc
	$(CC) -g -c Pipe.cc

BigQ.o: BigQ.cc
	$(CC) -g -c BigQ.cc

Statistics.o: Statistics.cc
	$(CC) -g -c Statistics.cc
	
y.tab.o: Parser.y
	yacc -d Parser.y
	#sed $(tag) y.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" 
	g++ -c y.tab.c

yyfunc.tab.o: ParserFunc.y
	yacc -p "yyfunc" -b "yyfunc" -d ParserFunc.y
	#sed $(tag) yyfunc.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" 
	g++ -c yyfunc.tab.c

lex.yy.o: Lexer.l
	lex  Lexer.l
	gcc -c lex.yy.c

lex.yyfunc.o: LexerFunc.l
	lex -Pyyfunc LexerFunc.l
	gcc  -c lex.yyfunc.c

clean: 
	rm -f *.o
	rm -f *.out
	rm -f y.tab.*
	rm -f yyfunc.tab.*
	rm -f lex.yy.*
	rm -f lex.yyfunc*
	rm -f $(TESTS) gtest.a gtest_main.a *.o
	#rm -f *.bin
	rm -f *.meta
