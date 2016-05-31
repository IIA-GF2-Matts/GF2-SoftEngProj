

#ifndef GF2_LOCALESTRINGS_H
#define GF2_LOCALESTRINGS_H

#include <string>

/** Translate a string using the current locale.
 *
 * @param s    The string to translate
 * @return     The translated string, or the string passed to it
 *             if no translation was available.
 */
const char* t(const char* s);


/** Stores a string-translation table
 *
 * @author Diesel
 */
class LocaleStrings {
private:
    int _L;
    int _N;
    int _O;
    int _T;
    int _S;
    int _H;

    char* _data;
    int* _ref;

    bool dataLoad(const char* fname);
    bool readDataHeader();
    int intAt(int addr) const;
    unsigned int uintAt(int addr) const;
    int getIndex(const char* str, int l, int h) const;

public:
    /** Adds a .mo file used for translations with t(...)
     *
     * @param loc  The locale name, or a blank string to use the user's locale.
     * @return     True if the locale was set correctly, false otherwise.
     */
    static bool AddTranslations(const char* loc, const char* cat);

    /** Looks up the string in the table
     *
     * @param i    The index in the table
     * @return     A pointer to the string in the table
     */
    const char* get(int i) const;

    /** Looks up the translated string in the table
     *
     * @param i    The index in the table
     * @return     A pointer to the string in the table
     */
    const char* getTranslated(int i) const;

    /** Finds the index of the string in the table
     *
     * @param str  The string to search for.
     */
    int getIndex(const char* str) const;

    /** Gets the translation for a given string
     *
     * @param str  The string to search for.
     * @return     The translated version of the string, or
     *             str if no translation was found.
     */
    const char* translate(const char* str) const;

    /** Opens a .mo language definition file
     *
     * @param file The path to the file to open.
     * @return     True if the file was opened and the header
     *             parsed correctly.
     */
    bool open(const char* file);

    /** The LocaleStrings constructor
     *
     */
    LocaleStrings();

    /** The LocaleStrings copy constructor.
     *
     */
    LocaleStrings(const LocaleStrings& ls);

    /** Clears resources allocated by LocaleStrings
     *
     */
    ~LocaleStrings();
};


#endif