
#include <string>
#include <exception>
#include <sstream>
#include <fstream>
#include <ostream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <iomanip>

#include "iposstream.h"

#include "errorhandler.h"



// class errorcollector

errorcollector::errorcollector() {
}

int errorcollector::count(MessageType typ) const {
    return _count[typ];
}

int errorcollector::errCount() const {
    return count(MsgError);
}

int errorcollector::warnCount() const {
    return count(MsgWarning);
}

int errorcollector::infoCount() const {
    return count(MsgInfo);
}

void errorcollector::report(mattmessage e) {
    messages.push_back(e);
    _count[e.getType()]++;
}

void errorcollector::print(std::ostream& os) const {
    for (auto it : messages) {
        os << it.what() << "\n\n";
    }
}



const SourcePos& mattmessage::pos() const {
    return _pos;
}


/// Reads the source file, and gets the line the error occured on.
std::string mattmessage::getErrorLine(int cmax) {

    cmax -= 4;

    if (!_srcLine.empty()) {
        return _srcLine;
    }


    if (_pos.fileStr().empty()) {
        return "";
    }

    try {
        std::ifstream ifs;
        ifs.open(_pos.fileStr(), std::ifstream::in | std::ifstream::binary);

        int p = _pos.Abs - _pos.Column;
        int c = _pos.Column;

        if (_pos.Column > cmax) {
            p = _pos.Abs - cmax;
            c = cmax;
        }

        ifs.clear();
        ifs.seekg(p);

        while (std::isspace(ifs.peek())) {
            ifs.get();
            c--;
        }

        std::getline(ifs, _srcLine);

        if ((int)_srcLine.length() > cmax) {
            int A = std::max(0, c - cmax/2);
            int B = std::min(A+cmax, int(_srcLine.length()));
            A = std::max(0, B-cmax);

            c = c-A;
            _srcLine = "... " + _srcLine.substr(A, B-A);
            c += 4;
        }
        _srcLineErrCol = c;
    }
    catch (...) {
        _srcLine = "(error getting source line)";
    }

    return _srcLine;
}


std::string mattmessage::getErrorMessage() {
    return _errorMessage;
}


MessageType mattmessage::getType() const {
    return _type;
}


bool mattmessage::is(MessageType t) const {
    return _type == t;
}



/// Returns the complete error message for printing.
const char* mattmessage::what() const throw () {
    return _errorMessage.c_str();
}


/// Create a new mattmessage, and builds the message.
mattmessage::mattmessage(std::string message, SourcePos pos)
        : mattmessage(message, pos, MsgInfo) {
}


/// Create a new mattmessage, and builds the message.
mattmessage::mattmessage(std::string message, SourcePos pos, MessageType typ)
        : mattmessage(message, pos, typ, ErrGeneral) {
}


std::string mattmessage::formatMessage(int cmax) {
    std::ostringstream oss;

    static const std::string mpTypStr[4] = {
        "Error", "Warning", "Note", "Info"
    };
    static const std::string mpETypStr[4] = {
        "Error", "SyntaxError", "SemanticError", "RuntimeError"
    };

    auto cls = oss.tellp();

    if (!_fname.empty())
        oss << _fname << " ";

    oss << "(" << _pos.Line << ":" << _pos.Column << "): ";
    int p = oss.tellp() - cls;

    if (_type == MsgError)
        oss << mpETypStr[_etype];
    else
        oss << mpTypStr[_type];

    oss << ": ";
    int x = oss.tellp() - cls;

    // Check if line would fit into max width
    if (((int)_message.length() + x > cmax)
            && (x < cmax-20)) {
        // We can use our funky word wrap
        int y = 0, z;

        // Get last space character that could fit on this line.
        z = _message.rfind(' ', cmax-x);
        if ((z == -1) || (z <= (cmax-x)/2)) {
            z = cmax - x;
        }

        oss << _message.substr(y, z);
        y = z+1;

        while (std::isspace(_message[y])) {
            y++;
        }

        while (y < (int)_message.length()) {
            z = _message.rfind(' ', y+cmax-p);

            if ((z == -1) || (z-y <= (cmax-p)/2)) {
                z = y + cmax - p;
            }
            else if (_message.length() - y <= cmax-p) {
                z = _message.length();
            }

            oss << "\n" << std::setfill(' ')
                << std::setw(std::min(cmax, int(p+z-y)))
                << _message.substr(y, z-y);

            y = z+1;

            while (std::isspace(_message[y])) {
                y++;
            }
        }
    }
    else {
        oss << _message;
    }

    getErrorLine(cmax);

    if (!_srcLine.empty()) {
        oss << "\n" << _srcLine;
    }

    if (_srcLineErrCol >= 0) {
        oss << "\n" << std::setfill(' ') << std::setw(_srcLineErrCol) << "^";
        if (_type == MsgError)
            oss << " ERROR";
    }

    return oss.str();
}

mattmessage::mattmessage(std::string message, SourcePos pos, MessageType typ, ErrorType etyp)
    : _message(message), _pos(pos), _type(typ), _etype(etyp) {

    const std::string& file = _pos.fileStr();
    if (file != "") {
        auto rsl = file.rfind('/');

        if (rsl == std::string::npos)
            _fname = file;
        else {
            _fname = file.substr(rsl+1);
        }
    }
    else {
        _fname = "";
    }
    _srcLineErrCol = -1;

    _errorMessage = formatMessage(73); // 73
}


// class matterror

matterror::matterror(std::string message, SourcePos pos)
        : mattmessage(message, pos, MsgError) {
}

matterror::matterror(std::string message, SourcePos pos, ErrorType etype)
        : mattmessage(message, pos, MsgError, etype) {
}


// class mattsyntaxerror

mattsyntaxerror::mattsyntaxerror(std::string message, SourcePos pos)
        : matterror(message, pos, ErrSyntax) {
}


// class mattsemanticerror

mattsemanticerror::mattsemanticerror(std::string message, SourcePos pos)
        : matterror(message, pos, ErrSemantic) {
}


// class mattruntimeerror

mattruntimeerror::mattruntimeerror(std::string message, SourcePos pos)
        : matterror(message, pos, ErrRuntime) {
}


// class mattwarning

mattwarning::mattwarning(std::string message, SourcePos pos)
        : mattmessage(message, pos, MsgWarning) {
}


// class mattnote

mattnote::mattnote(std::string message, SourcePos pos)
        : mattmessage(message, pos, MsgNote) {
}

