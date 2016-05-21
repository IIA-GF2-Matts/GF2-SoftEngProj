
#include <iostream>
#include <sstream>

#include "errorhandler.h"
#include "scanner.h"
#include "names.h"

#include "parser.h"



bool isLegalGateInputNamestring(namestring s, int maxn) {
    if (s.length() < 2
        || std::toupper(s[0]) != 'I'
        || !std::isdigit(s[1])
        || (s[1] == '0')) return false;

    int i = s[1]-'0';

    if (s.length() > 2) {
        if (s.length() > 3 || !std::isdigit(s[2])) return false;

        i = i*10 + (s[2] - '0');
    }

    return (i > 0) && (i <= maxn);
}

parser::parser(network* netz, devices* devz, monitor* mons, scanner& scan, names* nms)
    : _netz(netz), _devz(devz), _mons(mons), _scan(scan), _nms(nms) {
        //_devz->debug(true);
}

parser::~parser() {
}


bool parser::readin() {
    Token tk = _scan.peek();
    parseFile(tk);

    _netz->checknetwork(errs);

    return errs.errCount() == 0;
}

void parser::stepAndPeek(Token& tk) {
    _scan.step();
    tk = _scan.peek();
}


// EBNF Rows:

// file = { statement } ;
void parser::parseFile(Token& tk) {
    while (tk.type != TokType::EndOfFile) {
        try {
            parseStatement(tk);
        }
        catch (matterror& e) {
            errs.report(e);

            // Consume tokens until the next statement is reached
            while (tk.type != TokType::DevKeyword && tk.type != TokType::MonitorKeyword) {
                if (tk.type == TokType::EndOfFile) {
                    stepAndPeek(tk);
                    break;
                }
                stepAndPeek(tk);
            }
        }
    }

    // TODO: Check for errors after parsing.

    std::cout << "File parsed with "
            << errs.errCount() << " errors and "
            << errs.warnCount() << " warnings." << std::endl;
}


// statement = definedevice | definemonitor ;
void parser::parseStatement(Token& tk) {
    switch (tk.type) {
        case TokType::DevKeyword:
            stepAndPeek(tk);
            parseDefineDevice(tk);

            break;
        case TokType::MonitorKeyword:
            stepAndPeek(tk);
            parseDefineMonitor(tk);

            break;
        default:
            // Todo: Better message.
            throw matterror("Unexpected token type. Expected a device or monitor defintion.", _scan.getFile(), tk.at);
    }
}


// definedevice = "dev" , devicename , [ "=" , type ] , data ;
void parser::parseDefineDevice(Token& tk) {
    Token nameToken;

    if (tk.type != TokType::Identifier) {
        throw matterror("Expected a device name.", _scan.getFile(), tk.at);
    }
    // dev, as, monitor are handled by the scanner and made *Keywords
    // device types are handled by the scanner and made DeviceTypes

    nameToken = tk;
    name dv = _nms->lookup(nameToken.name);

    stepAndPeek(tk);

    if (tk.type == TokType::Equals) {
        // Semantic check: has devicename been defined before?
        if (_netz->finddevice(dv) != NULL) {
            throw matterror("Device types may not be assigned to devices that already exist.", _scan.getFile(), tk.at);
        }

        stepAndPeek(tk);

        if (tk.type != TokType::DeviceType) {
            // Todo: Suggest types
            throw matterror("Expected device type.", _scan.getFile(), tk.at);
        }
        // make device (which adds it to the network)
        bool success;
        if (tk.devtype == aswitch)
            // -1 sets switch to floating
            _devz->makedevice(tk.devtype, dv, -1, success, nameToken.at);
        else
            _devz->makedevice(tk.devtype, dv, 0, success, nameToken.at);

        // Debug
        /*
        std::cout << "made device " << _nms-namestr(dv);
        std::cout << " regtype ";
        _devz->writedevice(_netz->finddevice(dv)->kind);
        //_devz->writedevice(_devz->devkind(dv));
        cout << std::endl;
        */
        if (!success) {
            throw matterror("Unable to add device.", _scan.getFile(), tk.at);
            // TODO: Better error message? Shouldn't ever reach here.
        }

        stepAndPeek(tk);
    }

    parseData(tk, dv);
}


// data = optionset | ";" ;
void parser::parseData(Token& tk, name dv) {
    if (tk.type == TokType::SemiColon) {
        stepAndPeek(tk);
    }
    else if (tk.type == TokType::Brace) {
        stepAndPeek(tk);

        parseOptionSet(tk, dv);
    }
    else {
        // Todo: Better error message.
        throw matterror("Unexpected token. Expecting ; or {.", _scan.getFile(), tk.at);
    }
}


// optionset = "{" , { option } , "}" ;
void parser::parseOptionSet(Token& tk, name dv) {
    while (tk.type != TokType::CloseBrace) {
        if (tk.type == TokType::EndOfFile) {
            throw matterror("Unterminated braces.", _scan.getFile(), tk.at);
        }

        try {
            parseOption(tk, dv);
        }
        catch (matterror& e) {
            errs.report(e);

            // Consume tolens until you get to the end of option/block.
            while (tk.type != TokType::SemiColon) {
                if (tk.type == TokType::CloseBrace) {
                    stepAndPeek(tk);
                    return;
                }
                stepAndPeek(tk);
            }
            stepAndPeek(tk);
        }
    }
    stepAndPeek(tk);
}

void parser::getUnknownPinError(Signal& sig, std::ostringstream& oss) {
    devicekind dkind = _netz->finddevice(sig.device)->kind;

    oss << *sig.device << " (of type "
        << *_devz->getname(dkind)
        << ") ";

    if (sig.pin == blankname) {
        oss << "has no default output pin.";
    } else {
        oss << "has no output pin " << *sig.pin << ".";
    }

    if (dkind == dtype)
        oss << " Use Q or Qbar.";
}

template<typename T>
void parser::getPredefinedError(devlink dvl, name key, T prevval, std::ostringstream& oss) {
    // todo: note should be a separate note message
    oss << "Attempt to redefine " //<< *_devz->getname(dvl->kind)
        << *dvl->id << "." << *key
        << ". Note: previously defined as "
        << prevval << " at position " << dvl->definedAt;
}


// option = key , ":" , value , ";" ;
void parser::parseOption(Token& tk, name dv) {
    devlink dvl = _netz->finddevice(dv);
    Token keytk, valuetk;
    parseKey(tk, dvl, keytk);

    if (tk.type != TokType::Colon) {
        throw matterror("Expected colon.", _scan.getFile(), tk.at);
    }

    stepAndPeek(tk);
    parseValue(tk, dvl, keytk);

    if (tk.type != TokType::SemiColon) {
        throw matterror("Missing a semicolon on the end.", _scan.getFile(), tk.at);
    }
    stepAndPeek(tk);
}

// key = identifier ;
void parser::parseKey(Token& tk, devlink dvl, Token& keytk) {

    if (tk.type != TokType::Identifier) {
        throw matterror("Expected a key.", _scan.getFile(), tk.at);
    }

    keytk = tk;
    name keyname = _nms->lookup(keytk.name);

    switch(dvl->kind) {
        // Todo: quality check errors
        case aswitch:
            if (keytk.name != "InitialValue")
                throw matterror("Switches may only have an `InitialValue` attribute.", _scan.getFile(), keytk.at);
            break;
        case aclock:
            if (keytk.name != "Period")
                throw matterror("Clocks may only have a `Period` attribute.", _scan.getFile(), keytk.at);
            break;
        case andgate:
            if (!isLegalGateInputNamestring(keytk.name, 16))
                throw matterror("AND gates may only have input pin attributes (up to 16), labelled I1 to I16", _scan.getFile(), keytk.at);
            break;
        case nandgate:
            if (!isLegalGateInputNamestring(keytk.name, 16))
                throw matterror("NAND gates may only have input pin attributes (up to 16), labelled I1 to I16", _scan.getFile(), keytk.at);
            break;
        case orgate:
            if (!isLegalGateInputNamestring(keytk.name, 16))
                throw matterror("OR gates may only have input pin attributes (up to 16), labelled I1 to I16", _scan.getFile(), keytk.at);
            break;
        case norgate:
            if (!isLegalGateInputNamestring(keytk.name, 16))
                throw matterror("NOR gates may only have input pin attributes (up to 16), labelled I1 to I16", _scan.getFile(), keytk.at);
            break;
        case xorgate:
            if (!isLegalGateInputNamestring(keytk.name, 2))
                throw matterror("XOR gates may only have input pin attributes (up to 2), labelled I1 to I2", _scan.getFile(), keytk.at);
            break;
        case dtype:
            if (!(keytk.name == "DATA" || keytk.name == "CLK" || keytk.name == "SET" || keytk.name == "CLEAR"))
                throw matterror("DTYPE devices may only have DATA, CLK, SET or CLEAR input pins assigned", _scan.getFile(), keytk.at);
                // Todo: closest word suggestion
            break;
        case baddevice:
        default:
            // Should never reach here
            // Todo: better error message?
            throw matterror("Could not assign key to a bad device type", _scan.getFile(), keytk.at);
    }

    // check if key has already been defined for particular device
    switch(dvl->kind) {
        // Todo: check errors
        case aswitch:
            if (dvl->swstate != floating) {
                std::ostringstream oss;
                getPredefinedError(dvl, keyname, dvl->swstate == high ? 1 : 0, oss);
                throw matterror(oss.str(), _scan.getFile(), keytk.at);
            }
            break;
        case aclock:
            if (dvl->frequency != 0) {
                std::ostringstream oss;
                getPredefinedError(dvl, keyname, dvl->frequency, oss);
                throw matterror(oss.str(), _scan.getFile(), keytk.at);
            }
            break;
        case andgate:
        case nandgate:
        case orgate:
        case norgate:
        case xorgate:
        case dtype:
        default: {
            inplink il = _netz->findinput(dvl, keyname);
            if (il != NULL && (il->connect != NULL)) {
                std::ostringstream oss, prevval;
                outplink ol = il->connect;
                devlink dl = _netz->findoutputdevice(ol);
                if (dl == NULL)
                    throw matterror("Should never reach here. Call Tim.", _scan.getFile(), keytk.at);

                prevval << *dl->id;

                if (ol->id != blankname)
                    prevval << *ol->id;
                getPredefinedError(dvl, keyname, prevval.str(), oss);
                throw matterror(oss.str(), _scan.getFile(), keytk.at);
            }
            break;
        }
    }
    stepAndPeek(tk);
}


bool parser::isLegalProperty(devlink dvl, name keyname) {
    return ( (dvl->kind == aclock && keyname == _nms->lookup("Period"))
            || (dvl->kind == aswitch && keyname == _nms->lookup("InitialValue")));
}


// value = signalname | number ;
void parser::parseValue(Token& tk, devlink dvl, Token& keytk) {
    Token valuetk = tk;
    name keyname = _nms->lookup(keytk.name);

    if (valuetk.type == TokType::Identifier) {
        // a signal
        Signal sig = parseSignalName(tk);

        if (isLegalProperty(dvl, keyname)) {
            throw matterror("Attempt to assign a signal to a property", _scan.getFile(), valuetk.at);
        }

        assignPin(dvl, keytk, valuetk, sig);
    } else if (valuetk.type == TokType::Number) {
        stepAndPeek(tk);

        if (isLegalProperty(dvl, keyname)) {
            assignProperty(dvl, keytk, valuetk);
        }
        else {
            if (valuetk.number != 0 && tk.number != 1)
                // Todo: improve error message
                throw matterror("Invalid signal.", _scan.getFile(), valuetk.at);
            Signal sig;
            sig.device = _nms->lookup(valuetk.number  ? "1" : "0");
            assignPin(dvl, keytk, valuetk, sig);
        }
    } else {
        throw matterror("Expected an Identifier or Number", _scan.getFile(), valuetk.at);
    }
}

void parser::assignPin(devlink dvl, Token keytk, Token valuetk, Signal sig) {
    name keyname = _nms->lookup(keytk.name);
    // Ensure signal exists
    signal_legality badSignal = isBadSignal(sig);
    if (badSignal) {
        if (badSignal == ILLEGAL_DEVICE) {
            throw matterror("Devices must be defined before being referenced", _scan.getFile(), valuetk.at);
        } else {
            // ILLEGAL_PIN
            std::ostringstream oss;
            oss << "Unable to set input pin. ";
            getUnknownPinError(sig, oss);
            throw matterror(oss.str(), _scan.getFile(), valuetk.at);
        }
    }

    // connect the gate
    bool success = false;
    if (!_netz->findinput(dvl, keyname))
        _netz->addinput(dvl, keyname, keytk.at);

    // todo: store token position?
    _netz->makeconnection(dvl->id, keyname, sig.device, sig.pin, success);
    if (!success)
        // Todo: improve error message
        throw matterror("Could not make connection", _scan.getFile(), keytk.at);
}

void parser::assignProperty(devlink dvl, Token keytk, Token valuetk) {
    bool success = false;
    if (dvl->kind == aswitch) {
        // Switch
        if (valuetk.type != TokType::Number || (valuetk.number != 0 && valuetk.number != 1))
            throw matterror("Switches must have initial values of either 0 or 1", _scan.getFile(), valuetk.at);

        asignal sig = valuetk.number ? high : low;
        _devz->setswitch(dvl->id, sig, success, keytk.at);

        if (!success)
            throw matterror("Could not set switch initial value", _scan.getFile(), valuetk.at);

    } else if (dvl->kind == aclock) {
        // Clock
        if (valuetk.type != TokType::Number || valuetk.number < 1 || valuetk.number > 32767)
            throw matterror("Clock periods must be integers between 1 and 32767", _scan.getFile(), valuetk.at);

        // Todo: these might want to store a different token position
        _devz->setclock(dvl->id, valuetk.number, success, keytk.at);

        if (!success)
            throw matterror("Could not set clock period", _scan.getFile(), valuetk.at);
    }
}



// definemonitor = "monitor" , monitorset , ";" ;
void parser::parseDefineMonitor(Token& tk) {
    for (;;) {
        parseMonitor(tk);

        if (tk.type == TokType::SemiColon) {
            stepAndPeek(tk);
            break;
        }
        else if (tk.type != TokType::Comma) {
            throw matterror("Expected a comma in the monitor list.", _scan.getFile(), tk.at);
        }
        stepAndPeek(tk);
    }
}


// monitor = signalname , [ "as" , signalname ] ;
void parser::parseMonitor(Token& tk) {
    Token montk = tk;
    Signal monsig, alisig;

    monsig = parseSignalName(tk);

    // Ensure signal exists
    signal_legality badSignal = isBadSignal(monsig);
    if (badSignal) {
        if (badSignal == ILLEGAL_DEVICE) {
            throw matterror("Devices must be defined before being monitored", _scan.getFile(), montk.at);
        } else {
            // ILLEGAL_PIN
            std::ostringstream oss;
            oss << "Unable to set monitor point. ";
            getUnknownPinError(monsig, oss);
            throw matterror(oss.str(), _scan.getFile(), montk.at);
        }
    }

    if (tk.type == TokType::AsKeyword) {
        stepAndPeek(tk);
        Token tkSig = tk;
        alisig = parseSignalName(tk);

        // Warn if signal exists
        if (!isBadSignal(alisig)) {
            // mattwarning w("Alias signal name already exists.", _scan.getFile(), tkSig);
            errs.report(mattwarning("Alias signal name already exists.", _scan.getFile(), tkSig.at));
        }
    }

    bool success = false;
    _mons->makemonitor(monsig.device, monsig.pin, success, alisig.device, alisig.pin);
    if (!success)
        // Todo: improve error message
        throw matterror("Could not make monitor", _scan.getFile(), tk.at);
}

parser::signal_legality parser::isBadSignal(Signal& sig) {
    devlink dvlnk = _netz->finddevice(sig.device);
    if (dvlnk == NULL)
        return ILLEGAL_DEVICE;
    if (_netz->findoutput(dvlnk, sig.pin) == NULL)
        return ILLEGAL_PIN;
    return LEGAL_SIGNAL;
}

// signalname = devicename , [ "." , pin ] ;
Signal parser::parseSignalName(Token& tk) {

    Signal ret;
    ret.device = blankname;
    ret.pin = blankname;

    if (tk.type != TokType::Identifier) {
        throw matterror("Expected a signal name.", _scan.getFile(), tk.at);
    }

    // Todo: handle 0 or 1 connections
    ret.device = _nms->lookup(tk.name);

    stepAndPeek(tk);

    if (tk.type == TokType::Dot) {
        stepAndPeek(tk);

        if (tk.type != TokType::Identifier) {
            throw matterror("Expected a pin name.", _scan.getFile(), tk.at);
        }

        ret.pin = _nms->lookup(tk.name);
        // Todo: ensure pin is acceptable identifier
        stepAndPeek(tk);
    }

    return ret;
}

const errorcollector& parser::errors() const {
    return errs;
}

