
#include <iostream>

#include "errorhandler.h"
#include "scanner.h"

#include "parser.h"


parser::parser(network* netz, devices* devz, monitor* mons, scanner& scan, names* nms)
    : _netz(netz), _devz(devz), _mons(mons), _scan(scan), _nms(nms), errs(cout) {
}

parser::~parser() {
}


bool parser::readin() {
    Token tk = _scan.peek();
    parseFile(tk);

    return true;
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
            errs << e.what() << std::endl;
            return;
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
            throw matterror("Unexpected token type. Expected a device or monitor defintion.", _scan.getFile(), tk.at);
    }
}


// definedevice = "dev" , devicename , [ "=" , type ] , data ;
void parser::parseDefineDevice(Token& tk) {
    Token nameToken;

    if (tk.type != TokType::Identifier) {
        throw matterror("Expected a device name.", _scan.getFile(), tk.at);
    }
    if (deviceTypes.find(tk.name) != deviceTypes.end()) {
    	throw matterror("Device name must not be the same as a reserved device type", _scan.getFile(), tk.at);
    }
    // dev, as, monitor should be handled by the scanner
    // may want to test anyway

    nameToken = tk;
    name dv = blankname;
    // Todo: Name lookup

    stepAndPeek(tk);

    if (tk.type == TokType::Equals) {
        // Semantic check: has devicename been defined before?

        stepAndPeek(tk);

        if (tk.type != TokType::DeviceType) {
            // Todo: Suggest types
            throw matterror("Expected device type.", _scan.getFile(), tk.at);
        }

        // Create device of type, add it to the network
        name newDeviceName = _nms->lookup(nameToken.name);

        devlink newDeviceLink;
        if (tk.name == "CLOCK") {
    		_netz->adddevice(aclock, newDeviceName, newDeviceLink);
    	} else if (tk.name == "SWITCH") {
    		_netz->adddevice(aswitch, newDeviceName, newDeviceLink);
    	} else if (tk.name == "AND") {
    		_netz->adddevice(andgate, newDeviceName, newDeviceLink);
    	} else if (tk.name == "NAND") {
    		_netz->adddevice(nandgate, newDeviceName, newDeviceLink);
    	} else if (tk.name == "OR") {
    		_netz->adddevice(orgate, newDeviceName, newDeviceLink);
    	} else if (tk.name == "NOR") {
    		_netz->adddevice(norgate, newDeviceName, newDeviceLink);
    	} else if (tk.name == "DTYPE") {
    		_netz->adddevice(xorgate, newDeviceName, newDeviceLink);
    	} else if (tk.name == "XOR") {
    		_netz->adddevice(dtype, newDeviceName, newDeviceLink);
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

        parseOption(tk, dv);
    }

    stepAndPeek(tk);
}


// option = key , ":" , value , ";" ;
void parser::parseOption(Token& tk, name dv) {
    Token key;

    if (tk.type != TokType::Identifier) {
        throw matterror("Expected a key.", _scan.getFile(), tk.at);
    }

    key = tk;
    // Todo: Name lookup.

    stepAndPeek(tk);

    if (tk.type != TokType::Colon) {
        throw matterror("Expected colon.", _scan.getFile(), tk.at);
    }

    stepAndPeek(tk);

    if (tk.type == TokType::Identifier) {
        Signal value = parseSignalName(tk);

        // Todo: Create the connection.
        // if (is connection not setting a property)
        // netz->makeconnection(dv, key, value.device, value.pin, ok);
    }
    else if (tk.type == TokType::Number) {
        Token num = tk;

        // Todo: Check the value
        // Checl property is not already set.
        // Set the property.

        stepAndPeek(tk);
    }

    if (tk.type != TokType::SemiColon) {
        throw matterror("Missing a semicolon on the end.", _scan.getFile(), tk.at);
    }
    stepAndPeek(tk);
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

    Signal sig = parseSignalName(tk);

    if (tk.type == TokType::AsKeyword) {
        stepAndPeek(tk);

        Signal alias = parseSignalName(tk);
        // Todo: Warn if alias already used.
    }

    // Todo: Add to monitors list.
}


// signalname = devicename , [ "." , pin ] ;
Signal parser::parseSignalName(Token& tk) {

    Signal ret;
    ret.device = blankname;
    ret.pin = blankname;

    if (tk.type != TokType::Identifier) {
        throw matterror("Expected a signal name.", _scan.getFile(), tk.at);
    }

    Token devName = tk;
    // Todo : Name lookup

    stepAndPeek(tk);

    if (tk.type == TokType::Dot) {
        stepAndPeek(tk);

        if (tk.type != TokType::Identifier) {
            throw matterror("Expected a pin name.", _scan.getFile(), tk.at);
        }

        Token pinName = tk;
        // Todo: Name lookup

        stepAndPeek(tk);
    }

    return ret;
}
