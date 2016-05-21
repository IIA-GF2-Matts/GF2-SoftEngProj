
#ifndef GF2_NAMES_H
#define GF2_NAMES_H

#include <set>

#include "cistring.h"


typedef cistring namestring;
typedef std::set<namestring>::const_iterator name;

extern const name blankname;       /* special name (defaults to end iterator)*/
extern const namestring blanknamestr;

/// Names Table Class
/// Stores the names using the STL `std::set` container.
/// The type stored is `cistring`, which is a case-insensitive variant on `std::string`.
/// Name IDs are in fact `const_iterator`s to items in the set, it's worth noting that
///   the standard enforces that changes to the set won't change an items iterator.
/// `blankname` is a special case, stored as the `end()` iterator of the set. This
///   cannot be dereferenced or used like the other iterators.
class names{
private:
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

  const namestring& namestr(name id) const;
    /* Returns the string representation of the name id                    */

  int namelength (name id);
    /* Returns length ie number of characters in given name                */

  names (void);
    /* names initialises the name table.  This procedure is called at      */
    /* system initialisation before any of the above procedures/functions  */
    /* are used.                                                           */
};

#endif /* GF2_NAMES_H */
