
#include <ostream>
#include <string>
#include <set>

#ifndef GF2_SOURCEPOS_H
#define GF2_SOURCEPOS_H


/** A character position within a source file.
 *
 * @author Diesel
 */
struct SourcePos {
private:
    static std::set<std::string> fileNames;

    /** Returns an iterator the the file name within the set
     *  This allows string sharing between SourcePos instances.
     *
     * @param[in]  f    The string file name.
     *
     * @return     The iterator to the file name in the table.
     */
    std::set<std::string>::const_iterator getFile(std::string f);

public:
    /// The file name
    std::set<std::string>::const_iterator File;
    /// The line number
	int Line;
    /// The character column
	int Column;
    /// The absolute character position within the file.
	int Abs;

    /** Return the string file name
     *
     * @return     The string file name
     */
    const std::string& fileStr() const;

    /** Sets the file name.
     *
     * @param[in]  f     The string file name
     */
    void setFile(std::string f);

    /** Creates and populates a new SourcePos struct instance.
     */
	SourcePos();
    SourcePos(int line, int col);
    SourcePos(int line, int col, int abs);
	SourcePos( std::string file, int line, int col, int abs);

    /** Streams a source position to an output stream.
     *
     * @param      The output stream
     * @param      The source position
     *
     * @return     A reference to the output stream so that streaming operators
     *             can be chained.
     */
	friend std::ostream& operator<<( std::ostream& os, const SourcePos& pos);
};


#endif // GF2_SOURCEPOS_H
