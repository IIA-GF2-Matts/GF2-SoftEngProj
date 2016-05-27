#include <set>
#include <algorithm>
#include <list>
#include <ostream>
#include "names.h"

#include "autocorrect.h"


/** A naive implementation of Levenshtein Distance 
 *  See https://en.wikipedia.org/wiki/Levenshtein_distance
 *
 * Takes two inputs, and returns a "distance" between them
 * based on the number of changes that have to be made in order
 * to make the two inputs equivalent. A shorter distance corresponds
 * to a closer match.
 *
 * @author Judge, optimised by Diesel
 * 
 * @param[in]  s     Input 1
 * @param[in]  t     Input 2
 * @param[in] sE     Position of the end of input 1
 * @param[in] tE     Position of the end of input 2
 * 
 * @return           The distance between the inputs
 *
 */
template<typename T>
int LevenshteinDistance(const T& s, const T& t, int sE, int tE) {
    int dist;

    // base case: empty strings
    if (sE == 0) return tE;
    if (tE == 0) return sE;

    // test if last characters of the strings match

    dist = T::traits_type::ne(s[sE-1], t[tE-1]);

    // return minimum of
    // remove char from s
    int a = LevenshteinDistance<T>(s, t, sE-1, tE) + 1;
    // remove char from t
    int b = LevenshteinDistance<T>(s, t, sE, tE-1) + 1;
    // remove char from both
    int c = LevenshteinDistance<T>(s, t, sE-1, tE - 1) + dist;

    // Return minumum of a, b, c
    return std::min({a, b, c});
}

/** Takes an input and returns a set containing the
 *  closest matches from a set of candidates, along with the
 *  calculated distance between them. A lower distance means
 *  a closer match.
 *
 *  @author Judge
 */
template<typename T>
int closestMatchesT(T s, const std::set<T> &candidates, std::list<T> &matches) {
    // gets closest matches, returns distance
    int dist = 9999; // expecting 1 - 4
    int d;

    int sL = s.length();
    for (const auto i : candidates) {
        d = LevenshteinDistance<T>(s, i, sL, i.length());
        if (d < dist) {
            dist = d;
            matches.clear();
            matches.push_back(i);
        } else if (d == dist) {
            matches.push_back(i);
        }
    }
    return dist;
}

/** Takes an input name and generates an error message suggesting possible
 *  intented inputs based on a set of candidates. 
 *  If no close matches are found, the stream is not altered.
 *
 *  @author  Judge   
 */
void getClosestMatchError(namestring nm, std::set<namestring> candidates, std::ostream& oss) {
    std::list<namestring> matches;
    int dist = closestMatches(nm, candidates, matches);

    // if no close matches, don't do anything
    if (dist < 3 && matches.size() > 0) {
        auto i = matches.begin();

        oss << "Did you mean";
        for (; i != std::prev(matches.end()); ++i) {
            oss << " " << *i;
        }
        if (matches.size() > 1)
            oss << " or";
        oss << " " << *std::prev(matches.end()) << "?";
    }
}
