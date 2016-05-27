/** Autocorrect utility functions
 *
 *  @author Judge
 */


#include <set>
#include <list>
#include <ostream>
#include "names.h"

/** Candidate set of device types
 * Used by the autocorrect to suggest the closest match device type(s)
 */
const std::set<namestring> devicesset = {
  "CLOCK",
  "SWITCH",
  "AND",
  "NAND",
  "OR",
  "NOR",
  "XOR",
  "DTYPE"
};

/** Candidate set of dtype inputs
 * Used by the autocorrect to suggest the closest match input(s)
 */
const std::set<namestring> dtypeinset = {
  "DATA",
  "CLK",
  "SET",
  "CLEAR"
};

/** Takes an input and returns a set containing the
 *  closest matches from a set of candidates, along with the
 *  calculated distance between them and the input. A lower distance means
 *  a closer match.
 *
 * @param[in]  s           The input to match
 * @param[in]  candidates  Set of candidates to match to the input
 * @param[out] matches     Set closest matches
 * 
 * @return     The distance between the input and the set of closest matches
 */
template<typename T>
int closestMatchesT(T s, const std::set<T> &candidates, std::list<T> &matches);


constexpr auto closestMatches = &closestMatchesT<namestring>;


/** Takes an input name and generates an error message suggesting possible
 *  intented inputs based on a set of candidates. 
 *  If no close matches are found, the stream is not altered.
 *
 * @param[in]  nm          The input to match
 * @param[in]  candidates  Set of candidates to match to the input
 * @param      oss         Stream into which the error message will be placed
 * 
 * @return     
 */
void getClosestMatchError(namestring nm, std::set<namestring> candidates, std::ostream& oss);
