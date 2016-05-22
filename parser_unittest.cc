
#include "parser.h"
#include "gtest/gtest.h"

#include <sstream>
#include <vector>
#include "errorhandler.h"
#include "scanner.h"
#include "names.h"

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
    return getSetOptionAction(devnm, keynm, valnum, "", "");
}
Action getSetOptionAction(namestring devnm, namestring keynm, namestring valname) {
    return getSetOptionAction(devnm, keynm, -1, valname, "");
}
Action getSetOptionAction(namestring devnm, namestring keynm, int valnum, namestring valname) {
    return getSetOptionAction(devnm, keynm, valnum, valname, "");
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

    }

public:
    static void pushAction(Action ac) {
        actionstaken.push_back(ac);
    }
};

std::vector<Action> ParserTest::actionstaken;



/// Dummy network builder to accept parser output 

networkbuilder::networkbuilder(network* netz, devices* devz, monitor* mons, scanner* scan, names* nms)
    : _netz(netz), _devz(devz), _mons(mons), _scan(scan), _nms(nms) {
}
networkbuilder::~networkbuilder() {
}

void networkbuilder::defineDevice(Token& devName, Token& type) {
    Action ac = getDefineDeviceAction(*devName.id, type.devtype);
    ParserTest::pushAction(ac);
}

void networkbuilder::setInputValue(Token& devName, Token& keyTok, Token& valTok) { 
    Action ac = getSetOptionAction(*devName.id, *keyTok.id, valTok.number, *valTok.id);
    ParserTest::pushAction(ac);
}

void networkbuilder::setInputSignal(Token& devName, Token& keyTok, Signal& valSig) { 
    Action ac = getSetOptionAction(*devName.id, *keyTok.id, *valSig.device.id, *valSig.pin.id);
    ParserTest::pushAction(ac);
}



void networkbuilder::defineMonitor(Signal& monSig) {
    Signal aliSig;
    defineMonitor(monSig, aliSig);    
}

void networkbuilder::defineMonitor(Signal& monSig, Signal& aliSig) {
    Action ac = getDefineMonitorAction(*monSig.device.id, *monSig.pin.id, *aliSig.device.id, *aliSig.pin.id);
    ParserTest::pushAction(ac);
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
        getSetOptionAction("CLK", "InitialPeriod", 5)
    });
}