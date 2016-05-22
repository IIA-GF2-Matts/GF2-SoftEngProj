
#include <istream>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

#include "names.h"
#include "iposstream.h"
#include "errorhandler.h"
#include "network.h"

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
};


// struct Token

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

int scanner::readChar() {
    return _ips.get();
}


Token scanner::readNext() {
    Token ret;

    int c = readChar();

    while (!_ips.eof() && std::isspace(c)) {
        c = readChar();
    }

    if (_ips.eof()) {
        ret.at = _ips.Pos;
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

    return ret;
}


name scanner::readName(int c1) {
    std::basic_ostringstream<char, ci_char_traits> oss;
    oss << char(c1);

    while (std::isalnum(_ips.peek()) || _ips.peek() == '_') {
        oss << char(_ips.get());
    }

    return _nmz->lookup(oss.str());
}


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


scanner::scanner(names* nmz)
        : _open(false), _nmz(nmz) {

    kwordDev = _nmz->lookup("dev");
    kwordMonitor = _nmz->lookup("monitor");
    kwordAs = _nmz->lookup("as");
}


void scanner::open(std::istream* is, std::string fname) {
    _ips.setStream(is, fname);
    _file = fname;

    _open = true;
    _next = readNext();
}

scanner::~scanner() {
}


Token scanner::step() {
    Token ret = _next;

    _next = scanner::readNext();

    return ret;
}


Token scanner::peek() const {
    return _next;
}


std::string scanner::getFile() const {
    return _file;
}


// class fscanner

fscanner::fscanner(names* nmz) : scanner(nmz) {
}

void fscanner::open(std::string fname) {
    _ifs.open(fname, std::ifstream::in | std::ifstream::binary);

    scanner::open(&_ifs, fname);
}


// class strscanner

strscanner::strscanner(names* nmz, std::string str)
        : scanner(nmz), _iss(std::ifstream::in | std::ifstream::binary) {

    _iss.str(str);
    scanner::open(&_iss, "");
}
