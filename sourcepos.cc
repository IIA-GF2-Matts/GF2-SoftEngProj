

#include <ostream>
#include "sourcepos.h"


// struct SourcePos

SourcePos::SourcePos()
	: SourcePos( 0, 0, 0 ) {
}

SourcePos::SourcePos( int line, int col, int abs )
	: SourcePos( line, col, abs, abs - col ) {
}

SourcePos::SourcePos( int line, int col, int abs, int lineStart )
	: Line( line ),
	  Column( col ),
	  Abs( abs ),
	  LineStart( lineStart ) {
}

std::ostream& operator<<( std::ostream& os, const SourcePos& sp ) {
	return( os << sp.Line << ":" << sp.Column );
}