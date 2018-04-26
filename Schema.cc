#include "Schema.h"
#include <fstream>

int Schema :: Find (string &attName) {
	for (int i = 0; i < numAtts; i++) {
		if (attName == myAtts[i].name) {
			return i;
		}
	}

	// if we made it here, the attribute was not found
	return -1;
}

Type Schema :: FindType (string &attName) {
	for (int i = 0; i < numAtts; i++) {
		if (attName == myAtts[i].name) {
			return myAtts[i].myType;
		}
	}

	// if we made it here, the attribute was not found
	return Int;
}

int Schema :: GetNumAtts () {
	return numAtts;
}

Attribute *Schema :: GetAtts () {
	return myAtts.data();
}


Schema :: Schema (string fpath, int num_atts, std::vector<Attribute>& atts) {
	fileName = fpath;
	numAtts = num_atts;
	myAtts = move(atts);
}

Schema :: Schema (string fName, string relName, string alias) {

	ifstream foo(fName);
	
	// this is enough space to hold any tokens
	string space;

	foo >> space;
	int totscans = 1;

	// see if the file starts with the correct keyword
	if (space != "BEGIN") {
		cout << "Unfortunately, this does not seem to be a schema file.\n";
		exit (1);
	}	
		
	while (1) {

		// check to see if this is the one we want
		foo >> space;
		totscans++;
		if (space != relName) {

			bool is_begin = false;
			// it is not, so suck up everything to past the BEGIN
			while (foo >> space) {
				totscans++;
				if (space == "BEGIN") {
					is_begin = true;
					break;
				}
			}
			if (!is_begin) {
				cerr << "Could not find the schema for the specified relation.\n";
				exit (1);
			}

		// otherwise, got the correct file!!
		} else {
			break;
		}
	}

	// suck in the file name
	foo >> space;
	totscans++;
	fileName = space;

	// count the number of attributes specified
	numAtts = 0;
	while (1) {
		foo >> space;
		if (space == "END") {
			break;		
		} else {
			foo >> space;
			numAtts++;
		}
	}

	// now actually load up the schema
	foo.close();
	foo.open(fName);

	// go past any un-needed info
	for (int i = 0; i < totscans; i++) {
		foo >> space;
	}

	// and load up the schema
	myAtts.resize(numAtts);
	for (int i = 0; i < numAtts; i++ ) {

		// read in the attribute name
		foo >> space;
		string n = alias == "" ? space : (alias + "." + space);
		myAtts[i].name = n; //strdup (space);

		// read in the attribute type
		foo >> space;
		if (space == "Int") {
			myAtts[i].myType = Int;
		} else if (space == "Double") {
			myAtts[i].myType = Double;
		} else if (space == "String") {
			myAtts[i].myType = String;
		} else {
			cout << "Bad attribute type for " << myAtts[i].name << "\n";
			exit (1);
		}
	}

	foo.close();
}

