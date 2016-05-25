
#ifdef EXPERIMENTAL
#ifndef GF2_IMPORTEDDEVICE_H
#define GF2_IMPORTEDDEVICE_H

#include <vector>

#include "../com/names.h"
#include "../com/errorhandler.h"
#include "network.h"
// #include "../sim/devices.h"
// #include "../sim/monitor.h"
class devices;
class monitor;


struct importeddevice {
    names* nmz;
    network* netz;
    devices* dmz;
    monitor* mmz;
    errorcollector& errs;

    std::vector<devicerec*> inputs;
    std::vector<std::pair<name, outputrec*>> outputs;

    importeddevice(names* nm, errorcollector& errc);
    ~importeddevice();

    bool scanAndParse(std::string file);

    void tick();
    void execute();

    bool hasInput(name pin) const;
    bool hasOutput(name mon) const;
    bool setInput(name pin, asignal value);
    bool getOutput(name mon, asignal& value);
};
typedef importeddevice* importedlink;



#endif
#endif
