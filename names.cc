#include "names.h"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;


/* Name storage and retrieval routines */

names::names(void)  /* the constructor */
{
}

name names::lookup (namestring str)
{
	if (str == "") return blankname;

	return _nameSet.insert(str).first;
}

name names::cvtname (namestring str)
{
	if (str == "") return blankname;

	return _nameSet.find(str);
}

void names::writename (name id)
{
	if (id == blankname) {
		std::cout << "(blank)";
	}
	else {
		std::cout << *id;
	}
}

int names::namelength (name id)
{
	if (id == blankname)
		return 0;
	
	return id->length();
}
