

#include <string>
#include <exception>

#include "iposstream.h"

#ifndef GF2_ERRORHANDLER_H
#define GF2_ERRORHANDLER_H


/// Specific exception type for Matt language errors
class matterror : public std::exception {
private:
    std::string _errorMessage;

    std::string _message;
    std::string _file;
    SourcePos _pos;
    std::string _srcLine;
    int _srcLineErrCol;

    std::string getErrorLine();
public:
    std::string getErrorMessage();

    /// Returns the complete error message for printing.
    virtual const char* what() const noexcept;

    /// Create a new matterror, and builds the message.
    matterror(std::string message, std::string file, SourcePos pos);
};


#endif
