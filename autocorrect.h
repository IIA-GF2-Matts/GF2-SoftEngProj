#include <set>
#include "cistring.h"


const std::set<cistring> devicesset = {
  "CLOCK",
  "SWITCH",
  "AND",
  "NAND",
  "OR",
  "NOR",
  "XOR",
  "DTYPE"
};

const std::set<cistring> dtypeoutset = {
  "DATA", 
  "CLK", 
  "SET",
  "CLEAR"
};


// takes a cistring input and returns a set containing the
// closest matches from a set of candidates, along with the 
// calculated distance between them. Lower means a closer match.
int closestMatches(cistring s, const std::set<cistring> &candidates, std::set<cistring> &matches);

