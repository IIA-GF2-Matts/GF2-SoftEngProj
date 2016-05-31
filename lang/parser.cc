
#include <iostream>
#include <sstream>
#include <set>
#include <iterator>

#include "../com/errorhandler.h"
#include "../com/names.h"
#include "../com/autocorrect.h"
#include "../com/formatstring.h"
#include "../com/localestrings.h"

#include "scanner.h"

#include "parser.h"


parser::parser(network* netz, devices* devz, monitor* mons, scanner* scan, names* nms)
    : _netz(netz), _devz(devz), _mons(mons), _scan(scan), _nms(nms)
    , netbuild(netz, devz, mons, nms, errs) {
        //_devz->debug(true);
}

parser::~parser() {
}


bool parser::readin() {

    Token tk;

    try {
        for (;;) {
            tk = _scan->peek();
            parseFile(tk);

            if (_scan->getParent()) {
                // Go back to original file.
                scanner* old = _scan;
                _scan = _scan->getParent();
                delete old;
            }
            else
                break;
        }

       _netz->checknetwork(errs);
    }
    catch (matterror& e) {
        errs.report(e);
    }

    errs.print(std::cout);

    std::cout << formatString(
        t("File parsed with {0} errors and {1} warnings."),
            errs.errCount(),
            errs.warnCount())
        << std::endl;

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
                try {
                    if (tk.type == TokType::EndOfFile) {
                        stepAndPeek(tk);
                        break;
                    }
                    stepAndPeek(tk);
                }
                catch (mattsyntaxerror& e) {
                    errs.report(e);
                }
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
        case TokType::ImportKeyword:
            stepAndPeek(tk);
            parseImport(tk);
            break;
        default:
            throw mattsyntaxerror("Unexpected token type. Expected a device or monitor definition (beginning with dev or monitor keywords).", tk.at);
    }
}


/// import = "import" , string , ";" ;
void parser::parseImport(Token& tk) {
    if (tk.type != TokType::String) {
        throw mattsyntaxerror("Expected a string filename to import.", tk.at);
    }

    Token incStr = tk;

    stepAndPeek(tk);

    if (tk.type != TokType::SemiColon) {
        throw mattsyntaxerror("Missing the semicolon on the end of import statement.", tk.at);
    }

    stepAndPeek(tk);

    // check for recursive imports
    //todo: check tocken position
    scanner* sc = _scan;
    while (sc) {
        if (sc->getFile() == incStr.str) {
            throw mattsemanticerror("File already included.", incStr.at);
        }

        sc = sc->getParent();
    }

    // Use new scanner for file.
    fscanner* f = new fscanner(_nms);
    if (!f->open(incStr.str)) {
        delete f;

        throw mattruntimeerror(t("Unable to read import file."), incStr.at);
    }

    f->setParent(_scan);
    _scan = f;
    tk = _scan->peek();
}


// definedevice = "dev" , devicename , [ "=" , type ] , data ;
// With extensions enabled:
// type         = string | identifier
void parser::parseDefineDevice(Token& tk) {
    Token nameToken;

    if (tk.type != TokType::Identifier) {
        throw mattsyntaxerror(t("Expected a device name."), tk.at);
    }
    // dev, as, monitor are handled by the scanner and made *Keywords
    // device types are handled by the scanner and made DeviceTypes

    nameToken = tk;

    stepAndPeek(tk);

    if (tk.type == TokType::Equals) {

        stepAndPeek(tk);

        if (tk.type == TokType::String) {
            // dev DN = "file";

            netbuild.importDevice(nameToken, tk);

            stepAndPeek(tk);
        }
        else {
            if (tk.type != TokType::DeviceType) {
                std::string errmsg;
                if (tk.type == TokType::Identifier) {
                    // get the closest match input and display a suggestion
                    errmsg = t("Expected a device type. ")
                        + getClosestMatchError(_nms->namestr(tk.id), devicesset);
                } else {
                    // user got it hopelessly wrong
                    errmsg = t("Expected a device type such as CLOCK, SWITCH, NAND or others (see documentation).");
                }
                throw mattsyntaxerror(errmsg, tk.at);
            }

            netbuild.defineDevice(nameToken, tk);

            stepAndPeek(tk);
        }
    }

    parseData(tk, nameToken);
}


// data = optionset | ";" ;
void parser::parseData(Token& tk, Token& devName) {
    if (tk.type == TokType::SemiColon) {
        stepAndPeek(tk);
    }
    else if (tk.type == TokType::Brace) {
        stepAndPeek(tk);

        parseOptionSet(tk, devName);
    }
    else {
        throw mattsyntaxerror(t("Unexpected token. Expecting ; (if the end of the statement), or { (if beginning an option block)."), tk.at);
    }
}


// optionset = "{" , { option } , "}" ;
void parser::parseOptionSet(Token& tk, Token& devName) {
    while (tk.type != TokType::CloseBrace) {
        if (tk.type == TokType::EndOfFile) {
            throw mattsyntaxerror(t("Unterminated braces."), tk.at);
        }

        try {
            parseOption(tk, devName);
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


// option = key , ":" , value , ";" ;
void parser::parseOption(Token& tk, Token& devName) {
    Token keytk, valuetk;

    if (tk.type != TokType::Identifier) {
        throw mattsyntaxerror(t("Expected a key."), tk.at);
    }

    keytk = tk;

    stepAndPeek(tk);

    if (tk.type != TokType::Colon) {
        throw mattsyntaxerror(t("Expected colon."), tk.at);
    }

    stepAndPeek(tk);
    parseValue(tk, devName, keytk);

    if (tk.type != TokType::SemiColon) {
        throw mattsyntaxerror(t("Missing a semicolon on the end."), tk.at);
    }
    stepAndPeek(tk);
}


// value = signalname | number | bitstream ;
void parser::parseValue(Token& tk, Token& devName, Token& keyTok) {
    Token valuetk = tk;

    if (valuetk.type == TokType::Identifier) {
        // a signal
        Signal sig = parseSignalName(tk);
        netbuild.setInputSignal(devName, keyTok, sig);

    } else if (valuetk.type == TokType::Number
        || valuetk.type == TokType::Bitstream) {

        netbuild.setInputValue(devName, keyTok, valuetk);
        stepAndPeek(tk);

    } else {
        throw mattsyntaxerror(t("Expected an Identifier or Number"), valuetk.at);
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
            throw mattsyntaxerror(t("Expected a comma in the monitor list."), tk.at);
        }
        stepAndPeek(tk);
    }
}


// monitor = signalname , [ "as" , signalname ] ;
void parser::parseMonitor(Token& tk) {
    Signal monsig, alisig;

    monsig = parseSignalName(tk);


    if (tk.type == TokType::AsKeyword) {
        stepAndPeek(tk);
        alisig = parseSignalName(tk);
    }

    netbuild.defineMonitor(monsig, alisig);
}


// signalname = devicename , [ "." , pin ] ;
Signal parser::parseSignalName(Token& tk) {

    Signal ret;

    if (tk.type != TokType::Identifier) {
        throw mattsyntaxerror(t("Expected a signal name."), tk.at);
    }

    ret.device = tk;

    stepAndPeek(tk);

    if (tk.type == TokType::Dot) {
        stepAndPeek(tk);

        if (tk.type != TokType::Identifier) {
            throw mattsyntaxerror(t("Expected a pin name."), tk.at);
        }

        ret.pin = tk;
        stepAndPeek(tk);
    }

    return ret;
}

const errorcollector& parser::errors() const {
    return errs;
}

