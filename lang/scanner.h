
#include <string>
#include <istream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

#include "../com/iposstream.h" // SourcePos
#include "../com/names.h" // names, namestring, name
#include "../sim/network.h" // devicekind


#ifndef GF2_SCANNER_H
#define GF2_SCANNER_H


/** Token Type Enum
 * Used by the Token class to store the type of the token.
 */
enum TokType {
    EndOfFile,
    DevKeyword,
    MonitorKeyword,
    AsKeyword,
    Equals,
    Colon,
    SemiColon,
    Comma,
    Brace,
    CloseBrace,
    Dot,
    Number,
    Identifier,
    DeviceType,
    ImportKeyword,
    String,
    Bitstream
};

extern const std::map<namestring, devicekind> deviceTypes;


/** Token class. Represents a lexical element in the source.
 *  Stores type, data and position.
 *
 *  @author Diesel
 */
class Token
{
public:
    SourcePos at; ///< The location of the start of the token in the character stream
    TokType type; ///< The type of the token
    name id; ///< If type == TokType::Identifier, this is the string name
    int number; ///< If type == TokType::Number, this is the integer value.
    devicekind devtype; ///< If type == TokType::DeviceType, this is the devicekind.
    std::string str; ///< If type == TokType::String, this is the string
    std::vector<bool> bitstr; ///< If type == TokType::Bitstream, this is the bit stream.

    /** Constructs a new Token.
     */
    Token();
    Token(SourcePos pos, TokType t);
    Token(TokType t);
    Token(TokType t, name s);
    Token(TokType t, int n);
};


/** Scanner class. Takes an inputted character stream and produces a stream of
 * tokens for the parser.
 *
 * @author Diesel
 */
class scanner
{
protected:
    iposstream _ips;
    Token _next;
    std::string _file;
    bool _open;
    bool _hasNext;
    names* _nmz;

    name kwordDev;
    name kwordMonitor;
    name kwordAs;
    name kwordImport;

    /** Read the next character from the input character stream
     *
     * @return The next character from the input stream.
     */
    int readChar();

    /** Analyse the next set of characters to find the next token in the stream
     *
     * @return     The next token from the input stream
     */
    Token readNext();

    /** Consume characters while they match a name
     *
     * @param[in]  c1    The previous character pulled from the stream
     * @return     The name id of the name read.
     */
    name readName(int c1);

    /** Consume characters while they match a number
     *
     * @param[in]  c1    The previous character pulled from the stream
     * @return     The integer read from the stream.
     */
    int readNumber(int c1);

    /** Consumes characters matching a string literal
     *
     * @param[in]  c1    The previous character pulled from the stream
     * @return     The string literal read from the stream.
     */
    std::string readString(int c1);

    /** Consumes characters matching a bitstream literal
     *
     * @param[in]  c1    The previous character pulled from the stream
     * @return     The bitstream literal read from the stream
     */
    std::vector<bool> readBitstream(int c1);

public:
    scanner* parent;

    /** Initialises a new scanner
     *
     * @param      nmz   The instance of the name table to use
     */
    scanner(names* nmz);

    /** Clears resources allocated by the scanner
     *  Virtual to ensure derivatives can release their streams.
     */
    virtual ~scanner();

    /** Open an input character stream with the given file name.
     *  Note: Streams should be opened as binary in order to prevent issues with
     *  changing EoL characters.
     *
     * @param      is     The input stream to wrap
     * @param[in]  fname  The file name for error reports.
     * @return     True if the stream was set as the base of the scanner.
     */
    bool open(std::istream* is, std::string fname);

    /** Steps forwards in the stream to the next token.
     *
     * @return     The next token in the input stream.
     */
    Token step();

    /** Peeks at the next token in the stream.
     *
     * @return     The next token in the input stream.
     */
    Token peek();

    /** Gets the name of the file currently being scanned
     *
     * @return     The file name.
     */
    std::string getFile() const;
};


/** File scanner class.
 *  Simple extension to scanner class that opens a file and initialises the
 *  scanner based on that stream.
 *
 *  @author Diesel
 */
class fscanner : public scanner
{
private:
    std::ifstream _ifs;
public:
    /** Initialises the file scanner
     *
     * @param      nmz   The instance of the name table to use.
     */
    fscanner(names* nmz);

    /** Clears resources allocated by the file scanner.
     */
    ~fscanner();

    /** Opens a file, and then sets that as the base for the scanner.
     *
     * @param[in]  fname  The file path to open
     * @return     True if the stream was set correctly.
     */
    bool open(std::string fname);
};


/** String scanner class.
 *  Simple extension to scanner class that opens a string stream and initialises the
 *  scanner based on that stream.
 *
 *  @author Diesel
 */
class strscanner : public scanner
{
private:
    std::istringstream _iss;
public:
    /** Creates the string scanner, and opens the string.
     *
     * @param      nmz   The instance of the name table to use
     * @param[in]  str   The input string.
     */
    strscanner(names* nmz, std::string str);
};


#endif // GF2_SCANNER_H
