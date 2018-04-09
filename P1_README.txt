Project 1, Prashant Mishra (UFID 7879-9401):

I have added the following new code files:
- DBFile_Internal.h
- HeapFile.h
- HeapFile.cc
- HeapFile_Test.cc (contains 2 gtest based unit tests)

And edited:
- DBFile.h
- DBFile.cc
- Makefile (Added executable for HeapFile_Test and changed executable main and test.out. 
            :Also added gtest support here)

To run HeapFile_Test simple run:

$ make HeapFile_Test
$ ./HeapFile_Test

For testing the given queries from README, I have created a new folder TestData-P1/
This folder contains a text file for each query for input, for e.g q1.txt.
To get the output for q1 in a txt file, you can run the following:
Note: before running this command, please change the tpch directory for tables according to you.

$ make test.out
$ ./test.out < TestData-P1/q1.txt > TestData-P1/q1-out.txt

The '<' operator means stdin will be treated as if getting values from file q1.txt
and '>' operator means stdout will be redirected to q1-out.txt.

This output data is already present in TestData-P1.
To run this test for all the 12 queries, I have made a small shell script p1_test_runner.sh
This can be easily run by using:

$ sh p1_test_runner.sh