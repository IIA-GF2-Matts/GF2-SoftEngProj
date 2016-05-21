#ifndef names_h
#define names_h

#include <string>
#include <set>

#include "cistring.h"


typedef cistring namestring;
typedef std::set<namestring>::const_iterator name;

const name blankname;       /* special name (defaults to end iterator)*/

class names{

  std::set<namestring> _nameSet;


public:
  name lookup (namestring str);
    /* Returns the internal representation of the name given in character  */
    /* form.  If the name is not already in the name table, it is          */
    /* automatically inserted.                                             */

  name cvtname (namestring str);
    /* Returns the internal representation of the name given in character  */
    /* form.  If the name is not in the name table then 'blankname' is     */
    /* returned.                                                           */

  void writename (name id);
    /* Prints out the given name on the console                            */

  int namelength (name id);
    /* Returns length ie number of characters in given name                */

  names (void);
    /* names initialises the name table.  This procedure is called at      */
    /* system initialisation before any of the above procedures/functions  */
    /* are used.                                                           */
};

#endif /* names_h */
