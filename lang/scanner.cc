
#include <istream>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../com/names.h"
#include "../com/iposstream.h"
#include "../com/errorhandler.h"
#include "../sim/network.h"

#include "scanner.h"


const std::map<namestring, devicekind> deviceTypes = {
    {"CLOCK", aclock},
    {"SWITCH", aswitch},
    {"AND", andgate},
    {"NAND", nandgate},
    {"OR", orgate},
    {"NOR", norgate},
    {"DTYPE", dtype},
    {"XOR", xorgate}
#ifdef EXPERIMENTAL
    ,{"SELECT", aselect}
    ,{"SIGGEN", siggen}
#endif
};


// struct Token

/** Constructs a new Token.
 *
 * @author Diesel
 */
Token::Token()
    : type(TokType::EndOfFile)
{
}

Token::Token(SourcePos pos, TokType t)
    : at(pos), type(t)
{
}

Token::Token(TokType t)
    : at(), type(t) {
}

Token::Token(TokType t, name s)
    : at(), type(t), id(s) {
}

Token::Token(TokType t, int num)
    : at(), type(t), number(num) {
    if (t == TokType::DeviceType) {
        devtype = devicekind(num);
    }
}



// class scanner

/** Read the next character from the input character stream
 *
 * @author Diesel
 */
int scanner::readChar() {
    return _ips.get();
}


/** Analyse the next set of characters to find the next token in the stream
 *
 * The Mattlang lexical elements can all be identified by the first character after
 * trimming leading whitespace.
 *
 * @author Diesel
 */
Token scanner::readNext() {
    Token ret;

    int c = readChar();

    // Remove leading whitespace
    while (!_ips.eof() && std::isspace(c)) {
        c = readChar();
    }

    if (_ips.eof()) {
        ret.at = _ips.Pos;
        _hasNext = true;
        return ret;
    }

    // Check for comment
    if (c == '/') {
        SourcePos p = _ips.Pos;
        c = readChar();

        if (c == '/') {
	        // Read until a newline.
	        do {
	            c = readChar();
	        } while (!_ips.eof() && (c != '\n'));
	    }
	    else if (c == '*') {
            bool hadStar = false;
            for (;;) {
                c = readChar();

                if (_ips.eof()) {
                    throw mattsyntaxerror("Unterminated block comment.", p);
                }

                if (hadStar && c == '/') {
                    break;
                }

                hadStar = (c == '*');
            }
	    }
        else {
        	throw mattsyntaxerror("Illegal character sequence '/' in file.", p);
        }

        if (_ips.eof()) {
            ret.at = _ips.Pos;
            _hasNext = true;
            return ret;
        }

        return readNext();
    }

    ret.at = _ips.Pos;

    // Conveniently, the scanner is also single lookahead.
    switch (c) {
        case '=':
            ret.type = TokType::Equals;
            break;
        case ':':
            ret.type = TokType::Colon;
            break;
        case ';':
            ret.type = TokType::SemiColon;
            break;
        case ',':
            ret.type = TokType::Comma;
            break;
        case '{':
            ret.type = TokType::Brace;
            break;
        case '}':
            ret.type = TokType::CloseBrace;
            break;
        case '.':
            ret.type = TokType::Dot;
            break;
#ifdef EXPERIMENTAL
        case '"':
            ret.type = TokType::String;
            ret.str = readString(c);
            break;
        case '$':
            ret.type = TokType::Bitstream;
            ret.bitstr = readBitstream(c);
            break;
#endif
        default:
            if (std::isdigit(c)) {
                ret.type = TokType::Number;
                ret.number = readNumber(c);

                if (ret.number < 0) {
                    throw mattsyntaxerror("Number is too large to be represented in Mattlab.", ret.at);
                }
            }
            else if (std::isalpha(c)) {
                ret.type = TokType::Identifier;
                ret.id = readName(c);

                // Note: namestring is cistring, so comparison here is
                // case-insensitive
                if (ret.id == kwordDev) {
                    ret.type = TokType::DevKeyword;
                }
                else if (ret.id == kwordMonitor) {
                    ret.type = TokType::MonitorKeyword;
                }
                else if (ret.id == kwordAs) {
                    ret.type = TokType::AsKeyword;
                }
#ifdef EXPERIMENTAL
                else if (ret.id == kwordImport) {
                    ret.type = TokType::ImportKeyword;
                }
#endif
                else { // Check for device types
                    auto it = deviceTypes.find(*ret.id);
                    if (it != deviceTypes.end()) {
                	   ret.type = TokType::DeviceType;
                       ret.devtype = it->second;
                    }
                }
            }
            else {
                // Lexer error: Illegal Character
                std::ostringstream oss;

                if (std::isprint(c)) {
                    oss << "Illegal Character '" << char(c) << "' in file.";
                }
                else if (c == '\0') {
                    oss << "Null byte in file. Note only ISO 8859 encoding is accepted";
                }
                else {
                    oss << "Illegal non-printing character, code "
                        << std::hex << int(c)
                        << " in file. ";
                }

                throw mattsyntaxerror(oss.str(), ret.at);
            }

            break;
    }

    _hasNext = true;
    return ret;
}


/** Consume characters while they match a name
 *
 * identifier     = letter , { alphanum } ;
 *
 * @author Diesel
 */
name scanner::readName(int c1) {
    std::basic_ostringstream<char, namestring::traits_type> oss;
    oss << char(c1);

    while (std::isalnum(_ips.peek()) || _ips.peek() == '_') {
        oss << char(_ips.get());
    }

    return _nmz->lookup(oss.str());
}


/** Consume characters while they match a number
 *
 * number         = digit , { digit } ;
 *
 * @author Diesel
 */
int scanner::readNumber(int c1) {
    int ret = c1 - '0';

    while(std::isdigit(_ips.peek())) {
        ret = ret*10 + (_ips.get() - '0');

        if (ret < 0) {
            // Overflow.
            // Consume remaining digits and return -1.

            while(std::isdigit(_ips.peek())) {
                _ips.get();
            }
            return -1;
        }
    }

    return ret;
}


#ifdef EXPERIMENTAL

/** Consumes characters matching a string literal
 *
 * stringliteral = string , stringliteral ;
 * string = '"' , { all characters - '"' } , '"' ;
 *
 * @author Diesel
 */
std::string scanner::readString(int c1) {
    std::ostringstream oss;

    SourcePos start = _ips.Pos;

    for (int c = _ips.get();;c = _ips.get()) {

        if (c == c1) {
            if (_ips.peek() == c1) {
                c = _ips.get();
            }
            else
                break;
        }

        if (_ips.eof()) {
            throw mattsyntaxerror("Unterminated string.", start);
        }

        oss << char(c);
    }

    return oss.str();
}


/** Consumes characters matching a bitstram literal
 *
 * bitstream = '$' , bit , { bit } ;
 * bit       = "0" | "1" ;
 *
 * @author Diesel
 */
std::vector<bool> scanner::readBitstream(int c1) {
    std::vector<bool> ret;

    SourcePos start = _ips.Pos;

    while(_ips.peek() == '0' || _ips.peek() == '0') {
        ret.push_back(_ips.get() == '1');
    }

    if (ret.size() == 0) {
        throw mattsyntaxerror("Was expecting a binary digit, empty bitstreams not allowed.", start);
    }

    return ret;
}

#endif


/** Initialises a new scanner
 *  Sets the name ids of keywords for use in comparisons
 *
 * @author Diesel
 */
scanner::scanner(names* nmz)
        : _open(false), _hasNext(false), _nmz(nmz), parent(NULL) {

    kwordDev = _nmz->lookup("dev");
    kwordMonitor = _nmz->lookup("monitor");
    kwordAs = _nmz->lookup("as");
    kwordImport = _nmz->lookup("import");
}


/** Clears resources allocated by the scanner
 *
 * @author Diesel
 */
scanner::~scanner() {
}


/** Open an input character stream with the given file name.
 *  Note: Streams should be opened as binary in order to prevent issues with
 *  changing EoL characters.
 *
 * @author Diesel
 */
bool scanner::open(std::istream* is, std::string fname) {
    if (is->good()) {
        _ips.setStream(is, fname);
        _file = fname;

        _open = true;
        _hasNext = false;
    }

    return _open;
}


/** Steps forwards in the stream to the next token.
 *
 * @author Diesel
 */
Token scanner::step() {
    Token ret = peek();
    _hasNext = false;

    return ret;
}


/** Peeks at the next token in the stream.
 *
 * @author Diesel
 */
Token scanner::peek() {
    if (!_hasNext) {
        _next = readNext();
    }

    return _next;
}


/** Gets the name of the file currently being scanned
 *
 * @author Diesel
 */
std::string scanner::getFile() const {
    return _file;
}


// class fscanner

/** Initialises the file scanner
 *
 * @author Diesel
 */
fscanner::fscanner(names* nmz) : scanner(nmz) {
}


/** Clears resources allocated by the file scanner.
 *
 * @author Diesel
 */
fscanner::~fscanner() {}

/** Opens a file, and then sets that as the base for the scanner.
 *  Note: Streams should be opened as binary in order to prevent issues with
 *  changing EoL characters.
 *
 * @author Diesel
 */
bool fscanner::open(std::string fname) {
    _ifs.open(fname, std::ifstream::in | std::ifstream::binary);

    return scanner::open(&_ifs, fname);
}


// class strscanner

/** Creates the string scanner, and opens the string.
 *
 * @author Diesel
 */
strscanner::strscanner(names* nmz, std::string str)
        : scanner(nmz), _iss(std::ifstream::in | std::ifstream::binary) {

    _iss.str(str);
    scanner::open(&_iss, "");
}
