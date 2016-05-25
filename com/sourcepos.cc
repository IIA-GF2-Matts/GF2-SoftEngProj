

#include <ostream>
#include <string>
#include <set>
#include "sourcepos.h"


// struct SourcePos

std::set<std::string> SourcePos::fileNames;

SourcePos::SourcePos()
    : SourcePos( "", 0, 0, 0 ) {
}

SourcePos::SourcePos(int line, int col)
    : SourcePos( "", line, col, 0 ) {
}

SourcePos::SourcePos(int line, int col, int abs)
    : SourcePos( "", line, col, abs ) {
}

SourcePos::SourcePos( std::string file, int line, int col, int abs )
	: File(getFile(file)),
      Line( line ),
	  Column( col ),
	  Abs( abs ) {
}


const std::string& SourcePos::fileStr() const {
    return *File;
}

void SourcePos::setFile(std::string f) {
    File = getFile(f);
}


std::set<std::string>::const_iterator SourcePos::getFile(std::string f) {
    return fileNames.insert(f).first;
}


std::ostream& operator<<( std::ostream& os, const SourcePos& sp ) {
    return( os << sp.Line << ":" << sp.Column );
}
