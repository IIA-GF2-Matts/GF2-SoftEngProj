

#include <string>
#include <exception>
#include <list>
#include <ostream>

#include "sourcepos.h"

#ifndef GF2_ERRORHANDLER_H
#define GF2_ERRORHANDLER_H





/// Specific exception type for Matt language errors
class matterror : public std::exception {
protected:
    std::string _errorMessage;

    std::string _message;
    std::string _file;
    SourcePos _pos;
    std::string _srcLine;
    int _srcLineErrCol;

    std::string getErrorLine();
public:
    const SourcePos& pos() const;
    std::string getErrorMessage();

    /// Returns the complete error message for printing.
    virtual const char* what() const throw ();

    /// Create a new matterror, and builds the message.
    matterror(std::string message, std::string file, SourcePos pos);
    matterror(std::string message, std::string file, SourcePos pos, bool warning);
};


/// Specific exception type for Matt language errors
class mattwarning : public matterror {
public:
    /// Create a new matterror, and builds the message.
    mattwarning(std::string message, std::string file, SourcePos pos);
};


/// Utility class for collecting errors to list later.
class errorcollector {
public:
    std::list<matterror> errors;
    std::list<mattwarning> warnings;

    void report(matterror e);
    void report(mattwarning w);

    void print(std::ostream& os) const;

    errorcollector();
};


#endif
