/* A unit tester framework for the parser
 * inputs are provided by a stream of tokens
 * the output is tested by a stream of expected actions to be 
 * matched with the actual actions taken by the parser. The 
 * scanner and network builder are simulated, with the inputs and 
 * outputs monitored.
 * 
 * Tests pass if the expected actions match the actual
 * actions taken by the parser on the input tokens.
 * 
 * Tests require the google test framework
 * https://github.com/google/googletest
 *
 * @author     Judge
 */

#include "parser.h"
#include "gtest/gtest.h"

#include <iostream>
#include <sstream>
#include <ostream>
#include <vector>

#include "../com/names.h"
#include "../com/errorhandler.h"
#include "../sim/network.h"
#include "scanner.h"

#include "networkbuilder.h"


// an Action is a representation of parser output
typedef enum {
    DefineDevice,
    SetOption,
    DefineMonitor
} actiontype;


struct Action {
    actiontype actype;
    namestring name1;
    namestring name2;
    namestring name3;
    namestring name4;
    devicekind devknd;
    int valnum;
};


::std::ostream& operator<<(::std::ostream& oss, const Action& ac) {
    return oss << "\t{ ac: " << ac.actype << ",\t"
        << "devicename: " << ac.name1 << ",\n\t"
        << "keyname/monpinname: " << ac.name2 << ",\t"
        << "valname/sigdevnm: " << ac.name3 << ",\n\t"
        << "sigpinname: " << ac.name4 << ",\t"
        << "devknd: " << ac.devknd << ",\t"
        << "valnum: " << ac.valnum << " };\n\t";
}


Action getDefineDeviceAction(namestring devnm, devicekind devknd) {
    return {
        DefineDevice,
        devnm,
        "",
        "",
        "",
        devknd,
        -1
    };
}

Action getSetOptionAction(namestring devnm, namestring keynm, int valnum, namestring valname, namestring sigpinname) {
    return {
        SetOption,
        devnm,
        keynm,
        valname,
        sigpinname,
        baddevice,
        valnum
    };
}
Action getSetOptionAction(namestring devnm, namestring keynm, int valnum) {
    return getSetOptionAction(devnm, keynm, valnum, "(blank)", "(blank)");
}
Action getSetOptionAction(namestring devnm, namestring keynm, namestring valname) {
    return getSetOptionAction(devnm, keynm, -1, valname, "(blank)");
}
Action getSetOptionAction(namestring devnm, namestring keynm, int valnum, namestring valname) {
    return getSetOptionAction(devnm, keynm, valnum, valname, "(blank)");
}
Action getSetOptionAction(namestring devnm, namestring keynm, namestring sigdevnm, namestring sigpinnm) {
    return getSetOptionAction(devnm, keynm, -1, sigdevnm, sigpinnm);
}

Action getDefineMonitorAction(namestring mondevnm, namestring monpinnm, namestring sigdevnm, namestring sigpinnm) {
    return {
        DefineMonitor,
        mondevnm,
        monpinnm,
        sigdevnm,
        sigpinnm,
        baddevice,
        -1
    };
}

bool operator==(const Action& a1, const Action& a2) {
    return a1.actype == a2.actype
        && a1.name1 == a2.name1
        && a1.name2 == a2.name2
        && a1.name3 == a2.name3
        && a1.name4 == a2.name4
        && a1.devknd == a2.devknd
        && a1.valnum == a2.valnum;
}


// parser test controller
// @author   Judge
class ParserTest : public ::testing::Test {

protected:
    static std::vector<Action> actionstaken;
    static std::vector<Token>::iterator _iter;
    static std::vector<Token> _tkstream;

    names *nmz;
    network *netz;
    devices *dmz;
    monitor *mmz;
    scanner *smz;
    networkbuilder *nwb;
    errorcollector errs;
    parser* psr;

    virtual void SetUp() {
        actionstaken.clear();
        nmz = new names();
        netz = new network(nmz);
        dmz = new devices(nmz, netz);
        mmz = new monitor(nmz, netz);
        smz = new scanner(nmz);
        nwb = new networkbuilder(netz, dmz, mmz, nmz, errs);
        psr = new parser(netz, dmz, mmz, smz, nmz);
    }

    virtual void TearDown() {
    }

    void testparserTokenStream(std::vector<Token> tkstream, std::vector<Action> acstream) {
        _tkstream = tkstream;
        _iter = _tkstream.begin();
        smz->step();
        psr->readin();
        EXPECT_EQ(acstream, actionstaken);
    }

public:
    static void pushAction(Action ac) {
        actionstaken.push_back(ac);
    }
    static Token getNextToken() {
        Token ret = *_iter;
        _iter++;
        return ret;
        //return *_iter++;
    }

    Token genToken(TokType tktype) {
        return Token(tktype);
    }
    Token genToken(TokType tktype, int num) {
        return Token(tktype, num);
    }
    Token genToken(TokType tktype, namestring str) {
        if (tktype == DeviceType)
            return Token(DeviceType, dmz->devkind(nmz->lookup(str)));
        else
            return Token(tktype, nmz->lookup(str));
    }
};

std::vector<Action> ParserTest::actionstaken;
std::vector<Token>::iterator ParserTest::_iter;
std::vector<Token> ParserTest::_tkstream;



/// Dummy network builder to accept parser output
// @author   Judge
networkbuilder::networkbuilder(network* netz, devices* devz, monitor* mons, names* nms, errorcollector& errc)
    : _netz(netz), _devz(devz), _mons(mons), _nms(nms), _errs(errc) {
}
networkbuilder::~networkbuilder() {
}

void networkbuilder::defineDevice(Token& devName, Token& type) {
    Action ac = getDefineDeviceAction(_nms->namestr(devName.id), type.devtype);
    ParserTest::pushAction(ac);
}

void networkbuilder::setInputValue(Token& devName, Token& keyTok, Token& valTok) {
    Action ac = getSetOptionAction(_nms->namestr(devName.id), _nms->namestr(keyTok.id), valTok.number, _nms->namestr(valTok.id));
    ParserTest::pushAction(ac);
}

void networkbuilder::setInputSignal(Token& devName, Token& keyTok, Signal& valSig) {
    Action ac = getSetOptionAction(_nms->namestr(devName.id), _nms->namestr(keyTok.id), _nms->namestr(valSig.device.id), _nms->namestr(valSig.pin.id));
    ParserTest::pushAction(ac);
}

void networkbuilder::defineMonitor(Signal& monSig) {
    Signal aliSig;
    defineMonitor(monSig, aliSig);
}

void networkbuilder::defineMonitor(Signal& monSig, Signal& aliSig) {
    Action ac = getDefineMonitorAction(_nms->namestr(monSig.device.id), _nms->namestr(monSig.pin.id), _nms->namestr(aliSig.device.id), _nms->namestr(aliSig.pin.id));
    ParserTest::pushAction(ac);
}

// Todo: move token to another file
Token::Token()
    : type(TokType::EndOfFile) {}

Token::Token(SourcePos pos, TokType t)
    : at(pos), type(t) {}

Token::Token(TokType t)
    : at(), type(t) {}

Token::Token(TokType t, name s)
    : at(), type(t), id(s) {}

Token::Token(TokType t, int num)
    : at(), type(t), number(num) {
    if (t == TokType::DeviceType) {
        devtype = devicekind(num);
    }
}


/// Dummy scanner to enter tokens to parser
// @author   Judge
scanner::scanner(names* nmz) {}
scanner::~scanner(){}
bool scanner::open(std::istream* is, std::string fname) {return true;}
std::string scanner::getFile() const {return "";}
Token scanner::step() {
    Token ret = _next;
    _next = readNext();
    return ret;
}
Token scanner::peek() {
    return _next;
}
Token scanner::readNext() {
    return ParserTest::getNextToken();
}





// parser unit tests

/* token types are:
    EndOfFile,
    DevKeyword,
    MonitorKeyword,
    AsKeyword,
    Equals,
    Colon,
    SemiColon,
    Comma,
    Brace,
    CloseBrace,
    Dot,
    Number,
    Identifier,
    DeviceType
*/
// Device types are:
//      aswitch, aclock, andgate, nandgate, orgate,
//      norgate, xorgate, dtype


// Action calls are
//
// getSetOptionAction("CLK", "Period", 5);
//          device name, option to set, value
//
// getSetOptionAction("G1", "I1", "G1", "(blank)")
//      device name, device pin, connection name, connection pin
//
// getDefineDeviceAction("CLK", aclock)
//        device name, device type
//
// getDefineMonitorAction("G1", "(blank)", "AliasName", "AliasPin")
//       device name, device pin, alias name, alias pin


// @author   Judge
TEST_F(ParserTest, SetClockPeriod){
    // dev CLK = CLOCK {Period : 5;}
    testparserTokenStream({
        genToken(DevKeyword),
        genToken(Identifier, "CLK"),
        genToken(Equals),
        genToken(DeviceType, "CLOCK"),
        genToken(Brace),
        genToken(Identifier, "Period"),
        genToken(Colon),
        genToken(Number, 5),
        genToken(SemiColon),
        genToken(CloseBrace),
        genToken(EndOfFile)
    },{
        getDefineDeviceAction("CLK", aclock),
        getSetOptionAction("CLK", "Period", 5)
    });
}

// @author   Judge
TEST_F(ParserTest, CreateAndGateLooped){
    // dev G1 = AND {I1 : G1;}
    testparserTokenStream({
        genToken(DevKeyword),
        genToken(Identifier, "G1"),
        genToken(Equals),
        genToken(DeviceType, "AND"),
        genToken(Brace),
        genToken(Identifier, "I1"),
        genToken(Colon),
        genToken(Identifier, "G1"),
        genToken(SemiColon),
        genToken(CloseBrace),
        genToken(EndOfFile)
    },{
        getDefineDeviceAction("G1", andgate),
        getSetOptionAction("G1", "I1", "G1", "(blank)")
    });
}

// @author   Judge
TEST_F(ParserTest, MonitorOrGat){
    // monitor G1 as ali.pin
    testparserTokenStream({
        genToken(MonitorKeyword),
        genToken(Identifier, "G1"),
        genToken(AsKeyword),
        genToken(Identifier, "ali"),
        genToken(Dot),
        genToken(Identifier, "pin"),
        genToken(SemiColon),
        genToken(EndOfFile)
    },{
        getDefineMonitorAction("G1", "(blank)", "ali", "pin")
    });
}



// testing catching errors
// @author   Judge
TEST_F(ParserTest, ErrorneousDevDefine){
    // devf G1 = XOR;
    testparserTokenStream({
        genToken(Identifier, "devf"),
        genToken(Identifier, "G1"),
        genToken(Equals),
        genToken(DeviceType, "XOR"),
        genToken(SemiColon),
        genToken(EndOfFile)
    },{
        // parser should take no action
    });
}


// testing device type as name
// @author   March
TEST_F(ParserTest, DeviceTypeAsName){
    // dev NAND = NAND;
    testparserTokenStream({
        genToken(DevKeyword),
        genToken(Identifier, "NAND"),
        genToken(Equals),
        genToken(DeviceType, "NAND"),
        genToken(SemiColon),
        genToken(EndOfFile)
    }, {
        // the scanner is responsible for checking such things
        getDefineDeviceAction("NAND", nandgate)
    });
}

// testing misspelt option
// @author   March
TEST_F(ParserTest, MisspeltOption){
    // dev CLK = CLOCK {Perod : 4; }
    testparserTokenStream({
        genToken(DevKeyword),
        genToken(Identifier, "CLK"),
        genToken(Equals),
        genToken(DeviceType, "CLOCK"),
        genToken(Brace),
        genToken(Identifier, "Perod"),
        genToken(Colon),
        genToken(Number, 4),
        genToken(SemiColon),
        genToken(CloseBrace),
        genToken(EndOfFile)
    }, {
        // the typo of perod would be caught in network builder
        getDefineDeviceAction("CLK", aclock),
        getSetOptionAction("CLK", "Perod", 4)
    });
}
