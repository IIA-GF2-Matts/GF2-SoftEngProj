
#include <iostream>

#include "names.h"
#include "network.h"
#include "devices.h"
#include "monitor.h"
#include "scanner.h"
#include "parser.h"
#include "userint.h"


int main(int argc, char const *argv[]) {
    if (argc != 2) { // check we have one command line argument
        std::cout << "Usage:      " << argv[0] << " [filename]" << std::endl;
        exit(1);
    }

    names* nmz = new names();
    network* netz = new network(nmz);
    devices* dmz = new devices(nmz, netz);
    monitor* mmz = new monitor(nmz, netz);
    fscanner* smz = new fscanner(nmz);
    smz->open(argv[1]);
    parser* pmz = new parser(netz, dmz, mmz, smz, nmz);

    if (pmz->readin ()) { // check the logic file parsed correctly
        // Construct the text-based interface
        userint umz(nmz, dmz, mmz);
        umz.userinterface();
    }

    delete pmz;
    // delete smz;
    delete mmz;
    delete dmz;
    delete netz;
    delete nmz;

    return 0;
}
