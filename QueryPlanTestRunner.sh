make clean
make QueryNode_Test
./QueryNode_Test < QueryPlanTest/q1.txt > QueryPlanTestOut/q1-out.txt
./QueryNode_Test < QueryPlanTest/q2.txt > QueryPlanTestOut/q2-out.txt
./QueryNode_Test < QueryPlanTest/q3.txt > QueryPlanTestOut/q3-out.txt
./QueryNode_Test < QueryPlanTest/q4.txt > QueryPlanTestOut/q4-out.txt
./QueryNode_Test < QueryPlanTest/q5.txt > QueryPlanTestOut/q5-out.txt
./QueryNode_Test < QueryPlanTest/q6.txt > QueryPlanTestOut/q6-out.txt
./QueryNode_Test < QueryPlanTest/q7.txt > QueryPlanTestOut/q7-out.txt
./QueryNode_Test < QueryPlanTest/q8.txt > QueryPlanTestOut/q8-out.txt
./QueryNode_Test < QueryPlanTest/q9.txt > QueryPlanTestOut/q9-out.txt
./QueryNode_Test < QueryPlanTest/q10.txt > QueryPlanTestOut/q10-out.txt
echo "==================================="
echo "All output can be found in the dir QueryPlanTestOut/"
echo "==================================="