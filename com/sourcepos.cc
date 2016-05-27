

#include <ostream>
#include <string>
#include <set>
#include "sourcepos.h"


// struct SourcePos

std::set<std::string> SourcePos::fileNames;

/**
 *
 * @author Diesel
 */
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


/** Return the string file name
 *
 * @author Diesel
 */
const std::string& SourcePos::fileStr() const {
    return *File;
}


/** Sets the file name.
 *
 * @author Diesel
 */
void SourcePos::setFile(std::string f) {
    File = getFile(f);
}


/** Returns an iterator the the file name within the set
 *
 * @author Diesel
 */
std::set<std::string>::const_iterator SourcePos::getFile(std::string f) {
    return fileNames.insert(f).first;
}


/** Streams a source position to an output stream.
 *
 * @author Diesel
 */
std::ostream& operator<<( std::ostream& os, const SourcePos& sp ) {
    return( os << sp.Line << ":" << sp.Column );
}
