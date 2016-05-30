
#ifndef GF2_DEVICES_H
#define GF2_DEVICES_H

#include <string>
#include <vector>

#include "../com/names.h"
#include "../com/sourcepos.h"
#include "../com/errorhandler.h"
#include "network.h"


/** Devices Class
 *  Used to create, manipulate and execute devices within a network.
 *
 *  @author Gee
 */
class devices{
  names* nmz;      // the version of the names module we use.
  network* netz;   // the version of the network module we use.

  typedef name devicetable[baddevice + 1];
  devicetable dtab;
  bool        steadystate;
  bool        debugging;

  void showdevice (devlink d);
  void makeswitch (name id, int setting, bool& ok, SourcePos at = SourcePos());
  void makeclock (name id, int frequency, SourcePos at = SourcePos());
  void makegate (devicekind dkind, name did, int ninputs, bool& ok, SourcePos at = SourcePos());
  void makedtype (name id, bool& ok, SourcePos at = SourcePos());
  void signalupdate (asignal target, asignal& sig);
  asignal inv (asignal s);
  void execswitch (devlink d);
  void execgate (devlink d, asignal x, asignal y);
  void execxorgate(devlink d);
  void execdtype (devlink d);
  void execclock(devlink d);
  void outsig (asignal s);

public:
  // Todo: Do these need to be public?
  void makeimported(name id, std::string fname, errorcollector& errs, SourcePos at = SourcePos());
  void execimported(devlink d);
  void makeselect (name id, int setting, bool& ok, SourcePos at = SourcePos());
  void execselect(devlink d, bool& ok);
  void makesiggen(name id, std::vector<bool> bits, int period, bool& ok, SourcePos at = SourcePos());
  void execsiggen(devlink d);

  name        clkpin, datapin, setpin;
  name        clrpin, qpin, qbarpin;     /* Input and Output Pin names */
  name        initvalnm, periodnm, signm;
  name        zero, one;
  name        highpin, lowpin, swpin;

  /** Adds a device to the network of the specified kind and name.
   *
   * @param[in]  dkind    The type of the device.
   * @param[in]  did      The id in the name table of the device.
   * @param[in]  variant  The variant of device. The meaning depends on the
   *                      type of device being created:
   * @param      ok       Returns false if an error occured.
   * @param[in]  at       The source position the device is being defined from
   *                      for later error handling.
   */
  void makedevice (devicekind dkind, name did, int variant, bool& ok, SourcePos at = SourcePos());

  /** Sets the state of the named switch.
   *
   * @param[in]  sid    The id in the name table of the switch to be set
   * @param[in]  level  The new output signal of the switch.
   * @param      ok     Returns false if an error occured, i.e. the switch was
   *                    not found.
   * @param[in]  at     The source position the switch is being set from.
   */
  void setswitch (name sid, asignal level, bool& ok, SourcePos at = SourcePos());

  /** Sets the frequency of the named clock.
   *
   * @param[in]  sid        The id in the name table of the clock to be set
   * @param[in]  frequency  The new period of the clock.
   * @param      ok         Returns false if an error occured, i.e. the clock was
   *                        not found.
   * @param[in]  at         The source position the clock is being set from.
   */
  void setclock (name sid, int frequency, bool& ok, SourcePos at = SourcePos());

  /** Updates clocks in the network, represents one "tick" of the
   *  simulation.
   */
  void updateclocks (void);

  /** Executes all devices in the network to simulate one complete clock
   *  cycle.
   *
   * @param      ok    Returns false if an error occured, i.e. the network
   *                   failed to stabilise.
   * @param[in]  tick  If false, then clocks aren't updated at the start of
   *                   execution.
   */
  void executedevices (bool& ok, bool tick = true);

  /** Resets the outputs of devices in the network to zero
   */
  void resetdevices();

  /** Returns the kind of device corresponding to the given name.
   *
   * @param[in]  id    The identifier in the name table to search for
   * @return     The devicekind corresponding to the id, or 'baddevice' if the
   *             name is not a legal device.
   */
  devicekind devkind (name id);

  /** Returns the name id of the given devicekind
   *
   * @param[in]  k     The device type.
   * @return     The name id in the name table, or blankname if the devicekind
   *             is invalid.
   */
  name getname(devicekind k) const;

  /** Prints out the given device kind. [deprecated - use std::cout << *getname()]
   *
   * @param[in]  k     The device type value.
   */
  void writedevice (devicekind k);

  /** Set the state of the internal debugging flag.
   *
   * @param on If true then debugging is enabled, otherwise debugging is disabled.
   */
  void debug (bool on);

  /** Initialises the devices object
   *
   * @param      names_mod  The names class instance to use.
   * @param      net_mod    The network class instance to use.
   */
  devices (names* names_mod, network* net_mod);

  /** Clears up resources allocated by devices.
   */
  ~devices();
};

#endif /* GF2_DEVICES_H */
