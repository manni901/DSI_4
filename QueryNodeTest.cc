#include <iostream>
#include <cstdio>
#include "QueryNode.h"
#include "QueryPlan.h"
#include "ParseUtil.h"
#include <unordered_map>
#include <memory>

using namespace std;

extern "C" struct YY_BUFFER_STATE *yy_scan_string(const char*);
extern "C" int yyparse(void);
extern struct AndList *boolean;

int main() {
    cout << "Enter query: \n";
    yyparse();
    cout << "Parsed input correctly\n";
    QueryPlan plan("Statistics.txt");
    cout << "\n\n\n\n===========================================\n";
    cout << "Query plan: \n";
    plan.Print();
}