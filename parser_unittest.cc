
#include "parser.h"
#include "gtest/gtest.h"

#include <iostream>
#include <sstream>
#include <vector>
#include "errorhandler.h"
#include "scanner.h"
#include "names.h"
#include "network.h"

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
    parser* psr;

    virtual void SetUp() {
        actionstaken.clear();
        nmz = new names();
        netz = new network(nmz);
        dmz = new devices(nmz, netz);
        mmz = new monitor(nmz, netz);
        smz = new scanner(nmz);
        nwb = new networkbuilder(netz, dmz, mmz, smz, nmz);
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
};

std::vector<Action> ParserTest::actionstaken;
std::vector<Token>::iterator ParserTest::_iter;
std::vector<Token> ParserTest::_tkstream;



/// Dummy network builder to accept parser output 

networkbuilder::networkbuilder(network* netz, devices* devz, monitor* mons, scanner* scan, names* nms)
    : _netz(netz), _devz(devz), _mons(mons), _scan(scan), _nms(nms) {
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
scanner::scanner(names* nmz) {}
scanner::~scanner(){}
void scanner::open(std::istream* is, std::string fname) {}
std::string scanner::getFile() const {}
Token scanner::step() {
    Token ret = _next;
    _next = readNext();
    return ret;
}
Token scanner::peek() const {
    return _next;
}
Token scanner::readNext() {
    return ParserTest::getNextToken();
}




// parser unit tests

TEST_F(ParserTest, ExampleParserTest){
    testparserTokenStream({
        Token(DevKeyword),
        Token(Identifier, nmz->lookup("CLK")),
        Token(Equals),
        Token(DeviceType, nmz->lookup("CLOCK")),
        Token(Brace),
        Token(Identifier, nmz->lookup("Period")),
        Token(Colon),
        Token(Number, 5),
        Token(CloseBrace),
        Token(EndOfFile)
    },{
        getDefineDeviceAction("CLK", aclock),
        getSetOptionAction("CLK", "Period", 5)
    });
}


TEST_F(ParserTest, ExampleParserTest2){
    testparserTokenStream({
        Token(DevKeyword),
        Token(Identifier, nmz->lookup("G1")),
        Token(Equals),
        Token(DeviceType, nmz->lookup("AND")),
        Token(Brace),
        Token(Identifier, nmz->lookup("I1")),
        Token(Colon),
        Token(Identifier, nmz->lookup("G1")),
        Token(CloseBrace),
        Token(EndOfFile)
    },{
        getDefineDeviceAction("G1", andgate),
        getSetOptionAction("G1", "I1", "G1", "(blank)")
    });
}