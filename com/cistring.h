

#include <string>
#include <ostream>


#ifndef GF2_CISTRING_H
#define GF2_CISTRING_H

/** Case-insensitive character traits
 *
 * This struct allows STL containers to use case insensitive strings easily, by
 * defining methods on how characters compare to each other.
 *
 * For this struct, "case" is defined using the C locale.
 *
 * @author Diesel
 */
struct ci_char_traits : public std::char_traits<char> {

    /** Equals operator for case insensitive characters
     *
     * @param[in]  c1    The first character
     * @param[in]  c2    The second character
     *
     * @return     c1 == c2
     */
	static bool eq( char, char );

    /** Not-equals operator for case insensitive characters
     *
     * @param[in]  c1    The first character
     * @param[in]  c2    The second character
     *
     * @return     c1 != c2
     */
	static bool ne( char, char );

    /** Less-than operator for case insensitive characters
     *
     * @param[in]  c1    The first character
     * @param[in]  c2    The second character
     *
     * @return     c1 < c2
     */
	static bool lt( char, char );

    /** Compares two strings of case insensitive characters
     *
     * @param[in]  s1    The first string
     * @param[in]  s2    The second string
     * @param[in]  n     The number of characters to compare.
     *
     * @return     -1 if s1 < s2
     *              0 if s1 == s2
     *              1 if s1 > s2
     */
	static int compare( const char*, const char*, size_t );

    /** Finds a character in a case insensitive string
     *
     * @param[in]  s     The haystack
     * @param[in]  n     The max number of characters to search.
     * @param[in]  a     The needle
     *
     * @return     The pointer to the first occurence of a in s, or &s[n] if it was
     *             not found.
     */
	static const char* find( const char*, int, char );
};

typedef std::basic_string<char, ci_char_traits> cistring;


/** Prints a cistring to an output stream
 *
 * @param      os    The output stream
 * @param[in]  obj   The string to print.
 *
 * @return     The reference to the output stream, allowing stream operators to
 *             be chained.
 */
std::ostream& operator<<(std::ostream& os, const cistring& obj);


#endif // GF2_CISTRING_H
