
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

#include "networkbuilder.h"



networkbuilder::networkbuilder(network* netz, devices* devz, monitor* mons, scanner& scan, names* nms)
    : _netz(netz), _devz(devz), _mons(mons), _scan(scan), _nms(nms) {
        //_devz->debug(true);
}

networkbuilder::~networkbuilder() {
}


void networkbuilder::defineDevice(Token& devName, Token& type) {
	
}


void networkbuilder::setInput(Token& devName, Token& keyTok, Token& valTok) {
    
}

void networkbuilder::defineMonitor(Signal& monSig) {
    Signal aliSig;
	defineMonitor(monSig, aliSig);    
}

void networkbuilder::defineMonitor(Signal& monSig, Signal& aliSig) {

}

