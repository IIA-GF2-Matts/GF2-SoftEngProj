
#include <istream>

#include "sourcepos.h"
#include "iposstream.h"



// class iposstream

// Creates an uninitialised positional stream
iposstream::iposstream()
	: Pos( 0, 0, 0 ), TabWidth( 1 ), _open(false), next( 1, 1, 1 ) {
}


// Creates a positional stream based on an input stream.
iposstream::iposstream( std::istream* base )
	: Pos( 0, 0, 0 ), TabWidth( 1 ), _open(true), basestream( base ), next( 1, 1, 1 ) {
}

iposstream::~iposstream() {
}

// reads the next character without extracting it
int iposstream::peek() {
	return( basestream->peek() );
}

// Extracts the next character from the stream, and increments the counters.
// Regardless of operating system, this function will return '\n' for a line
// ending.
int iposstream::get() {
	this->Pos = next;

	if (basestream->eof()) {
		return '\0';
	}

	int ret = basestream->get();
	this->next.Abs += 1;

	// Handle \r\n sequence, required when using windows line endings on a non
	// windows operating system.
	if ( ret == '\r' ) {
		if ( this->peek() == '\n' ) {
			basestream->get();
			this->next.Abs += 1;
		}

		ret = '\n';
	}

	// Eol characters are reported at the end of the line, rather than at the
	// beginning of the next
	if ( ret == '\n' ) {
		this->next.Line += 1;
		this->next.Column = 1;
		this->next.LineStart = this->next.Abs + 1;
	}
	// else if ( ret == '\t' ) { // Tab is not always counted as 1 character.
	// 	this->next.Abs += this->TabWidth - 1;
	// 	this->next.Column += this->TabWidth;
	// }
	else {
		this->next.Column += 1;
	}

	return( ret );
}

// checks if end-of-file has been reached
bool iposstream::eof() const {
	return( basestream->eof() );
}


// Set the base stream
void iposstream::setStream(std::istream* is) {
	basestream = is;
	_open = true;
}
