#include <set>
#include <list>
#include <ostream>
#include "names.h"


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

const std::set<namestring> dtypeoutset = {
  "DATA",
  "CLK",
  "SET",
  "CLEAR"
};


// takes a cistring input and returns a set containing the
// closest matches from a set of candidates, along with the
// calculated distance between them. Lower means a closer match.

template<typename T>
int closestMatchesT(T s, const std::set<T> &candidates, std::list<T> &matches);


constexpr auto closestMatches = &closestMatchesT<namestring>;


void getClosestMatchError(namestring nm, std::set<namestring> candidates, std::ostream& oss);
