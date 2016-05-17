

#include <string>
#include <ostream>

#include "names.h"
#include "scanner.h"
#include "network.h"
#include "devices.h"
#include "monitor.h"


#ifndef GF2_PARSER_H
#define GF2_PARSER_H

/// Signal struct, representing a DEVICE.PIN pair.
struct Signal {
    name device;
    name pin;
};


/// Parser class. Constructs the device network from a token stream,
class parser
{
private:
    network* _netz;
    devices* _devz;
    monitor* _mons;
    names* _nms;
    scanner _scan;
    std::ostream& errs;

    /// Steps over the next token, and peeks the one after
    void stepAndPeek(Token& tk);

    /// file = { statement } ;
    void parseFile(Token& tk);

    /// statement = definedevice | definemonitor ;
    void parseStatement(Token& tk);

    /// definedevice = "dev" , devicename , [ "=" , type ] , data ;
    void parseDefineDevice(Token& tk);

    /// data = optionset | ";" ;
    void parseData(Token& tk, name dv);

    /// optionset = "{" , { option } , "}" ;
    void parseOptionSet(Token& tk, name dv);

    /// option = key , ":" , value , ";" ;
    /// value = signalname | number ;
    void parseOption(Token& tk, name dv);

    /// definemonitor = "monitor" , monitorset , ";" ;
    /// monitorset = monitor , { "," , monitor } ;
    void parseDefineMonitor(Token& tk);

    /// signalname , [ "as" , signalname ] ;
    void parseMonitor(Token& tk);

    /// signalname = devicename , [ "." , pin ] ;
    Signal parseSignalName(Token& tk);

public:
    /// Construct a parser to work on the pointers to other classes
    parser(network* netz, devices* devz, monitor* mons, scanner& scan, names* nms);
    ~parser();

    /** Reads the definition of the logic system and builds the            */
    /** corresponding internal representation via calls to the 'Network'   */
    /** module and the 'Devices' module.                                   */
    bool readin();
};


#endif
