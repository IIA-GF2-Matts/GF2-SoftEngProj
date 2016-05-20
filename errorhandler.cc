
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
    std::cout << e.what() << std::endl;
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


    if (_file.empty()) {
        return "";
    }

    try {
        std::ifstream ifs;
        ifs.open(_file, std::ifstream::in | std::ifstream::binary);

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

        if (_srcLine.length() > cmax) {
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
mattmessage::mattmessage(std::string message, std::string file, SourcePos pos)
        : mattmessage(message, file, pos, MsgInfo) {
}


mattmessage::mattmessage(std::string message, std::string file, SourcePos pos, MessageType typ)
    : _message(message), _file(file), _pos(pos), _type(typ), _fname(file) {
    std::ostringstream oss;

    int cmax = 73;

    static const std::string mpTypStr[4] = {
        "Error", "Warning", "Note", "Info"
    };

    auto cls = oss.tellp();

    if (file != "") {
        auto rsl = file.rfind('/');

        if (rsl == std::string::npos)
            _fname = file;
        else {
            _fname = file.substr(rsl+1);
        }

        oss << _fname << " ";
    }

    _srcLineErrCol = -1;

    oss << "(" << pos.Line << ":" << pos.Column << "): ";
    int p = oss.tellp() - cls;
    oss << mpTypStr[typ] << ": ";
    int x = oss.tellp() - cls;

    if ((message.length() + x > cmax)
            && (x < cmax-20)) {
        // We can use our funky word wrap
        int y = 0, z;

        z = message.rfind(' ', cmax-x);
        if (z == std::string::npos || z <= cmax/2) {
            z = cmax - x;
        }

        oss << message.substr(y, z);
        y = z+1;

        while (y < message.length()) {
            z = message.rfind(' ', y+cmax-p);

            if (z == std::string::npos || z <= y+cmax/2) {
                z = y + cmax - p;
            }

            oss << "\n" << std::setfill(' ')
                << std::setw(std::min(cmax, int(p+message.length()-y)))
                << message.substr(y, z-y);

            y = z+1;
        }
    }
    else {
        oss << message;
    }

    getErrorLine(cmax);

    if (!_srcLine.empty()) {
        oss << "\n" << _srcLine;
    }

    if (_srcLineErrCol >= 0) {
        oss << "\n" << std::setfill(' ') << std::setw(_srcLineErrCol) << "^";
        if (typ == MsgError)
            oss << " ERROR";
    }

    _errorMessage = oss.str();
}


// class matterror

matterror::matterror(std::string message, std::string file, SourcePos pos)
        : mattmessage(message, file, pos, MsgError) {
}


// class mattwarning

mattwarning::mattwarning(std::string message, std::string file, SourcePos pos)
        : mattmessage(message, file, pos, MsgWarning) {
}


// class mattnote

mattnote::mattnote(std::string message, std::string file, SourcePos pos)
        : mattmessage(message, file, pos, MsgWarning) {
}

