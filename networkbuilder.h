

#ifndef GF2_NETWORKBUILDER_H
#define GF2_NETWORKBUILDER_H

#include <ostream>

#include "parser.h"
#include "names.h"
#include "scanner.h"
#include "network.h"
#include "devices.h"
#include "monitor.h"
#include "errorhandler.h"


typedef enum {
    LEGAL_SIGNAL = 0,
    ILLEGAL_DEVICE,
    ILLEGAL_PIN
} signal_legality;


class networkbuilder
{
private:
    network* _netz;
    devices* _devz;
    monitor* _mons;
    scanner* _scan;
    names* _nms;
    errorcollector errs;

public:
    networkbuilder(network* netz, devices* devz, monitor* mons, names* nms);
    ~networkbuilder();

	void defineDevice(Token& devName, Token& type);
    void setInputValue(Token& devName, Token& keyTok, Token& valTok);
    void setInputSignal(Token& devName, Token& keyTok, Signal& valSig);
    void defineMonitor(Signal& monSig);
    void defineMonitor(Signal& monSig, Signal& aliSig);

    // get the error message when a key has been previously defined
    template<typename T>
    void getPredefinedError(devlink dvl, name key, T prevval, std::ostream& oss);

    // get the error message when a signal pin is unknown
    void getUnknownPinError(Signal& sig, std::ostringstream& oss);

    // checks if keyname is a valid property key of a devlice
    bool isLegalProperty(devlink dl, name keyname);

    // links a device's input pin (specified by keytk) to a signal
    void assignPin(devlink dvl, Token keytk, Signal sig);

    // sets a device's properties to that specified by valuetk
    // note, should check that the key is valid first
    void assignProperty(devlink dvl, Token keytk, Token valuetk);

    // checks whether device and pin are defined
    signal_legality isBadSignal(Signal& sig);

    void checkKey(devlink dvl, Token& keyTok);
};


#endif
