#include <set>
#include <set>
#include <algorithm>
#include "cistring.h"

#include "autocorrect.h"


template<typename T>
int LevenshteinDistance(const T& s, const T& t, int sE, int tE) {
    // A naive implementation of Levenshtein Distance (Matt Judge)
    // See https://en.wikipedia.org/wiki/Levenshtein_distance
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
