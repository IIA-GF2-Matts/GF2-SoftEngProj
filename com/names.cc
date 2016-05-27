
#include <set>

#include "names.h"


const name blankname;
const namestring blanknamestr = "(blank)";


/** Initialises the name table.
 *
 * @author Diesel
 */
names::names(void) {
}


/** Returns the internal representation of the name given in character form.
 *  If the name doesn't already exist in the table it is added.
 *
 * @author Diesel
 */
name names::lookup (namestring str)
{
	if (str == "") return blankname;

	return _nameSet.insert(str).first;
}


/** Returns the internal representation of the name given in character form.
 *
 * @author Diesel
 */
name names::cvtname (namestring str)
{
	if (str == "") return blankname;

	return _nameSet.find(str);
}


/** Returns the string representation of the name id
 *
 * @author Diesel
 */
const namestring& names::namestr(name id) const {
	return (id == blankname) ? blanknamestr : *id;
}


/** Returns the length of the string corresponding to the id.
 *
 * @author Diesel
 */
int names::namelength (name id)
{
	if (id == blankname)
		return 0;

	return id->length();
}
