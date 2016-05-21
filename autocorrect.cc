#include <set>
#include "cistring.h"


int LevenshteinDistance(cistring s, cistring t) {
    // A naive implementation of Levenshtein Distance (Matt Judge)
    // See https://en.wikipedia.org/wiki/Levenshtein_distance
    int dist;
    int len_s = s.length();
    int len_t = t.length();

    // base case: empty strings
    if (len_s == 0) return len_t;
    if (len_t == 0) return len_s;

    // test if last characters of the strings match
    if (s[len_s-1] == t[len_t-1])
        dist = 0;
    else
        dist = 1;

    // return minimum of
    // remove char from s
    int a = LevenshteinDistance(s.substr(0, len_s - 1), t) + 1;
    // remove char from t
    int b = LevenshteinDistance(s, t.substr(0, len_t - 1)) + 1;
    // remove char from both
    int c = LevenshteinDistance(s.substr(0, len_s - 1), t.substr(0, len_t - 1)) + dist;

    if (a < b)
        if (c < a)
            return c;
        else
            return a;
    else
        if (c < b)
            return c;
        return b;
}

// Todo: ensure case insensitive
int closestMatches(cistring s, const std::set<cistring> &candidates, std::set<cistring> &matches) {
    // gets closest matches, returns distance
    int dist = 9999; // expecting 1 - 4
    int d;

    for (std::set<cistring>::iterator i = candidates.begin(); i != candidates.end(); ++i) {
        d = LevenshteinDistance(s, *i);
        if (d < dist) {
            dist = d;
            matches.clear();
            matches.insert(*i);
        } else if (d == dist) {
            matches.insert(*i);            
        }
    }
    return dist;
}
