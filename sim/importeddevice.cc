
#include <map>
#include <string>

#include "../com/localestrings.h"
#include "../lang/scanner.h"
#include "../lang/parser.h"

#include "importeddevice.h"


/** Initialises a new importeddevice structure
 *
 * @author Diesel
 */
importeddevice::importeddevice(names* nm, errorcollector& errc)
        : nmz(nm), errs(errc) {
    netz = new network(nmz);
    dmz = new devices(nmz, netz);
    mmz = new monitor(nmz, netz);
}

/** Clears resources allocated by importeddevice
 *
 * @author Diesel
 */
importeddevice::~importeddevice() {
    delete mmz;
    delete dmz;
    delete netz;
}


/** Reads in a Mattlang circuit definition and parses it to a network.
 *
 * @author Diesel
 */
bool importeddevice::scanAndParse(std::string file) {
    fscanner* smz = new fscanner(nmz);
    if (smz->open(file)) {
        parser pmz(netz, dmz, mmz, smz, nmz);

        bool ok = pmz.readin();

        if (ok) {
            // Find inputs
            inputs = netz->findswitches();

            // Find outputs
            name D, P;
            for (int n = 0; n < mmz->moncount(); n++) {
                mmz->getmonname(n, D, P);
                if (P != blankname) {
                    errs.report(mattwarning(t("In imported devices, monitors using pin names are ignored. "), mmz->getdefinedpos(n)));
                }
                else {
                    outputs.push_back({D, mmz->getoutplink(n)});
                }
            }

            // Check
            if (outputs.empty()) {
                errs.report(mattwarning(t("No outputs defined for imported device."), SourcePos(file, 0, 0, 0)));
            }
            // Todo: Other checks required?
        }

        return ok;
    }
    else {
        throw mattruntimeerror(t("Unable to open file for importing."), SourcePos(file, 0, 0, 0));
    }

    delete smz;

    return false;
}


/** Updates the clocks in the device.
 *
 * @author Diesel
 */
void importeddevice::tick() {
    dmz->updateclocks();
}


/** Executes the imported device.
 *
 * @author Diesel
 */
void importeddevice::execute() {
    bool ok = true;

    // Execute devices without incrementing clocks.
    dmz->executedevices(ok, false);

    if (!ok) {
        // Todo: Report position
        throw mattruntimeerror(t("Imported device network failed to stabilise."), SourcePos());
    }
}


/** Checks if the imported device has an input pin.
 *
 * @author Diesel
 */
bool importeddevice::hasInput(name pin) const {
    for (auto it : inputs) {
        if (it->id == pin) {
            return true;
        }
    }
    return false;
}


/** Checks if the imported device has an output pin
 *
 * @author Diesel
 */
bool importeddevice::hasOutput(name mon) const {
    for (auto it : outputs) {
        if (it.first == mon) {
            return true;
        }
    }
    return false;
}


/** Sets the value of an input pin
 *
 * @author Diesel
 */
bool importeddevice::setInput(name pin, asignal value) {
    for (auto it : inputs) {
        if (it->id == pin) {
            it->swstate = value;
            return true;
        }
    }
    return false;
}


/** Retrieves the value of an output pin.
 *
 * @author Diesel
 */
bool importeddevice::getOutput(name mon, asignal& value) {
    for (auto it : outputs) {
        if (it.first == mon) {
            value = it.second->sig;
            return true;
        }
    }
    return false;
}
