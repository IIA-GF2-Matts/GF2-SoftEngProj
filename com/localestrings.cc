

/* .mo File Format
 * Take from gettext documentation
 * https://www.gnu.org/software/gettext/manual/html_node/MO-Files.html


        byte
             +------------------------------------------+
          0  | magic number = 0x950412de                |
             |                                          |
          4  | file format revision = 0                 |
             |                                          |
          8  | number of strings                        |  == N
             |                                          |
         12  | offset of table with original strings    |  == O
             |                                          |
         16  | offset of table with translation strings |  == T
             |                                          |
         20  | size of hashing table                    |  == S
             |                                          |
         24  | offset of hashing table                  |  == H
             |                                          |
             .                                          .
             .    (possibly more entries later)         .
             .                                          .
             |                                          |
          O  | length & offset 0th string  ----------------.
      O + 8  | length & offset 1st string  ------------------.
              ...                                    ...   | |
O + ((N-1)*8)| length & offset (N-1)th string           |  | |
             |                                          |  | |
          T  | length & offset 0th translation  ---------------.
      T + 8  | length & offset 1st translation  -----------------.
              ...                                    ...   | | | |
T + ((N-1)*8)| length & offset (N-1)th translation      |  | | | |
             |                                          |  | | | |
          H  | start hash table                         |  | | | |
              ...                                    ...   | | | |
  H + S * 4  | end hash table                           |  | | | |
             |                                          |  | | | |
             | NUL terminated 0th string  <----------------' | | |
             |                                          |    | | |
             | NUL terminated 1st string  <------------------' | |
             |                                          |      | |
              ...                                    ...       | |
             |                                          |      | |
             | NUL terminated 0th translation  <---------------' |
             |                                          |        |
             | NUL terminated 1st translation  <-----------------'
             |                                          |
              ...                                    ...
             |                                          |
             +------------------------------------------+
*/

#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <cstdlib>
#include "localestrings.h"

/// The currently used localestring table.
std::list<LocaleStrings> g_langTables;


/** Adds a .mo file used for translations with t(...)
 *
 * @author Diesel
 */
bool LocaleStrings::AddTranslations(const char* locstr, const char* cat) {
    std::locale loc(locstr);
    std::string lname = loc.name();

    // For some reason we use LC_ALL not LANG
    const char* lnamecs = std::getenv("LC_ALL");
    if (lnamecs) lname = lnamecs;

    // Try open the language file.
    std::ostringstream fname;
    fname << "./intl/";

    int i = lname.find('_');
    if (i != -1) {
        fname << lname.substr(0, i);
    }
    else {
        // Todo: Is this the most sensible approach?
        fname << lname;
    }

    fname << "/" << cat << ".mo";

    LocaleStrings tbl;
    if (tbl.open(fname.str().c_str())) {
        g_langTables.push_back(tbl);
        return true;
    }
    return false;
}


/** Translate a string using the current locale.
 *
 * @author Diesel
 */
const char* t(const char* s) {
    int r;
    for (auto& l : g_langTables) {
        r = l.getIndex(s);
        if (r >= 0) return l.getTranslated(r);
    }

    return s;
}


/** Retrieves an integer from the data block.
 *
 * @author Diesel
 */
int LocaleStrings::intAt(int addr) const {
    if (addr > _L-4) return -1;
    return *reinterpret_cast<int*>(&_data[addr]);
}


/** Retrieves an unsigned integer from the data block.
 *
 * @author Diesel
 */
unsigned int LocaleStrings::uintAt(int addr) const {
    if (addr > _L-4) return -1;
    return *reinterpret_cast<unsigned int*>(&_data[addr]);
}


/** Loads a .mo file into memory
 *
 * @author Diesel
 */
bool LocaleStrings::dataLoad(const char* fname) {
    std::ifstream ifs;

    ifs.open(fname, std::ifstream::ate | std::ifstream::binary);
    if (!ifs) return false;
    _L = ifs.tellg();

    _data = new char[_L];
    _ref = new int(1);
    ifs.seekg(0);
    ifs.read(_data, _L);

    ifs.close();

    return true;
}


/** Reads in the information from the data header.
 *
 * @author Diesel
 */
bool LocaleStrings::readDataHeader() {
    if (uintAt(0) != 0x950412de || _L < 28) {
        return false; // Not a mo file.
    }

    if (intAt(4) > 0) {
        // The version is higher than what this library was
        // written for. May not work correctly.
    }

    _N = intAt(8);
    _O = intAt(12);
    _T = intAt(16);
    _S = intAt(20);
    _H = intAt(24);

    return true;
}


/** Looks up the string in the table
 *
 * @author Diesel
 */
const char* LocaleStrings::get(int i) const {
    if (!_N || i > _N) return nullptr;

    int addr = _O + 8*i + 4;
    int str = intAt(addr);

    return &_data[str];
}


/** Looks up the translated string in the table
 *
 * @author Diesel
 */
const char* LocaleStrings::getTranslated(int i) const {
    if (!_N || i > _N) return nullptr;
    
    int addr = _T + 8*i + 4;
    int str = intAt(addr);

    return &_data[str];
}


/** Finds the index of the string in the table
 *
 * @author Diesel
 */
int LocaleStrings::getIndex(const char* str) const {
    if (!_N) return -1;
    return getIndex(str, 0, _N);
}


/** Binary search for the string in the table.
 *  Note .MO files always have sorted strings.
 *
 * @author Diesel
 */
int LocaleStrings::getIndex(const char* str, int l, int h) const {
    if (h < l) return -1;

    int m  = (l+h) / 2;

    const char* ms = get(m);
    if (!ms) return -1;

    int c = std::string(str).compare(ms);

    if (c < 0)
        return getIndex(str, l, m-1);
    else if (c > 0)
        return getIndex(str, m+1, h);
    
    return m;
}


/** Gets the translation for a given string
 *
 * @author Diesel
 */
const char* LocaleStrings::translate(const char* str) const {
    int i = getIndex(str);
    if (i < 0) return str;

    return getTranslated(i);
}


/** Opens a .mo language definition file
 *
 * @author Diesel
 */
bool LocaleStrings::open(const char* file) {
    return dataLoad(file) && readDataHeader();
}


/** The LocaleStrings constructor
 *
 * @author Diesel
 */
LocaleStrings::LocaleStrings() : _ref(0) {
}


/** The LocaleStrings copy constructor
 *
 * @author Diesel
 */
LocaleStrings::LocaleStrings(const LocaleStrings& ls)
    : _L(ls._L), _N(ls._N), _O(ls._O),
        _T(ls._T), _S(ls._S), _H(ls._H),
        _data(ls._data), _ref(ls._ref)
{
    *ls._ref += 1;
}


/** Clears resources allocated by LocaleStrings
 *
 * @author Diesel
 */
LocaleStrings::~LocaleStrings() {
    if (_ref && !--*_ref) {
        delete[] _data;
        delete _ref;
    }
}