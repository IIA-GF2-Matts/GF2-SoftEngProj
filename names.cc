#include "names.h"
#include <iostream>
#include <string>
#include <cstdlib>


const name blankname;
const namestring blanknamestr = "(blank)";


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

const namestring& names::namestr(name id) const {
	return (id == blankname) ? blanknamestr : *id;
}


int names::namelength (name id)
{
	if (id == blankname)
		return 0;

	return id->length();
}
