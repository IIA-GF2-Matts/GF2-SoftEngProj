


#include <string>
#include <sstream>
#include <stdexcept>


/** Prints the Nth argument of a packed parameter list.
 *
 * @author Diesel
 */
template<typename T, typename... Args>
bool fmtprint(std::ostream& os, int N, T first, Args... args) {
	if (!N) {
		os << first;
		return true;
	}
	return fmtprint(os, N-1, args...);
}


/** Prints the Nth argument of a packed parameter list (specialization for length = 1)
 *
 * @author Diesel
 */
template<typename T>
bool fmtprint(std::ostream& os, int N, T v) {
	if (!N) {
		os << v;
		return true;
	}
	return false;
}



/** Formats a string according to a format pattern.
 * 
 * @author Diesel
 */
template<class ...TA>
std::string formatString(std::string f, TA... pargs) {
	std::ostringstream oss;

	std::string::size_type c = 0, p, n;
	for (;c < f.length();) {
		p = f.find('{', c);

		if (p == std::string::npos) {
			oss << f.substr(c);
			break;
		}

		oss << f.substr(c, p-c);

		n = 0;
		while (std::isdigit(f[++p])) {
			n = n*10 + (f[p]-'0');
		}

		if (f[p] != '}')
			throw std::invalid_argument("Format string bad.");
		p++;

		if (!fmtprint(oss, n, pargs...)) {
			throw std::out_of_range("Index was out of range of arguments");
		}

		c = p;
	}

	return oss.str();
}
