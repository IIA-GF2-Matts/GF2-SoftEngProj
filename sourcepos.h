
#include <ostream>


#ifndef GF2_SOURCEPOS_H
#define GF2_SOURCEPOS_H

// Represents a position within a file.
struct SourcePos {
	// std::string& File;
	int Line;
	int Column;
	int Abs;
	int LineStart;

	SourcePos();
	SourcePos( int, int, int );
	SourcePos( int, int, int, int );

	friend std::ostream& operator<<( std::ostream&, const SourcePos& );
};


#endif
