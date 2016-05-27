
#include <cctype>
#include <string>
#include <ostream>

#include "cistring.h"


/** Equals operator for case insensitive characters
 *
 * @author Diesel
 */
bool ci_char_traits::eq( char c1, char c2 ) {
	return std::toupper( c1 ) == std::toupper( c2 );
}


/** Not-equals operator for case insensitive characters
 *
 * @author Diesel
 */
bool ci_char_traits::ne( char c1, char c2 ) {
	return std::toupper( c1 ) != std::toupper( c2 );
}


/** Less-than operator for case insensitive characters
 *
 * @author Diesel
 */
bool ci_char_traits::lt( char c1, char c2 ) {
	return std::toupper( c1 ) < std::toupper( c2 );
}


/** Compares two strings of case insensitive characters
 *
 * @author Diesel
 */
int ci_char_traits::compare( const char* s1, const char* s2, size_t n ) {
	while( n-- != 0 ) {
		if( ci_char_traits::lt( *s1, *s2 ) ) {
			return -1;
		}

		if( ci_char_traits::ne( *s1, *s2 ) ) {
			return 1;
		}

		++s1;
		++s2;
	}

	return 0;
}


/** Finds a character in a case insensitive string
 *
 * @author Diesel
 */
const char* ci_char_traits::find( const char* s, int n, char a ) {
	while( n-- > 0 && ci_char_traits::ne( *s, a ) ) {
		++s;
	}

	return s;
}

/** Prints a cistring to an output stream
 *
 * @author Diesel
 */
std::ostream& operator<<(std::ostream& os, const cistring& obj)
{
	os << obj.c_str();

    return os;
}
