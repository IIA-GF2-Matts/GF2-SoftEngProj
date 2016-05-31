

#ifndef GF2_FORMATSTRING_H
#define GF2_FORMATSTRING_H

#include <string>

/** Produces a formatted string from arguments, a la printf
 *
 * The format string uses {N} to show argument N (zero based). For example:
 *
 * formatString("Hello, {0}!", "World")
 * => "Hello, World!"
 *
 * Arguments to the format string can be of any data type that has stream
 * operator overload.
 *
 * @param f      The format string.
 * @param pargs  The remaining arguments to use in the format string.
 * @return       The formatted string.
 */
template<class ...TA>
std::string formatString(std::string f, TA... pargs);


#endif