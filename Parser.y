 
%{

	#include "ParseTree.h"
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <iostream>
	#include <vector>
	#include <unordered_set>

	extern "C" int yylex();
	extern "C" int yyparse();
	extern "C" void yyerror(char *s);
  
	// these data structures hold the result of the parsing
	struct FuncOperator *finalFunction; // the aggregate function (NULL if no agg)
	std::vector<std::pair<std::string, std::string>> tables; // the list of tables and aliases in the query
	struct AndList *boolean; // the predicate in the WHERE clause
	std::vector<std::string> groupingAtts; // grouping atts (NULL if no grouping)
	std::vector<std::string> attsToSelect; // the set of attributes in the SELECT (NULL if no such atts)
	int distinctAtts; // 1 if there is a DISTINCT in a non-aggregate query 
	int distinctFunc;  // 1 if there is a DISTINCT in an aggregate query
	std::string mytable; // table name for create table.
	int query_type;
	std::vector<std::pair<std::string, int>> modern_schema;
	int db_file_type;
	std::vector<std::string> sort_atts;
	std::string insert_file_name;
	std::string output_file;
	int output_mode;

%}

// this stores all of the types returned by production rules
%union {
 	struct FuncOperand *myOperand;
	struct FuncOperator *myOperator; 
	struct ComparisonOp *myComparison;
	struct Operand *myBoolOperand;
	struct OrList *myOrList;
	struct AndList *myAndList;
	char *actualChars;
	char whichOne;
	int atttype;
}



%token <actualChars> Name
%token <actualChars> Float
%token <actualChars> Inti
%token <actualChars> Stringg
%token SELECT
%token GROUP 
%token DISTINCT
%token BY
%token FROM
%token WHERE
%token SUM
%token AS
%token AND
%token OR
%token CREATE
%token TABLE
%token INSERT
%token INTO
%token DROP
%token EXIT
%token SORTED
%token HEAP
%token ON
%token INTEGERR
%token DOUBLEE
%token STRINGG
%token SET
%token OUTPUT
%token NONE
%token STDOUT

%type <myOrList> OrList
%type <myAndList> AndList
%type <myOperand> SimpleExp
%type <myOperator> CompoundExp
%type <whichOne> Op 
%type <myComparison> BoolComp
%type <myComparison> Condition
%type <myBoolOperand> Literal
%type <atttype> Typ

%start SQL


//******************************************************************************
// SECTION 3
//******************************************************************************
/* This is the PRODUCTION RULES section which defines how to "understand" the 
 * input language and what action to take for each "statment"
 */

%%

SQL: SELECT WhatIWant FROM Tables WHERE AndList ';'
{
	query_type = 0;
	boolean = $6;	
	YYACCEPT;
}

| SELECT WhatIWant FROM Tables WHERE AndList GROUP BY GroupAtts ';'
{
	query_type = 0;
	boolean = $6;	
	YYACCEPT;
}

| CREATE TABLE Name '(' AttPair ')' AS TableType ';'
{
	query_type = 1;
	mytable = $3;
	YYACCEPT;
}
| INSERT File INTO Name ';'
{
	query_type = 2;
	mytable = $4;
	YYACCEPT;
}
| DROP TABLE Name ';'
{
	query_type = 3;
	mytable = $3;
	YYACCEPT;
}
| SET OUTPUT Output ';'
{
	query_type = 5;
	YYACCEPT;
}
| EXIT ';'
{
	query_type = 4;
	YYACCEPT;
}
;

Output: STDOUT
{
	output_mode = 1;
}
| NONE
{
	output_mode = 0;
}
| Stringg
{
	output_mode = 2;
	output_file = $1;
}
;

File: Stringg
{
	insert_file_name = $1;
}
| Name
{
	insert_file_name = $1;
}
;

TableType: HEAP
{
	db_file_type = 0;
} 
| SORTED ON SortAtts
{
	db_file_type = 1;
};

WhatIWant: Function ',' Atts 
{
	distinctAtts = 0;
}

| Function
{
	distinctAtts = 0;
}

| Atts 
{
	distinctAtts = 0;
	finalFunction = NULL;
}

| DISTINCT Atts
{
	distinctAtts = 1;
	finalFunction = NULL;
};

Function: SUM '(' CompoundExp ')'
{
	distinctFunc = 0;
	finalFunction = $3;
}

| SUM DISTINCT '(' CompoundExp ')'
{
	distinctFunc = 1;
	finalFunction = $4;
};

AttPair: Name Typ
{
	modern_schema.emplace_back($1, $2);
}
| AttPair ',' Name Typ
{
	modern_schema.emplace_back($3, $4);
}
;

Typ: INTEGERR
{
	$$ = 0;
}
| DOUBLEE
{
	$$ = 1;
}
| STRINGG
{
	$$ = 2;
}
;

GroupAtts: Name
{
	groupingAtts.push_back($1);
} 

| GroupAtts ',' Name
{
	groupingAtts.push_back($3);
};

SortAtts: Name
{
	sort_atts.push_back($1);
} 

| SortAtts ',' Name
{
	sort_atts.push_back($3);
};

Atts: Name
{
	attsToSelect.push_back($1);
} 

| Atts ',' Name
{
	attsToSelect.push_back($3);
};

Tables: Name AS Name 
{
	tables.emplace_back($1, $3);
}

| Tables ',' Name AS Name
{
	tables.emplace_back($3, $5);
}



CompoundExp: SimpleExp Op CompoundExp
{
	$$ = (struct FuncOperator *) malloc (sizeof (struct FuncOperator));	
	$$->leftOperator = (struct FuncOperator *) malloc (sizeof (struct FuncOperator));
	$$->leftOperator->leftOperator = NULL;
	$$->leftOperator->leftOperand = $1;
	$$->leftOperator->right = NULL;
	$$->leftOperand = NULL;
	$$->right = $3;
	$$->code = $2;	

}

| '(' CompoundExp ')' Op CompoundExp
{
	$$ = (struct FuncOperator *) malloc (sizeof (struct FuncOperator));	
	$$->leftOperator = $2;
	$$->leftOperand = NULL;
	$$->right = $5;
	$$->code = $4;	

}

| '(' CompoundExp ')'
{
	$$ = $2;

}

| SimpleExp
{
	$$ = (struct FuncOperator *) malloc (sizeof (struct FuncOperator));	
	$$->leftOperator = NULL;
	$$->leftOperand = $1;
	$$->right = NULL;	

}

| '-' CompoundExp
{
	$$ = (struct FuncOperator *) malloc (sizeof (struct FuncOperator));	
	$$->leftOperator = $2;
	$$->leftOperand = NULL;
	$$->right = NULL;	
	$$->code = '-';

}
;

Op: '-'
{
	$$ = '-';
}

| '+'
{
	$$ = '+';
}

| '*'
{
	$$ = '*';
}

| '/'
{
	$$ = '/';
}
;

AndList: '(' OrList ')' AND AndList
{
        // here we need to pre-pend the OrList to the AndList
        // first we allocate space for this node
        $$ = (struct AndList *) malloc (sizeof (struct AndList));

        // hang the OrList off of the left
        $$->left = $2;

        // hang the AndList off of the right
        $$->rightAnd = $5;

}

| '(' OrList ')'
{
        // just return the OrList!
        $$ = (struct AndList *) malloc (sizeof (struct AndList));
        $$->left = $2;
        $$->rightAnd = NULL;
}
;

OrList: Condition OR OrList
{
        // here we have to hang the condition off the left of the OrList
        $$ = (struct OrList *) malloc (sizeof (struct OrList));
        $$->left = $1;
        $$->rightOr = $3;
}

| Condition
{
        // nothing to hang off of the right
        $$ = (struct OrList *) malloc (sizeof (struct OrList));
        $$->left = $1;
        $$->rightOr = NULL;
}
;

Condition: Literal BoolComp Literal
{
        // in this case we have a simple literal/variable comparison
        $$ = $2;
        $$->left = $1;
        $$->right = $3;
}
;

BoolComp: '<'
{
        // construct and send up the comparison
        $$ = (struct ComparisonOp *) malloc (sizeof (struct ComparisonOp));
        $$->code = LESS_THAN;
}

| '>'
{
        // construct and send up the comparison
        $$ = (struct ComparisonOp *) malloc (sizeof (struct ComparisonOp));
        $$->code = GREATER_THAN;
}

| '='
{
        // construct and send up the comparison
        $$ = (struct ComparisonOp *) malloc (sizeof (struct ComparisonOp));
        $$->code = EQUALS;
}
;

Literal : Stringg
{
        // construct and send up the operand containing the string
        $$ = (struct Operand *) malloc (sizeof (struct Operand));
        $$->code = STRING;
        $$->value = $1;
}

| Float
{
        // construct and send up the operand containing the FP number
        $$ = (struct Operand *) malloc (sizeof (struct Operand));
        $$->code = DOUBLE;
        $$->value = $1;
}

| Inti
{
        // construct and send up the operand containing the integer
        $$ = (struct Operand *) malloc (sizeof (struct Operand));
        $$->code = INT;
        $$->value = $1;
}

| Name
{
        // construct and send up the operand containing the name
        $$ = (struct Operand *) malloc (sizeof (struct Operand));
        $$->code = NAME;
        $$->value = $1;
}
;


SimpleExp: 

Float
{
        // construct and send up the operand containing the FP number
        $$ = (struct FuncOperand *) malloc (sizeof (struct FuncOperand));
        $$->code = DOUBLE;
        $$->value = $1;
} 

| Inti
{
        // construct and send up the operand containing the integer
        $$ = (struct FuncOperand *) malloc (sizeof (struct FuncOperand));
        $$->code = INT;
        $$->value = $1;
} 

| Name
{
        // construct and send up the operand containing the name
        $$ = (struct FuncOperand *) malloc (sizeof (struct FuncOperand));
        $$->code = NAME;
        $$->value = $1;
}
;

%%

