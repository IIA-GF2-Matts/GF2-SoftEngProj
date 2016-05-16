
#include <istream>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

#include "names.h"
#include "iposstream.h"
#include "cistring.h"
#include "errorhandler.h"

#include "scanner.h"



// struct Token

Token::Token()
    : type(TokType::EndOfFile)
{
}

Token::Token(SourcePos pos, TokType t)
    : at(pos), type(t)
{
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

        if (c != '/') {
            throw matterror("Illegal character sequence '/' in file.", _file, p);
        }

        // Read until a newline.
        do {
            c = readChar();
        } while (!_ips.eof() && (c != '\n'));

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
            }
            else if (std::isalpha(c)) {
                ret.type = TokType::Identifier;
                ret.name = readName(c);

                // Note: namestring is cistring, so comparison here is
                // case-insensitive
                if (ret.name == "dev") {
                    ret.type = TokType::DevKeyword;
                }
                else if (ret.name == "monitor") {
                    ret.type = TokType::MonitorKeyword;
                }
                else if (ret.name == "as") {
                    ret.type = TokType::AsKeyword;
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

                throw matterror(oss.str(), _file, ret.at);
            }

            break;
    }

    return ret;
}


namestring scanner::readName(int c1) {
    std::ostringstream oss;
    oss << char(c1);

    while (std::isalnum(_ips.peek()) || _ips.peek() == '_') {
        oss << char(_ips.get());
    }

    return oss.str();
}


int scanner::readNumber(int c1) {
    int ret = c1 - '0';

    while(std::isdigit(_ips.peek())) {
        ret = ret*10 + (_ips.get() - '0');
    }

    return ret;
}


scanner::scanner()
        : _open(false) {
}


void scanner::open(std::istream* is, std::string fname) {
    _ips.setStream(is);
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

fscanner::fscanner() : scanner() {
}

void fscanner::open(std::string fname) {
    _ifs.open(fname, std::ifstream::in | std::ifstream::binary);

    scanner::open(&_ifs, fname);
}
