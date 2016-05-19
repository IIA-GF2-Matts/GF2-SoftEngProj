
#include <string>
#include <exception>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <iostream>

#include "iposstream.h"

#include "errorhandler.h"



// class errorcollector

errorcollector::errorcollector() {
}

void errorcollector::report(matterror e) {
    std::cout << e.what() << std::endl;
    errors.push_back(e);
}

void errorcollector::report(mattwarning w) {
    std::cout << w.what() << std::endl;
    warnings.push_back(w);
}






/// Reads the source file, and gets the line the error occured on.
std::string matterror::getErrorLine() {

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

        if (_pos.Column > 150) {
            p = _pos.Abs - 150;
            c = 150;
        }

        ifs.clear();
        ifs.seekg(p);

        while (std::isspace(ifs.peek())) {
            ifs.get();
            c--;
        }

        std::getline(ifs, _srcLine);

        // if (_srcLine.length() > 150) {
        //     int A = std::max(0, c-75);
        //     int B = std::min(A+150, int(_srcLine.length()));
        //     A = std::max(0, B-150);

        //     c = c-A;
        //     _srcLine = _srcLine.substr(A, B-A);
        // }
        _srcLineErrCol = c;
    }
    catch (...) {
        _srcLine = "(error getting source line)";
    }

    return _srcLine;
}


std::string matterror::getErrorMessage() {
    return _errorMessage;
}


/// Returns the complete error message for printing.
const char* matterror::what() const throw () {
    return _errorMessage.c_str();
}


/// Create a new matterror, and builds the message.
matterror::matterror(std::string message, std::string file, SourcePos pos)
        : matterror(message, file, pos, false) {
}


matterror::matterror(std::string message, std::string file, SourcePos pos, bool warning)
    : _message(message), _file(file), _pos(pos) {
    std::ostringstream oss;

    if (file != "") {
        oss << file << " ";
    }

    _srcLineErrCol = -1;

    oss << "(" << pos.Line << ":" << pos.Column << "): "
        << (warning ? "Warning: " : "Error: ") << message;

    getErrorLine();
    if (!_srcLine.empty()) {
        oss << "\n" << _srcLine << "\n";
    }

    if (_srcLineErrCol >= 0) {
        oss << std::setfill('-') << std::setw(_srcLineErrCol+6) << "^ "
            << (warning ? "" : "ERROR") << "\n";
    }

    _errorMessage = oss.str();
}


// class mattwarning

mattwarning::mattwarning(std::string message, std::string file, SourcePos pos)
        : matterror(message, file, pos, true) {
}

