


#ifndef GF2_PARSER_H
#define GF2_PARSER_H

#include <string>
#include <ostream>
#include <sstream>
#include <set>

#include "names.h"
#include "scanner.h"
#include "network.h"
#include "devices.h"
#include "monitor.h"
#include "errorhandler.h"


/// Signal struct, representing a DEVICE.PIN pair.
struct Signal {
    Token device;
    Token pin;
};

#include "networkbuilder.h"


/// Parser class. Constructs the device network from a token stream,
class parser
{
private:
    network* _netz;
    devices* _devz;
    monitor* _mons;
    scanner* _scan;
    names* _nms;
    errorcollector errs;
    networkbuilder netbuild;


    /// Steps over the next token, and peeks the one after
    void stepAndPeek(Token& tk);

    /// file = { statement } ;
    void parseFile(Token& tk);

    /// statement = definedevice | definemonitor ;
    void parseStatement(Token& tk);

#ifdef EXPERIMENTAL
    /// include = "include" , string , ";" ;
    void parseInclude(Token& tk);
#endif

    /// definedevice = "dev" , devicename , [ "=" , type ] , data ;
    void parseDefineDevice(Token& tk);

    /// data = optionset | ";" ;
    void parseData(Token& tk, Token& devName);

    /// optionset = "{" , { option } , "}" ;
    void parseOptionSet(Token& tk, Token& devName);

    /// option = key , ":" , value , ";" ;
    void parseOption(Token& tk, Token& devName);

    /// value = signalname | number ;
    void parseValue(Token& tk, Token& devName, Token& keytk);

    /// definemonitor = "monitor" , monitorset , ";" ;
    /// monitorset = monitor , { "," , monitor } ;
    void parseDefineMonitor(Token& tk);

    /// signalname , [ "as" , signalname ] ;
    void parseMonitor(Token& tk);

    /// signalname = devicename , [ "." , pin ] ;
    Signal parseSignalName(Token& tk);


public:
    /// Construct a parser to work on the pointers to other classes
    parser(network* netz, devices* devz, monitor* mons, scanner* scan, names* nms);
    ~parser();

    /** Reads the definition of the logic system and builds the            */
    /** corresponding internal representation via calls to the 'Network'   */
    /** module and the 'Devices' module.                                   */
    bool readin();

    const errorcollector& errors() const;
};


#endif
