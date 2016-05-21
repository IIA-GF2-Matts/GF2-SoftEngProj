
#include <iostream>
#include <sstream>
#include <set>
#include <iterator>

#include "errorhandler.h"
#include "scanner.h"
#include "names.h"
#include "autocorrect.h"

#include "parser.h"



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

parser::parser(network* netz, devices* devz, monitor* mons, scanner* scan, names* nms)
    : _netz(netz), _devz(devz), _mons(mons), _scan(scan), _nms(nms) {
        //_devz->debug(true);
}

parser::~parser() {
}


bool parser::readin() {
    Token tk = _scan->peek();
    parseFile(tk);

    _netz->checknetwork(errs);

    std::cout << "File parsed with "
            << errs.errCount() << " errors and "
            << errs.warnCount() << " warnings." << std::endl;

    return errs.errCount() == 0;
}

void parser::stepAndPeek(Token& tk) {
    _scan->step();
    tk = _scan->peek();
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
            throw mattsyntaxerror("Unexpected token type. Expected a device or monitor definition.", tk.at);
    }
}


// definedevice = "dev" , devicename , [ "=" , type ] , data ;
void parser::parseDefineDevice(Token& tk) {
    Token nameToken;

    if (tk.type != TokType::Identifier) {
        throw mattsyntaxerror("Expected a device name.", tk.at);
    }
    // dev, as, monitor are handled by the scanner and made *Keywords
    // device types are handled by the scanner and made DeviceTypes

    nameToken = tk;

    stepAndPeek(tk);

    if (tk.type == TokType::Equals) {
        // Semantic check: has devicename been defined before?
        if (_netz->finddevice(nameToken.id) != NULL) {
            throw mattsemanticerror("Device types may not be assigned to devices that already exist.", tk.at);
        }

        stepAndPeek(tk);

        if (tk.type != TokType::DeviceType) {
            std::ostringstream oss;
            oss << "Expected device type. ";
            // Todo: Alternative if number?
            if (tk.type == TokType::Identifier)
                getClosestMatchError(*tk.id, devicesset, oss);
            throw mattsyntaxerror(oss.str(), tk.at);
        }
        // make device (which adds it to the network)
        bool success;
        if (tk.devtype == aswitch)
            // -1 sets switch to floating
            _devz->makedevice(tk.devtype, nameToken.id, -1, success, nameToken.at);
        else
            _devz->makedevice(tk.devtype, nameToken.id, 0, success, nameToken.at);

        // Debug
        /*
        std::cout << "made device " << _nms-namestr(nameToken.id);
        std::cout << " regtype ";
        _devz->writedevice(_netz->finddevice(nameToken.id)->kind);
        //_devz->writedevice(_devz->devkind(nameToken.id));
        cout << std::endl;
        */
        if (!success) {
            throw mattruntimeerror("Unable to add device.", tk.at);
            // TODO: Better error message? Shouldn't ever reach here.
        }

        stepAndPeek(tk);
    }

    parseData(tk, nameToken.id);
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
        throw mattsyntaxerror("Unexpected token. Expecting ; or {.", tk.at);
    }
}


// optionset = "{" , { option } , "}" ;
void parser::parseOptionSet(Token& tk, name dv) {
    while (tk.type != TokType::CloseBrace) {
        if (tk.type == TokType::EndOfFile) {
            throw mattsyntaxerror("Unterminated braces.", tk.at);
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
void parser::getPredefinedError(devlink dvl, name key, T prevval, std::ostringstream& oss) {
    // todo: note should be a separate note message
    oss << "Attempt to redefine " //<< *_devz->getname(dvl->kind)
        << *dvl->id << "." << *key
        << ". Note: previously defined as "
        << prevval << " at position " << dvl->definedAt;
}

void parser::getClosestMatchError(namestring nm, std::set<cistring> candidates, std::ostringstream& oss) {
    std::list<cistring> matches;
    int dist = closestMatches(nm, candidates, matches);

    if (dist < 3 && matches.size() > 0) {
        auto i = matches.begin();

        oss << "Did you mean";
        for (; i != std::prev(matches.end()); ++i) {
            oss << " " << *i;
        }
        if (matches.size() > 1)
            oss << " or";
        oss << " " << *std::prev(matches.end()) << "?";
    }
}



// option = key , ":" , value , ";" ;
void parser::parseOption(Token& tk, name dv) {
    devlink dvl = _netz->finddevice(dv);
    Token keytk, valuetk;
    parseKey(tk, dvl, keytk);

    if (tk.type != TokType::Colon) {
        throw mattsyntaxerror("Expected colon.", tk.at);
    }

    stepAndPeek(tk);
    parseValue(tk, dvl, keytk);

    if (tk.type != TokType::SemiColon) {
        throw mattsyntaxerror("Missing a semicolon on the end.", tk.at);
    }
    stepAndPeek(tk);
}

// key = identifier ;
void parser::parseKey(Token& tk, devlink dvl, Token& keytk) {

    if (tk.type != TokType::Identifier) {
        throw mattsyntaxerror("Expected a key.", tk.at);
    }

    keytk = tk;

    switch(dvl->kind) {
        // Todo: quality check errors
        case aswitch:
            if (*keytk.id != "InitialValue")
                throw mattsemanticerror("Switches may only have an `InitialValue` attribute.", keytk.at);
            break;
        case aclock:
            if (*keytk.id != "Period")
                throw mattsemanticerror("Clocks may only have a `Period` attribute.", keytk.at);
            break;
        case andgate:
            if (!isLegalGateInputNamestring(*keytk.id, 16))
                throw mattsemanticerror("AND gates may only have input pin attributes (up to 16), labelled I1 to I16", keytk.at);
            break;
        case nandgate:
            if (!isLegalGateInputNamestring(*keytk.id, 16))
                throw mattsemanticerror("NAND gates may only have input pin attributes (up to 16), labelled I1 to I16", keytk.at);
            break;
        case orgate:
            if (!isLegalGateInputNamestring(*keytk.id, 16))
                throw mattsemanticerror("OR gates may only have input pin attributes (up to 16), labelled I1 to I16", keytk.at);
            break;
        case norgate:
            if (!isLegalGateInputNamestring(*keytk.id, 16))
                throw mattsemanticerror("NOR gates may only have input pin attributes (up to 16), labelled I1 to I16", keytk.at);
            break;
        case xorgate:
            if (!isLegalGateInputNamestring(*keytk.id, 2))
                throw mattsemanticerror("XOR gates may only have input pin attributes (up to 2), labelled I1 to I2", keytk.at);
            break;
        case dtype:
            // Todo: Name table comparison rather than string comparison
            if (!(*keytk.id == "DATA" || *keytk.id == "CLK" || *keytk.id == "SET" || *keytk.id == "CLEAR")) {
                std::ostringstream oss;
                oss << "DTYPE devices may only have DATA, CLK, SET or CLEAR input pins assigned. ";
                getClosestMatchError(*keytk.id, dtypeoutset, oss);
                throw mattsemanticerror(oss.str(), keytk.at);
            }
            break;
        case baddevice:
        default:
            // Should never reach here
            // Todo: better error message?
            throw mattruntimeerror("Could not assign key to a bad device type", keytk.at);
    }

    // check if key has already been defined for particular device
    switch(dvl->kind) {
        // Todo: check errors
        case aswitch:
            if (dvl->swstate != floating) {
                std::ostringstream oss;
                getPredefinedError(dvl, keytk.id, dvl->swstate == high ? 1 : 0, oss);
                throw mattsemanticerror(oss.str(), keytk.at);
            }
            break;
        case aclock:
            if (dvl->frequency != 0) {
                std::ostringstream oss;
                getPredefinedError(dvl, keytk.id, dvl->frequency, oss);
                throw mattsemanticerror(oss.str(), keytk.at);
            }
            break;
        case andgate:
        case nandgate:
        case orgate:
        case norgate:
        case xorgate:
        case dtype:
        default: {
            inplink il = _netz->findinput(dvl, keytk.id);
            if (il != NULL && (il->connect != NULL)) {
                std::ostringstream oss, prevval;
                outplink ol = il->connect;
                devlink dl = _netz->findoutputdevice(ol);
                if (dl == NULL)
                    // Should never reach here
                    throw mattruntimeerror("Device with requested output pin could not be found in the network. Connection not made.", keytk.at);

                prevval << *dl->id;

                if (ol->id != blankname)
                    prevval << *ol->id;
                getPredefinedError(dvl, keytk.id, prevval.str(), oss);
                throw mattsemanticerror(oss.str(), keytk.at);
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

    if (valuetk.type == TokType::Identifier) {
        // a signal
        Signal sig = parseSignalName(tk);

        if (isLegalProperty(dvl, keytk.id)) {
            throw mattsemanticerror("Attempt to assign a signal to a property", valuetk.at);
        }

        assignPin(dvl, keytk, valuetk, sig);
    } else if (valuetk.type == TokType::Number) {
        stepAndPeek(tk);

        if (isLegalProperty(dvl, keytk.id)) {
            assignProperty(dvl, keytk, valuetk);
        }
        else {
            if (valuetk.number != 0 && tk.number != 1)
                // Todo: improve error message
                throw mattsyntaxerror("Invalid signal.", valuetk.at);
            Signal sig;
            sig.device = valuetk;
            sig.device.type = TokType::Identifier;
            sig.device.id = _nms->lookup(valuetk.number  ? "1" : "0");
            assignPin(dvl, keytk, valuetk, sig);
        }
    } else {
        throw mattsyntaxerror("Expected an Identifier or Number", valuetk.at);
    }
}

void parser::assignPin(devlink dvl, Token keytk, Token valuetk, Signal sig) {
    // Ensure signal exists
    signal_legality badSignal = isBadSignal(sig);
    if (badSignal) {
        if (badSignal == ILLEGAL_DEVICE) {
            throw mattsemanticerror("Devices must be defined before being referenced", valuetk.at);
        } else {
            // ILLEGAL_PIN
            std::ostringstream oss;
            oss << "Unable to set input pin. ";
            getUnknownPinError(sig, oss);
            throw mattsemanticerror(oss.str(), valuetk.at);
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

void parser::assignProperty(devlink dvl, Token keytk, Token valuetk) {
    bool success = false;
    if (dvl->kind == aswitch) {
        // Switch
        if (valuetk.type != TokType::Number || (valuetk.number != 0 && valuetk.number != 1))
            throw mattsemanticerror("Switches must have initial values of either 0 or 1", valuetk.at);

        asignal sig = valuetk.number ? high : low;
        _devz->setswitch(dvl->id, sig, success, keytk.at);

        if (!success)
            throw mattruntimeerror("Could not set switch initial value", valuetk.at);

    } else if (dvl->kind == aclock) {
        // Clock
        if (valuetk.type != TokType::Number || valuetk.number < 1 || valuetk.number > 32767)
            throw mattsemanticerror("Clock periods must be integers between 1 and 32767", valuetk.at);

        // Todo: these might want to store a different token position
        _devz->setclock(dvl->id, valuetk.number, success, keytk.at);

        if (!success)
            throw mattruntimeerror("Could not set clock period", valuetk.at);
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
            throw mattsyntaxerror("Expected a comma in the monitor list.", tk.at);
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
            throw mattsemanticerror("Devices must be defined before being monitored", montk.at);
        } else {
            // ILLEGAL_PIN
            std::ostringstream oss;
            oss << "Unable to set monitor point. ";
            getUnknownPinError(monsig, oss);
            throw mattruntimeerror(oss.str(), montk.at);
        }
    }

    if (tk.type == TokType::AsKeyword) {
        stepAndPeek(tk);
        Token tkSig = tk;
        alisig = parseSignalName(tk);

        // Warn if signal exists
        if (!isBadSignal(alisig)) {
            errs.report(mattwarning("Alias signal name already exists.", tkSig.at));
        }
    }

    bool success = false;
    _mons->makemonitor(monsig.device.id, monsig.pin.id, success, alisig.device.id, alisig.pin.id);
    if (!success)
        // Todo: improve error message
        throw mattruntimeerror("Could not make monitor", tk.at);
}

parser::signal_legality parser::isBadSignal(Signal& sig) {
    devlink dvlnk = _netz->finddevice(sig.device.id);
    if (dvlnk == NULL)
        return ILLEGAL_DEVICE;
    if (_netz->findoutput(dvlnk, sig.pin.id) == NULL)
        return ILLEGAL_PIN;
    return LEGAL_SIGNAL;
}

// signalname = devicename , [ "." , pin ] ;
Signal parser::parseSignalName(Token& tk) {

    Signal ret;

    if (tk.type != TokType::Identifier) {
        throw mattsyntaxerror("Expected a signal name.", tk.at);
    }

    ret.device = tk;

    stepAndPeek(tk);

    if (tk.type == TokType::Dot) {
        stepAndPeek(tk);

        if (tk.type != TokType::Identifier) {
            throw mattsyntaxerror("Expected a pin name.", tk.at);
        }

        ret.pin = tk;
        // Todo: ensure pin is acceptable identifier
        stepAndPeek(tk);
    }

    return ret;
}

const errorcollector& parser::errors() const {
    return errs;
}

