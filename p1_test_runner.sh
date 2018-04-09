# This the test runner for queries for P1
# For each q[n].txt file, the output goes in q[n]-out.txt
# Before running this please update the tpch table files directory in test.cc
# Also make sure that all the tables are loaded for 1G scale.
make test.out
echo "Running Q1, output in TestData-P1/q1-out.txt"
./test.out < TestData-P1/q1.txt > TestData-P1/q1-out.txt
echo "Running Q2, output in TestData-P1/q2-out.txt"
./test.out < TestData-P1/q2.txt > TestData-P1/q2-out.txt
echo "Running Q3, output in TestData-P1/q3-out.txt"
./test.out < TestData-P1/q3.txt > TestData-P1/q3-out.txt
echo "Running Q4, output in TestData-P1/q4-out.txt"
./test.out < TestData-P1/q4.txt > TestData-P1/q4-out.txt
echo "Running Q5, output in TestData-P1/q5-out.txt"
./test.out < TestData-P1/q5.txt > TestData-P1/q5-out.txt
echo "Running Q6, output in TestData-P1/q6-out.txt"
./test.out < TestData-P1/q6.txt > TestData-P1/q6-out.txt
echo "Running Q7, output in TestData-P1/q7-out.txt"
./test.out < TestData-P1/q7.txt > TestData-P1/q7-out.txt
echo "Running Q8, output in TestData-P1/q8-out.txt"
./test.out < TestData-P1/q8.txt > TestData-P1/q8-out.txt
echo "Running Q9, output in TestData-P1/q9-out.txt"
./test.out < TestData-P1/q9.txt > TestData-P1/q9-out.txt
echo "Running Q10, output in TestData-P1/q10-out.txt"
./test.out < TestData-P1/q10.txt > TestData-P1/q10-out.txt
echo "Running Q11, output in TestData-P1/q11-out.txt"
./test.out < TestData-P1/q11.txt > TestData-P1/q11-out.txt
echo "Running Q12, output in TestData-P1/q12-out.txt"
./test.out < TestData-P1/q12.txt > TestData-P1/q12-out.txt
