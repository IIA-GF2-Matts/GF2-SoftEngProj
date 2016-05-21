

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
    scanner _scan;
    names* _nms;
    errorcollector errs;


    typedef enum {
        LEGAL_SIGNAL = 0,
        ILLEGAL_DEVICE,
        ILLEGAL_PIN
    } signal_legality;

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
    void parseOption(Token& tk, name dv);

    /// key = identifier ;
    void parseKey(Token& tk, devlink dvl, Token& keytk);

    /// value = signalname | number ;
    void parseValue(Token& tk, devlink dvl, Token& keytk);

    /// definemonitor = "monitor" , monitorset , ";" ;
    /// monitorset = monitor , { "," , monitor } ;
    void parseDefineMonitor(Token& tk);

    /// signalname , [ "as" , signalname ] ;
    void parseMonitor(Token& tk);

    /// signalname = devicename , [ "." , pin ] ;
    Signal parseSignalName(Token& tk);

    // checks whether device and pin are defined
    signal_legality isBadSignal(Signal& sig);

    // get the error message when a signal pin is unknown
    void getUnknownPinError(Signal& sig, std::ostringstream& oss) ;

    // get the error message when a key has been previously defined
    template<typename T>
    void getPredefinedError(devlink dvl, name key, T prevval, std::ostringstream& oss);

    // get the error message when a suggested input is required
    void getClosestMatchError(namestring nm, std::set<cistring> candidates, std::ostringstream& oss);

    // checks if keyname is a valid property key of a devlice
    bool isLegalProperty(devlink dl, name keyname);

    // links a device's input pin (specified by keytk) to a signal
    void assignPin(devlink dvl, Token keytk, Token valuetk, Signal sig);

    // sets a device's properties to that specified by valuetk
    // note, should check that the key is valid first
    void assignProperty(devlink dvl, Token keytk, Token valuetk);



public:
    /// Construct a parser to work on the pointers to other classes
    parser(network* netz, devices* devz, monitor* mons, scanner& scan, names* nms);
    ~parser();

    /** Reads the definition of the logic system and builds the            */
    /** corresponding internal representation via calls to the 'Network'   */
    /** module and the 'Devices' module.                                   */
    bool readin();

    const errorcollector& errors() const;
};


#endif
