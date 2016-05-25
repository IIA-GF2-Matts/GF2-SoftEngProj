
#include <string>
#include <istream>
#include <fstream>
#include <sstream>
#include <map>

#include "iposstream.h" // SourcePos
#include "names.h" // namestring
#include "network.h"


#ifndef GF2_SCANNER_H
#define GF2_SCANNER_H


/// Token Type Enum
/// Used by the Token class to store the type of the token.
enum TokType {
    EndOfFile,
    DevKeyword,
    MonitorKeyword,
    AsKeyword,
    Equals,
    Colon,
    SemiColon,
    Comma,
    Brace,
    CloseBrace,
    Dot,
    Number,
    Identifier,
    DeviceType,
    ImportKeyword,
    String
};

extern const std::map<namestring, devicekind> deviceTypes;


/// Token class. Represents a lexical element in the source.
/// Stores type, data and position.
class Token
{
public:
    SourcePos at; ///< The location of the start of the token in the character stream
    TokType type; ///< The type of the token
    name id; ///< If type == TokType::Identifier, this is the string name
    int number; ///< If type == TokType::Number, this is the integer value.
    devicekind devtype; ///< If type == TokType::DeviceType, this is the devicekind.
#ifdef EXPERIMENTAL
    std::string str;
#endif

    Token();
    Token(SourcePos pos, TokType t);
    Token(TokType t);
    Token(TokType t, name s);
    Token(TokType t, int n);
};


/// Scanner class. Takes an inputted character stream and produces a stream of
/// tokens for the parser.
class scanner
{
protected:
    iposstream _ips;
    Token _next;
    std::string _file;
    bool _open;
    bool _hasNext;
    names* _nmz;

    name kwordDev;
    name kwordMonitor;
    name kwordAs;
    name kwordImport;

    /// Read the next character from the input character stream
    int readChar();

    /// Analyse the next set of characters to find the next token in the stream
    Token readNext();

    /// Consume characters while they match a name
    name readName(int c1);

    /// Consume characters while they match a number
    int readNumber(int c1);

#ifdef EXPERIMENTAL
    /// Consumes characters matching a string.
    std::string readString(int c1);
#endif

public:
    scanner* parent;

    scanner(names* nmz);
    virtual ~scanner();

    /// Open an input character stream with the given file name.
    /// Note: Streams should be opened as binary in order to prevent issues with
    /// changing EoL characters.
    bool open(std::istream* is, std::string fname);

    /// Steps to the next token
    Token step();

    /// Returns the next token in the stream
    Token peek();

    /// Returns the file name being scanned.
    std::string getFile() const;
};


/// File scanner class.
/// Simple extension to scanner class that opens a file and initialises the
/// scanner based on that stream.
class fscanner : public scanner
{
private:
    std::ifstream _ifs;
public:
    fscanner(names* nmz);
    ~fscanner();

    bool open(std::string fname);
};


class strscanner : public scanner
{
private:
    std::istringstream _iss;
public:
    strscanner(names* nmz, std::string str);
};

#endif
