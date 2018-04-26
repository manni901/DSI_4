
#ifndef SCHEMA_H
#define SCHEMA_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"

struct att_pair {
	char *name;
	Type type;
};
struct Attribute {

	std::string name;
	Type myType;
};

class OrderMaker;
class Schema {

	// gives the attributes in the schema
	int numAtts = 0;
	std::vector<Attribute> myAtts;

	// gives the physical location of the binary file storing the relation
	string fileName;

	friend class Record;

public:

	// gets the set of attributes, but be careful with this, since it leads
	// to aliasing!!!
	Attribute *GetAtts ();

	// returns the number of attributes
	int GetNumAtts ();

	// this finds the position of the specified attribute in the schema
	// returns a -1 if the attribute is not present in the schema
	int Find (string& attName);

	// this finds the type of the given attribute
	Type FindType (string& attName);

	// this reads the specification for the schema in from a file
	Schema (string fName, string relName, string alias = "");

	// this composes a schema instance in-memory
	Schema (string fName, int num_atts, std::vector<Attribute>& atts);

	// this constructs a sort order structure that can be used to
	// place a lexicographic ordering on the records using this type of schema
	int GetSortOrder (OrderMaker &order);

	Schema(Schema *copy) {
		numAtts = copy->numAtts;
		myAtts = copy->myAtts;
		fileName = copy->fileName;
	}

	Schema (Schema *left, Schema *right) {
		numAtts = left->numAtts + right->numAtts;
		myAtts = left->myAtts;
		myAtts.insert(myAtts.end(), right->myAtts.begin(), right->myAtts.end());
	}

	string ToString() {
		string ans = "";
		for (int i = 0; i < numAtts; i++) {
			string typ;
			switch(myAtts[i].myType) {
				case Int:
					typ = "Int";
					break;
				case String:
					typ = "String";
					break;
				case Double:
					typ = "Double";
					break;
				default:
					break;
			}
			ans += myAtts[i].name + " " + typ + "\n";
		}
		return ans;
	}

	void Print() {
		cout << "Output Schema:\n";
		cout << ToString();
	}

	void WriteToCatalog(string catalog_file) {
		std::ofstream out(catalog_file, std::ofstream::out | std::ofstream::app);
		out << "\n" << "BEGIN" << "\n" << fileName << "\n" << fileName + ".tbl" << "\n";
		out << ToString();
		out << "END\n";
	}

	~Schema () = default;

};

#endif