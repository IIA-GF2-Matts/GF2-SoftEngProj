
#include <string>
#include <ostream>
#include <sstream>
#include <set>

#include "../com/names.h"
#include "../com/errorhandler.h"
#include "../com/autocorrect.h"
#include "../sim/network.h"
#include "../sim/devices.h"
#include "../sim/monitor.h"
#include "../sim/importeddevice.h"

#include "scanner.h"
#include "parser.h"

#include "networkbuilder.h"


/** Checks if a name follows the correct format for a gate input (I##, 
 *  where ## is 1-maxn)
 *
 * @author Judge
 */
bool networkbuilder::isLegalGateInputNamestring(name n, int maxn) {
    namestring s = _nms->namestr(n);
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


/** Method to generate the error message when a signal pin has not been defined
 *
 * @author Judge
 */
void networkbuilder::getUnknownPinError(Signal& sig, std::ostringstream& oss) {
    devicekind dkind = _netz->finddevice(sig.device.id)->kind;

    oss << _nms->namestr(sig.device.id) << " (of type "
        << _nms->namestr(_devz->getname(dkind))
        << ") ";

    if (sig.pin.id == blankname) {
        oss << "has no default output pin.";
    } else {
        oss << "has no output pin " << _nms->namestr(sig.pin.id) << ".";
    }

    if (dkind == dtype)
        oss << " Use Q or Qbar.";
}

/** Method to generate the error message when a signal pin has not been defined
 *  Provides error and note messages (including details on the previous definition)
 *
 * @author Judge
 */
template<typename T>
void networkbuilder::getPredefinedError(devlink dvl, name key, T prevval, std::ostream& warnoss, std::ostream& noteoss) {
    warnoss << "Attempt to redefine " //<< *_devz->getname(dvl->kind)
        << _nms->namestr(dvl->id) << "." << _nms->namestr(key);
    noteoss << "Previously defined as " << prevval << " here";
}




/** Network builder constructor
 *
 * @author Judge
 */
networkbuilder::networkbuilder(network* netz, devices* devz, monitor* mons, names* nms, errorcollector& errc)
    : _netz(netz), _devz(devz), _mons(mons), _nms(nms), _errs(errc) {
        //_devz->debug(true);
}

networkbuilder::~networkbuilder() {
}

/** Defines and adds a new device to the network if determined to be a legal action
 *
 * @author Judge
 */
void networkbuilder::defineDevice(Token& devName, Token& type) {
    // Semantic check: has devicename been defined before?
    devlink dl = _netz->finddevice(devName.id);
    if (dl != NULL) {
        if (dl->kind == type.devtype) {
            _errs.report(mattwarning(
                "Repeated definition of the same device.", devName.at));
        }
        else {
            _errs.report(mattsemanticerror(
                "Device types may not be assigned to devices that already exist.", devName.at));
        }
        _errs.report(mattnote("Previously defined here.", dl->definedAt));
        return;
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
        // Shouldn't ever reach here
        _errs.report(mattruntimeerror(
            "Unable to add device to the network.", type.at));
        return;
    }
}


#ifdef EXPERIMENTAL

 /** Imports and adds a new device to the network from a secondary file if determined to be a legal action
 *
 * @author Diesel
 */
void networkbuilder::importDevice(Token& devName, Token& fileStr) {
    // Todo: Merge with defineDevice to avoid duplication
    devlink dl = _netz->finddevice(devName.id);
    if (dl != NULL) {
        if (dl->kind == imported) {
            _errs.report(mattwarning(
                "Repeated definition of the same device.", devName.at));
        }
        else {
            _errs.report(mattsemanticerror(
                "Device types may not be assigned to devices that already exist.", devName.at));
        }
        _errs.report(mattnote("Previously defined here.", dl->definedAt));
        return;
    }

    _devz->makeimported(devName.id, fileStr.str, _errs, devName.at);
}

#endif

/** Checks if a given key is legal for a device
 *   Checks if it's legal for the device type, then checks if it's been
 *     predefined.
 *
 * @author Judge
 */
bool networkbuilder::checkKey(devlink dvl, Token& keyTok) {
    // Todo: Merge key check switches

    // Key semantic checks
    switch(dvl->kind) {

        case aswitch:
            if (keyTok.id != _devz->initvalnm) {
                _errs.report(mattsemanticerror(
                    "Switches may only have an InitialValue attribute.", keyTok.at));
                return false;
            }
            break;
        case aclock:
            if (keyTok.id != _devz->periodnm) {
                _errs.report(mattsemanticerror(
                    "Clocks may only have a Period attribute.", keyTok.at));
                return false;
            }

            break;
        case andgate:
            if (!isLegalGateInputNamestring(keyTok.id, 16)) {
                _errs.report(mattsemanticerror(
                    "AND gates may only have input pin attributes (up to 16), labelled I1 to I16", keyTok.at));
                return false;
            }

            break;
        case nandgate:
            if (!isLegalGateInputNamestring(keyTok.id, 16)) {
                _errs.report(mattsemanticerror(
                    "NAND gates may only have input pin attributes (up to 16), labelled I1 to I16", keyTok.at));
                return false;
            }

            break;
        case orgate:
            if (!isLegalGateInputNamestring(keyTok.id, 16)) {
                _errs.report(mattsemanticerror(
                    "OR gates may only have input pin attributes (up to 16), labelled I1 to I16", keyTok.at));
                return false;
            }

            break;
        case norgate:
            if (!isLegalGateInputNamestring(keyTok.id, 16)) {
                _errs.report(mattsemanticerror(
                    "NOR gates may only have input pin attributes (up to 16), labelled I1 to I16", keyTok.at));
                return false;
            }

            break;
        case xorgate:
            if (!isLegalGateInputNamestring(keyTok.id, 2)) {
                _errs.report(mattsemanticerror(
                    "XOR gates may only have input pin attributes (up to 2), labelled I1 to I2", keyTok.at));
                return false;
            }

            break;
        case dtype:
            if (!(keyTok.id == _devz->datapin || keyTok.id == _devz->clkpin
                    || keyTok.id == _devz->setpin || keyTok.id == _devz->clrpin)) {
                std::ostringstream oss;
                oss << "DTYPE devices may only have DATA, CLK, SET or CLEAR input pins assigned. ";
                getClosestMatchError(_nms->namestr(keyTok.id), dtypeinset, oss);
                _errs.report(mattsemanticerror(oss.str(), keyTok.at));
                return false;
            }
            break;
#ifdef EXPERIMENTAL
        case imported:
            if (!dvl->device->hasInput(keyTok.id)) {
                std::ostringstream oss;
                oss << "Imported device " << _nms->namestr(dvl->id) << " has no input pin "
                    << _nms->namestr(keyTok.id);
                _errs.report(mattsemanticerror(oss.str(), keyTok.at));
                return false;
            }
            break;
        case aselect:
            if (!(keyTok.id == _devz->highpin || keyTok.id == _devz->lowpin || keyTok.id == _devz->swpin)) {
                std::ostringstream oss;
                oss << "SELECT device " << _nms->namestr(dvl->id) << " has no input pin "
                    << _nms->namestr(keyTok.id) << ". Correct pins are SW, HIGH and LOW";
                _errs.report(mattsemanticerror(oss.str(), keyTok.at));
                return false;
            }
            break;
#endif
        case baddevice:
        default:
            // Should never reach here
            _errs.report(mattruntimeerror(
                "Could not assign option to an unknown device type", keyTok.at));
            return false;
    }

    // check if key has already been defined for particular device
    switch(dvl->kind) {
        case aswitch:
            // initially set to floating
            if (dvl->swstate != floating) {
                std::ostringstream warnoss, noteoss;
                getPredefinedError(dvl, keyTok.id, dvl->swstate == high ? 1 : 0, warnoss, noteoss);
                _errs.report(mattsemanticerror(warnoss.str(), keyTok.at));
                _errs.report(mattnote(noteoss.str(), dvl->definedAt));
                return false;
            }
            break;
        case aclock:
            // initially set to 0
            if (dvl->frequency != 0) {
                std::ostringstream warnoss, noteoss;
                getPredefinedError(dvl, keyTok.id, dvl->frequency, warnoss, noteoss);
                _errs.report(mattsemanticerror(warnoss.str(), keyTok.at));
                _errs.report(mattnote(noteoss.str(), dvl->definedAt));
                return false;
            }
            break;
        case andgate:
        case nandgate:
        case orgate:
        case norgate:
        case xorgate:
        case dtype:
        case imported:
        case aselect:
        default: {
            // get the input link
            inplink il = _netz->findinput(dvl, keyTok.id);
            // xor predefines two inputs, so check a connection exists too
            if (il != NULL && (il->connect != NULL)) {
                outplink ol = il->connect;

                std::ostringstream warnoss, noteoss, prevval;

                devlink dl = _netz->findoutputdevice(ol);
                if (dl == NULL) {
                    // Should never reach here
                    _errs.report(mattruntimeerror(
                        "Device with requested output pin could not be found in the network. Connection not made.", keyTok.at));
                    return false;
                }

                prevval << *dl->id;

                if (ol->id != blankname)
                    prevval << "." << *ol->id;
                getPredefinedError(dvl, keyTok.id, prevval.str(), warnoss, noteoss);
                _errs.report(mattsemanticerror(warnoss.str(), keyTok.at));
                _errs.report(mattnote(noteoss.str(), il->definedAt));
                return false;
            }
            break;
        }
    }
    return true;
}


/** Sets an input property value of a device if determined to be a legal action
 *
 * @author Judge
 */
void networkbuilder::setInputValue(Token& devName, Token& keyTok, Token& valTok) {

    devlink dvl = _netz->finddevice(devName.id);

    if (!checkKey(dvl, keyTok)) {
        return;
    }

    if (isLegalProperty(dvl, keyTok.id)) {
        assignProperty(dvl, keyTok, valTok);
    }
    else {
        if (valTok.number != 0 && valTok.number != 1) {
            _errs.report(mattsemanticerror("Invalid signal name: input pins should be assigned a valid signal name or a logical state (0 or 1)", valTok.at));
            return;
        }
        Signal sig;
        sig.device = valTok;
        sig.device.type = TokType::Identifier;
        sig.device.id = _nms->lookup(valTok.number  ? "1" : "0");
        assignPin(dvl, keyTok, sig);
    }
}

/** Sets an input pin of a device to a signal if determined to be a legal action
 *
 * @author Judge
 */
void networkbuilder::setInputSignal(Token& devName, Token& keyTok, Signal& valSig) {

    devlink dvl = _netz->finddevice(devName.id);

    if (!checkKey(dvl, keyTok)) {
        return;
    }

    if (isLegalProperty(dvl, keyTok.id)) {
        // attempting to assign a signal to a property (not an input pin)
        if (keyTok.id == _devz->periodnm) {
            _errs.report(mattsemanticerror(
                "Clock periods must be numeric (assigning an identifier was attempted)", valSig.device.at));
        } else if (keyTok.id == _devz->initvalnm) {
            _errs.report(mattsemanticerror(
                "Initial values may only be 0 or 1, assigning a signal is not supported.", valSig.device.at));
        } else {
            _errs.report(mattsemanticerror(
                "Attempt to assign an identifier to a property", valSig.device.at));
        }
        return;
    }
    else {
        assignPin(dvl, keyTok, valSig);
    }
}

/** Creates a network link between a devices input pin and a signal output
 *  Checks the legality of such a connection prior to linking
 *
 * @author Judge
 */
void networkbuilder::assignPin(devlink dvl, Token keytk, Signal sig) {
    // Ensure signal exists
    signal_legality badSignal = isBadSignal(sig);
    if (badSignal) {
        if (badSignal == ILLEGAL_DEVICE) {
            _errs.report(mattsemanticerror("Devices must be defined before being referenced", sig.device.at));
        } else {
            // ILLEGAL_PIN
            std::ostringstream oss;
            oss << "Unable to set input pin. ";
            getUnknownPinError(sig, oss);
            _errs.report(mattsemanticerror(oss.str(), sig.device.at));
        }
        return;
    }

    // connect the gate
    bool success = false;
    inplink il = _netz->findinput(dvl, keytk.id);
    // check if the input link already exists (e.g. xor)
    if (!il)
        _netz->addinput(dvl, keytk.id, keytk.at);
    else
        il->definedAt = keytk.at;  // store this token position for xors

    _netz->makeconnection(dvl->id, keytk.id, sig.device.id, sig.pin.id, success);
    if (!success) {
        // should never reach here
        _errs.report(mattruntimeerror(
            "Could not make connection. One of the signals could not be found.", keytk.at));
        return;
    }
}

/** Assigns a value to a device's property
 *  Checks and reports invalid values
 *
 * @author Judge
 */
void networkbuilder::assignProperty(devlink dvl, Token keytk, Token valTok) {
    bool success = false;
    if (dvl->kind == aswitch) {
        // Switch
        if (valTok.type != TokType::Number || (valTok.number != 0 && valTok.number != 1)) {
            _errs.report(mattsemanticerror(
                "Switches must have initial values of either 0 or 1", valTok.at));
            return;
        }

        asignal sig = valTok.number ? high : low;
        _devz->setswitch(dvl->id, sig, success, keytk.at);

        if (!success) {
            _errs.report(mattruntimeerror("Could not set switch initial value", valTok.at));
            return;
        }

    } else if (dvl->kind == aclock) {
        // Clock
        if (valTok.type != TokType::Number || valTok.number < 1 || valTok.number > 32767) {
            _errs.report(mattsemanticerror(
                "Clock periods must be integers between 1 and 32767", valTok.at));
            return;
        }

        _devz->setclock(dvl->id, valTok.number, success, keytk.at);

        if (!success) {
            _errs.report(mattruntimeerror("Could not set clock period", valTok.at));
            return;
        }
    }
}

/** Creates a new monitor point (with no alias) if determined to be a
 *  legal action
 *
 * @author Judge
 */
void networkbuilder::defineMonitor(Signal& monSig) {
    Signal aliSig;
	defineMonitor(monSig, aliSig);
}

/** Creates a new monitor point if determined to be a
 *  legal action
 *
 * @author Judge
 */
void networkbuilder::defineMonitor(Signal& monSig, Signal& aliSig) {
    // Ensure signal exists
    signal_legality badSignal = isBadSignal(monSig);
    if (badSignal) {
        if (badSignal == ILLEGAL_DEVICE) {
            _errs.report(mattsemanticerror("Devices must be defined before being monitored", monSig.device.at));
            return;
        } else {
            // ILLEGAL_PIN
            std::ostringstream oss;
            oss << "Unable to set monitor point. ";
            getUnknownPinError(monSig, oss);
            _errs.report(mattruntimeerror(oss.str(), monSig.device.at));
            return;
        }
    }

    if (-1 != _mons->findmonitor(monSig.device.id, monSig.pin.id)) {
        // signal is already being monitored - warn
        _errs.report(mattwarning("Signal is already being monitored", monSig.device.at));
    }

    // Check Alias
    // check if token is defined (eof is default)
    if (aliSig.device.type != TokType::EndOfFile) {
        if (aliSig.device.id == monSig.device.id
            && aliSig.pin.id == monSig.pin.id) {
            // monitoring a signal as itself
            std::ostringstream oss;
            oss << "The 'as "
                << _nms->namestr(aliSig.device.id);
            if (aliSig.pin.id != blankname)
                oss << "." << _nms->namestr(aliSig.pin.id);
            oss << "' is not required in this case";
            _errs.report(mattnote(oss.str(), aliSig.device.at));

        // Warn if signal exists
        } else if (!isBadSignal(aliSig)) {
            _errs.report(mattwarning(
                "Alias signal name already exists as a device output in the network", aliSig.device.at));
        } else if (-1 != _mons->findmonitor(monSig.device.id, monSig.pin.id, true)) {
            // alias is already used, monitoring a diff signal
            _errs.report(mattwarning(
                "Alias name already exists, monitoring a different signal", aliSig.device.at));
        }
    }


    bool success = false;
    _mons->makemonitor(monSig.device.id, monSig.pin.id, success, aliSig.device.id, aliSig.pin.id);
    if (!success) {
        _errs.report(mattruntimeerror("Could not make monitor. All available monitors are used, or the device output could not be found.", monSig.device.at));
        return;
    }
}

/** Checks if a signal already exists (whether the device and pin of a signal are defined)
 *
 * @author Judge
 */
signal_legality networkbuilder::isBadSignal(Signal& sig) {
    devlink dvlnk = _netz->finddevice(sig.device.id);
    if (dvlnk == NULL)
        return ILLEGAL_DEVICE;
    if (_netz->findoutput(dvlnk, sig.pin.id) == NULL)
        return ILLEGAL_PIN;
    return LEGAL_SIGNAL;
}

/** Determine if a given property key is legal for a given device
 * does not check if the key has been previously assigned to
 *
 * @author Judge
 */
bool networkbuilder::isLegalProperty(devlink dvl, name keyname) {
    return ( (dvl->kind == aclock && keyname == _devz->periodnm)
            || (dvl->kind == aswitch && keyname == _devz->initvalnm));
}
