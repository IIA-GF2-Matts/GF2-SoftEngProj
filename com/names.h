
#ifndef GF2_NAMES_H
#define GF2_NAMES_H

#include <set>

#include "cistring.h"


typedef cistring namestring;
typedef std::set<namestring>::const_iterator name;

extern const name blankname;       /* special name (defaults to end iterator)*/
extern const namestring blanknamestr;

/** Names Table Class
 *
 * Stores the names using the STL `std::set` container.
 * The type stored is `cistring`, which is a case-insensitive variant on `std::string`.
 * Name IDs are in fact `const_iterator`s to items in the set, it's worth noting that
 *   the standard enforces that changes to the set won't change an items iterator.
 * `blankname` is a special case, stored as the `end()` iterator of the set. This
 *   cannot be dereferenced or used like the other iterators.
 *
 *  @author Gee, Diesel
 */
class names{
private:
  std::set<namestring> _nameSet;

public:
  /** Returns the internal representation of the name given in character form.
   *  If the name doesn't already exist in the table it is added.
   *
   * @param[in]  str   The name in string form
   * @return     The name's identifier.
   */
  name lookup (namestring str);

  /** Returns the internal representation of the name given in character form.
   *
   * @param[in]  str   The name in string form
   * @return     The name's identifier, or blankname if it isn't in the table.
   */
  name cvtname (namestring str);

  /** Returns the string representation of the name id
   *  Strings are capitalised as per their first use in the names table
   *
   * @param[in]  id    The name id
   * @return     The original string of the name id, or "(blank)" if blankname.
   */
  const namestring& namestr(name id) const;

  /** Returns the length of the string corresponding to the id.
   *
   * @param[in]  id    The name id
   * @return     The length of the string in the name table, or zero for blankname.
   */
  int namelength (name id);

  /** Initialises the name table.
   */
  names (void);
};

#endif /* GF2_NAMES_H */
