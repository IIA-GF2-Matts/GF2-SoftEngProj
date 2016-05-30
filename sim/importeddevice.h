
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


/** Data for an imported device
 *
 * @author Diesel
 */
struct importeddevice {
    names* nmz;
    network* netz;
    devices* dmz;
    monitor* mmz;
    errorcollector& errs;

    std::vector<devicerec*> inputs;
    std::vector<std::pair<name, outputrec*>> outputs;

    /** Initialises a new importeddevice structure
     *
     * @param      nm    The name table instance to use.
     * @param      errc  A reference to an errorcollector to report errors to.
     */
    importeddevice(names* nm, errorcollector& errc);

    /** Clears resources allocated by importeddevice
     */
    ~importeddevice();

    /** Reads in a Mattlang circuit definition and parses it to a network.
     *
     * @param[in]  file  The path to the file, relative to the current working
     *                   directory.
     *
     * @return     Returns true if the file was scanned and parsed without error.
     */
    bool scanAndParse(std::string file);

    /** Updates the clocks in the device.
     *  Should be called once per simulation cycle.
     */
    void tick();

    /** Executes the imported device.
     *  This is essentially a simulation cycle for the device, but without
     *  updating clocks.
     */
    void execute();

    /** Checks if the imported device has an input pin.
     *
     * @param[in]  pin    The id in the name table of the pin to check.
     * @return     True if the input exists, False otherwise.
     */
    bool hasInput(name pin) const;

    /** Checks if the imported device has an output pin
     *
     * @param[in]  mon    The id in the name table of the pin to check.
     * @return     True if the output exists, False otherwise.
     */
    bool hasOutput(name mon) const;

    /** Sets the value of an input pin
     *
     * @param      pin    The id in the name table of an input pin.
     * @param      value  The new value of the input.
     * @return     True if the input could be set, false otherwise.
     */
    bool setInput(name pin, asignal value);

    /** Retrieves the value of an output pin.
     *
     * @param[in]  mon    The id in the name table of an output pin.
     * @param      value  Returns the value of the output pin.
     * @return     True if the output could be retrieved. False otherwise.
     */
    bool getOutput(name mon, asignal& value);
};
typedef importeddevice* importedlink;


#endif // GF2_IMPORTEDDEVICE_H
