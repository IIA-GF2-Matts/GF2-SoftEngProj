#include <iostream>
#include "parser.h"

using namespace std;

/* The parser for the circuit definition files */


bool parser::readin (void)
{
  /* over to you */
    return false;
}

parser::parser(network* netz, devices* devz, monitor* mons, scanner& scan, names* nms)
    : _netz(netz), _devz(devz), _mons(mons), _scan(scan), _nms(nms), errs(cout) {
}
