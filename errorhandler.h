

#include <string>
#include <exception>
#include <list>
#include <ostream>

#include "sourcepos.h"

#ifndef GF2_ERRORHANDLER_H
#define GF2_ERRORHANDLER_H


enum MessageType {
    MsgError,
    MsgWarning,
    MsgNote,
    MsgInfo
};

enum ErrorType {
    ErrGeneral,
    ErrSyntax,
    ErrSemantic,
    ErrRuntime
};

/// Formats messages for the user from the parser/scanner.
class mattmessage : public std::exception {
protected:
    std::string _errorMessage;

    std::string _message;
    std::string _fname;
    SourcePos _pos;
    MessageType _type;
    ErrorType _etype;
    std::string _srcLine;
    int _srcLineErrCol;

    std::string getErrorLine(int cmax);
public:
    const SourcePos& pos() const;
    std::string getErrorMessage();
    MessageType getType() const;
    ErrorType getErrType() const;
    bool is(MessageType t) const;

    /// Returns the complete error message for printing.
    virtual const char* what() const throw ();

    /// Create a new mattmessage, and builds the message.
    mattmessage(std::string message, SourcePos pos);
    mattmessage(std::string message, SourcePos pos, MessageType type);
    mattmessage(std::string message, SourcePos pos, MessageType type, ErrorType etype);
};


/// Specific exception type for Matt language errors
class matterror : public mattmessage {
public:
    /// Create a new matterror, and builds the message.
    matterror(std::string message, SourcePos pos);
    matterror(std::string message, SourcePos pos, ErrorType etype);
};


/// Specific exception type for Matt syntax errors
class mattsyntaxerror : public matterror {
public:
    /// Create a new mattsyntaxerror, and builds the message.
    mattsyntaxerror(std::string message, SourcePos pos);
};


/// Specific exception type for Matt semantic errors
class mattsemanticerror : public matterror {
public:
    /// Create a new mattsemanticerror, and builds the message.
    mattsemanticerror(std::string message, SourcePos pos);
};


/// Specific exception type for Matt runtime errors
class mattruntimeerror : public matterror {
public:
    /// Create a new mattruntimeerror, and builds the message.
    mattruntimeerror(std::string message, SourcePos pos);
};


/// Formats a warning message
class mattwarning : public mattmessage {
public:
    /// Create a new mattwarning, and builds the message.
    mattwarning(std::string message, SourcePos pos);
};


/// Formats a note
class mattnote : public mattmessage {
public:
    /// Create a new mattnote, and builds the message.
    mattnote(std::string message, SourcePos pos);
};




/// Utility class for collecting errors to list later.
class errorcollector {
private:
    int _count[4] = {0};
public:
    std::list<mattmessage> messages;

    int count(MessageType typ) const;
    int errCount() const;
    int warnCount() const;
    int infoCount() const;

    void report(mattmessage e);

    void print(std::ostream& os) const;

    errorcollector();
};


#endif
