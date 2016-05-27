
#include <string>
#include <istream>

#include "sourcepos.h"


#ifndef GF2_IPOSSTREAM_H
#define GF2_IPOSSTREAM_H


/** Wraps an input character stream, whilst tracking source position.
 *
 * @author Diesel
 */
class iposstream {
public:
	/** Constructs a positional-stream
	 */
	iposstream();
	iposstream( std::istream* );
	iposstream( std::istream*, std::string fname );

	/** Clears resources allocated by the iposstream
	 */
	~iposstream();

	/** Returns the next character without extracting it
	 *
	 * @return     The next character in the stream
	 */
	int peek();

	/** Extracts the next character from the stream, and increments the counters
	 *
	 * @return     The next character in the stream
	 */
	int get();

	/** Checks the stream End of File flag
	 * @return True if the stream is at the end of the file.
	 */
	bool eof() const;

	/** Sets the base stream
	 *
	 * @param      is    The stream to base the positional stream on
	 * @param      fname The file name
	 */
	void setStream(std::istream* is);
	void setStream(std::istream* is, std::string fname );

	/** The current position of the stream
	 */
	SourcePos Pos;
	int TabWidth;

private:
	bool _open;
	std::istream* basestream;
	SourcePos next;
};


#endif // GF2_IPOSSTREAM_H
