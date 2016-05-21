
#include <string>
#include <istream>

#include "sourcepos.h"


#ifndef UTIL_IPOSSTREAM_H
#define UTIL_IPOSSTREAM_H


// An input stream that also records character positions.
// Todo: Make this into a fully featured stream class derived from std::istream
class iposstream {
public:
	iposstream();
	iposstream( std::istream* );
	iposstream( std::istream*, std::string fname );
	~iposstream();

	int peek();
	int get();
	bool eof() const;

	void setStream(std::istream* );
	void setStream(std::istream*, std::string fname );

	SourcePos Pos;
	int TabWidth;

private:
	bool _open;
	std::istream* basestream;
	SourcePos next;
};


#endif
