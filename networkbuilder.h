
#include "parser.h"
#include "names.h"
#include "scanner.h"
#include "network.h"
#include "devices.h"
#include "monitor.h"
#include "errorhandler.h"

#ifndef GF2_NETWORKBUILDER_H
#define GF2_NETWORKBUILDER_H




class networkbuilder
{
private:
    network* _netz;
    devices* _devz;
    monitor* _mons;
    scanner _scan;
    names* _nms;
    errorcollector errs;

public:
    networkbuilder(network* netz, devices* devz, monitor* mons, scanner& scan, names* nms);
    ~networkbuilder();
    
	void defineDevice(Token& devName, Token& type);
	void setInput(Token& devName, Token& keyTok, Token& valTok);
    void defineMonitor(Signal& monSig);
    void defineMonitor(Signal& monSig, Signal& aliSig);
};




#endif
