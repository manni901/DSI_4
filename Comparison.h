#ifndef COMPARISON_H
#define COMPARISON_H

#include <fstream>
#include <iostream>
#include <vector>
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "File.h"
#include "Record.h"
#include "Schema.h"

// This stores an individual comparison that is part of a CNF
class Comparison {

  friend class ComparisonEngine;
  friend class CNF;
  friend class OrderMaker;

  Target operand1;
  int whichAtt1;
  Target operand2;
  int whichAtt2;

  Type attType;

  CompOperator op;

public:
  Comparison();

  // copy constructor
  Comparison(const Comparison &copyMe);

  // print to the screen
  void Print();
};

class Schema;
class CNF;

// This structure encapsulates a sort order for records
class OrderMaker {

  friend class ComparisonEngine;
  friend class CNF;

  // Operator overload to read and write the order maker members numAtts,
  // whichAtts and whichTypes to file.
  friend std::ostream& operator<<(std::ostream& os, const OrderMaker& myorder);
  friend std::istream& operator>>(std::istream& is, OrderMaker& myorder);

  int numAtts;

  int whichAtts[MAX_ANDS];
  Type whichTypes[MAX_ANDS];

public:
  // creates an empty OrdermMaker
  OrderMaker();
  OrderMaker(const OrderMaker &other) { Copy(other); }


  OrderMaker &operator=(const OrderMaker &other) { 
    Copy(other);
    return *this;
  }
  // create an OrderMaker that can be used to sort records
  // based upon ALL of their attributes
  OrderMaker(Schema *schema);

  void AddAtt(int att_index, Type type) {
    whichAtts[numAtts] = att_index;
    whichTypes[numAtts] = type;
    numAtts++;
  }

  // print to the screen
  void Print();

  // Build query order and cnf_order for sorting comparison.
  void BuildQueryOrder(OrderMaker &query_order, OrderMaker &cnf_order,
                       CNF &cnf);

  void GetAtts(std::vector<int> &atts_to_keep) {
    for(int i = 0; i < numAtts; i++) {
      atts_to_keep.push_back(whichAtts[i]);
    }
  }

private:
  void Copy(const OrderMaker &other) {
    numAtts = other.numAtts;
	std::cout << "Inside copy " << other.numAtts << "\n";
    for (int i = 0; i < numAtts; ++i) {
      whichAtts[i] = other.whichAtts[i];
      whichTypes[i] = other.whichTypes[i];
    }
  }
};

class Record;

// This structure stores a CNF expression that is to be evaluated
// during query execution

class CNF {

  friend class ComparisonEngine;
  friend class OrderMaker;

  Comparison orList[MAX_ANDS][MAX_ORS];

  int orLens[MAX_ANDS];
  int numAnds;

public:
  // this returns an instance of the OrderMaker class that
  // allows the CNF to be implemented using a sort-based
  // algorithm such as a sort-merge join.  Returns a 0 if and
  // only if it is impossible to determine an acceptable ordering
  // for the given comparison
  int GetSortOrders(OrderMaker &left, OrderMaker &right);

  // print the comparison structure to the screen
  void Print();

  // this takes a parse tree for a CNF and converts it into a 2-D
  // matrix storing the same CNF expression.  This function is applicable
  // specifically to the case where there are two relations involved
  void GrowFromParseTree(struct AndList *parseTree, Schema *leftSchema,
                         Schema *rightSchema, Record &literal);

  // version of the same function, except that it is used in the case of
  // a relational selection over a single relation so only one schema is used
  void GrowFromParseTree(struct AndList *parseTree, Schema *mySchema,
                         Record &literal);
};

#endif
