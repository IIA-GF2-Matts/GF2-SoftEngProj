
#include <string>
#include <ostream>
#include <sstream>
#include <set>

#include "parser.h"
#include "names.h"
#include "scanner.h"
#include "network.h"
#include "devices.h"
#include "monitor.h"
#include "errorhandler.h"
#include "autocorrect.h"

#include "networkbuilder.h"



bool isLegalGateInputNamestring(namestring s, int maxn) {
    if (s.length() < 2
        || namestring::traits_type::ne(s[0], 'I')
        || !std::isdigit(s[1])
        || (s[1] == '0')) return false;

    int i = s[1]-'0';

    if (s.length() > 2) {
        if (s.length() > 3 || !std::isdigit(s[2])) return false;

        i = i*10 + (s[2] - '0');
    }

    return (i > 0) && (i <= maxn);
}


void networkbuilder::getUnknownPinError(Signal& sig, std::ostringstream& oss) {
    devicekind dkind = _netz->finddevice(sig.device.id)->kind;

    oss << *sig.device.id << " (of type "
        << *_devz->getname(dkind)
        << ") ";

    if (sig.pin.id == blankname) {
        oss << "has no default output pin.";
    } else {
        oss << "has no output pin " << *sig.pin.id << ".";
    }

    if (dkind == dtype)
        oss << " Use Q or Qbar.";
}

template<typename T>
void networkbuilder::getPredefinedError(devlink dvl, name key, T prevval, std::ostream& oss) {
    // todo: note should be a separate note message
    oss << "Attempt to redefine " //<< *_devz->getname(dvl->kind)
        << *dvl->id << "." << *key
        << ". Note: previously defined as "
        << prevval << " at position " << dvl->definedAt;
}





networkbuilder::networkbuilder(network* netz, devices* devz, monitor* mons, names* nms, errorcollector& errc)
    : _netz(netz), _devz(devz), _mons(mons), _nms(nms), _errs(errc) {
        //_devz->debug(true);
}

networkbuilder::~networkbuilder() {
}


void networkbuilder::defineDevice(Token& devName, Token& type) {
    // Semantic check: has devicename been defined before?
    if (_netz->finddevice(devName.id) != NULL) {
        throw mattsemanticerror("Device types may not be assigned to devices that already exist.", devName.at);
    }

    // make device (which adds it to the network)
    bool success;
    if (type.devtype == aswitch) {
        // -1 sets switch to floating
        _devz->makedevice(type.devtype, devName.id, -1, success, devName.at);
    }
    else {
        _devz->makedevice(type.devtype, devName.id, 0, success, devName.at);
    }

    if (!success) {
        // TODO: Better error message? Shouldn't ever reach here.
        throw mattruntimeerror("Unable to add device.", type.at);
    }
}


void networkbuilder::checkKey(devlink dvl, Token& keyTok) {

    // Todo: Merge key check switches


    // Key semantic checks
    switch(dvl->kind) {
        // Todo: quality check errors
        case aswitch:
            if (*keyTok.id != "InitialValue")
                throw mattsemanticerror("Switches may only have an `InitialValue` attribute.", keyTok.at);
            break;
        case aclock:
            if (*keyTok.id != "Period")
                throw mattsemanticerror("Clocks may only have a `Period` attribute.", keyTok.at);
            break;
        case andgate:
            if (!isLegalGateInputNamestring(*keyTok.id, 16))
                throw mattsemanticerror("AND gates may only have input pin attributes (up to 16), labelled I1 to I16", keyTok.at);
            break;
        case nandgate:
            if (!isLegalGateInputNamestring(*keyTok.id, 16))
                throw mattsemanticerror("NAND gates may only have input pin attributes (up to 16), labelled I1 to I16", keyTok.at);
            break;
        case orgate:
            if (!isLegalGateInputNamestring(*keyTok.id, 16))
                throw mattsemanticerror("OR gates may only have input pin attributes (up to 16), labelled I1 to I16", keyTok.at);
            break;
        case norgate:
            if (!isLegalGateInputNamestring(*keyTok.id, 16))
                throw mattsemanticerror("NOR gates may only have input pin attributes (up to 16), labelled I1 to I16", keyTok.at);
            break;
        case xorgate:
            if (!isLegalGateInputNamestring(*keyTok.id, 2))
                throw mattsemanticerror("XOR gates may only have input pin attributes (up to 2), labelled I1 to I2", keyTok.at);
            break;
        case dtype:
            // Todo: Name table comparison rather than string comparison
            if (!(*keyTok.id == "DATA" || *keyTok.id == "CLK" || *keyTok.id == "SET" || *keyTok.id == "CLEAR")) {
                std::ostringstream oss;
                oss << "DTYPE devices may only have DATA, CLK, SET or CLEAR input pins assigned. ";
                getClosestMatchError(*keyTok.id, dtypeoutset, oss);
                throw mattsemanticerror(oss.str(), keyTok.at);
            }
            break;
        case baddevice:
        default:
            // Should never reach here
            // Todo: better error message?
            throw mattruntimeerror("Could not assign key to a bad device type", keyTok.at);
    }

    // check if key has already been defined for particular device
    switch(dvl->kind) {
        // Todo: check errors
        case aswitch:
            if (dvl->swstate != floating) {
                std::ostringstream oss;
                getPredefinedError(dvl, keyTok.id, dvl->swstate == high ? 1 : 0, oss);
                throw mattsemanticerror(oss.str(), keyTok.at);
            }
            break;
        case aclock:
            if (dvl->frequency != 0) {
                std::ostringstream oss;
                getPredefinedError(dvl, keyTok.id, dvl->frequency, oss);
                throw mattsemanticerror(oss.str(), keyTok.at);
            }
            break;
        case andgate:
        case nandgate:
        case orgate:
        case norgate:
        case xorgate:
        case dtype:
        default: {
            inplink il = _netz->findinput(dvl, keyTok.id);
            if (il != NULL && (il->connect != NULL)) {
                std::ostringstream oss, prevval;
                outplink ol = il->connect;
                devlink dl = _netz->findoutputdevice(ol);
                if (dl == NULL)
                    // Should never reach here
                    throw mattruntimeerror("Device with requested output pin could not be found in the network. Connection not made.", keyTok.at);

                prevval << *dl->id;

                if (ol->id != blankname)
                    prevval << *ol->id;
                getPredefinedError(dvl, keyTok.id, prevval.str(), oss);
                throw mattsemanticerror(oss.str(), keyTok.at);
            }
            break;
        }
    }
}



void networkbuilder::setInputValue(Token& devName, Token& keyTok, Token& valTok) {

    devlink dvl = _netz->finddevice(devName.id);

    checkKey(dvl, keyTok);

    if (isLegalProperty(dvl, keyTok.id)) {
        assignProperty(dvl, keyTok, valTok);
    }
    else {
        if (valTok.number != 0 && valTok.number != 1)
            // Todo: improve error message
            throw mattsyntaxerror("Invalid signal.", valTok.at);
        Signal sig;
        sig.device = valTok;
        sig.device.type = TokType::Identifier;
        sig.device.id = _nms->lookup(valTok.number  ? "1" : "0");
        assignPin(dvl, keyTok, sig);
    }
}


void networkbuilder::setInputSignal(Token& devName, Token& keyTok, Signal& valSig) {

    devlink dvl = _netz->finddevice(devName.id);

    checkKey(dvl, keyTok);

    if (isLegalProperty(dvl, keyTok.id)) {
        // throw
    }
    else {
        assignPin(dvl, keyTok, valSig);
    }
}


void networkbuilder::assignPin(devlink dvl, Token keytk, Signal sig) {
    // Ensure signal exists
    signal_legality badSignal = isBadSignal(sig);
    if (badSignal) {
        if (badSignal == ILLEGAL_DEVICE) {
            throw mattsemanticerror("Devices must be defined before being referenced", sig.device.at);
        } else {
            // ILLEGAL_PIN
            std::ostringstream oss;
            oss << "Unable to set input pin. ";
            getUnknownPinError(sig, oss);
            throw mattsemanticerror(oss.str(), sig.device.at);
        }
    }

    // connect the gate
    bool success = false;
    if (!_netz->findinput(dvl, keytk.id))
        _netz->addinput(dvl, keytk.id, keytk.at);

    // todo: store token position?
    _netz->makeconnection(dvl->id, keytk.id, sig.device.id, sig.pin.id, success);
    if (!success)
        // Todo: improve error message
        throw mattruntimeerror("Could not make connection", keytk.at);
}

void networkbuilder::assignProperty(devlink dvl, Token keytk, Token valTok) {
    bool success = false;
    if (dvl->kind == aswitch) {
        // Switch
        if (valTok.type != TokType::Number || (valTok.number != 0 && valTok.number != 1))
            throw mattsemanticerror("Switches must have initial values of either 0 or 1", valTok.at);

        asignal sig = valTok.number ? high : low;
        _devz->setswitch(dvl->id, sig, success, keytk.at);

        if (!success)
            throw mattruntimeerror("Could not set switch initial value", valTok.at);

    } else if (dvl->kind == aclock) {
        // Clock
        if (valTok.type != TokType::Number || valTok.number < 1 || valTok.number > 32767)
            throw mattsemanticerror("Clock periods must be integers between 1 and 32767", valTok.at);

        // Todo: these might want to store a different token position
        _devz->setclock(dvl->id, valTok.number, success, keytk.at);

        if (!success)
            throw mattruntimeerror("Could not set clock period", valTok.at);
    }
}


void networkbuilder::defineMonitor(Signal& monSig) {
    Signal aliSig;
	defineMonitor(monSig, aliSig);
}

void networkbuilder::defineMonitor(Signal& monSig, Signal& aliSig) {


    // Ensure signal exists
    signal_legality badSignal = isBadSignal(monSig);
    if (badSignal) {
        if (badSignal == ILLEGAL_DEVICE) {
            throw mattsemanticerror("Devices must be defined before being monitored", monSig.device.at);
        } else {
            // ILLEGAL_PIN
            std::ostringstream oss;
            oss << "Unable to set monitor point. ";
            getUnknownPinError(monSig, oss);
            throw mattruntimeerror(oss.str(), monSig.device.at);
        }
    }

    // Check Alias
    if (aliSig.device.type != TokType::EndOfFile) {
        // Warn if signal exists
        if (!isBadSignal(aliSig)) {
            _errs.report(mattwarning("Alias signal name already exists.", aliSig.device.at));
        }
    }


    bool success = false;
    _mons->makemonitor(monSig.device.id, monSig.pin.id, success, aliSig.device.id, aliSig.pin.id);
    if (!success)
        // Todo: improve error message
        throw mattruntimeerror("Could not make monitor", monSig.device.at);
}


signal_legality networkbuilder::isBadSignal(Signal& sig) {
    devlink dvlnk = _netz->finddevice(sig.device.id);
    if (dvlnk == NULL)
        return ILLEGAL_DEVICE;
    if (_netz->findoutput(dvlnk, sig.pin.id) == NULL)
        return ILLEGAL_PIN;
    return LEGAL_SIGNAL;
}


bool networkbuilder::isLegalProperty(devlink dvl, name keyname) {
    return ( (dvl->kind == aclock && keyname == _nms->lookup("Period"))
            || (dvl->kind == aswitch && keyname == _nms->lookup("InitialValue")));
}
