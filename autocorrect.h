#include <set>
#include "cistring.h"

// takes a cistring input and returns a set containing the
// closest matches from a set of candidates, along with the 
// calculated distance between them. Lower means a closer match.
int closestMatches(cistring s, const std::set<cistring> &candidates, std::set<cistring> &matches);

