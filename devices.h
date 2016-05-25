#ifndef devices_h
#define devices_h

#include <string>

#include "names.h"
#include "network.h"
#include "sourcepos.h"
#include "errorhandler.h"

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
#ifdef EXPERIMENTAL
  void makeimported(name id, std::string fname, errorcollector& errs, SourcePos at = SourcePos());
  void execimported(devlink d);
#endif

  name        clkpin, datapin, setpin;
  name        clrpin, qpin, qbarpin;     /* Input and Output Pin names */
  name        initvalnm, periodnm;
  name        zero, one;

  void makedevice (devicekind dkind, name did, int variant, bool& ok, SourcePos at = SourcePos());
    /* Adds a device to the network of the specified kind and name.  The   */
    /* variant is used with such things as gates where it specifies the    */
    /* number of inputs. 'ok' returns true if operation succeeds.          */

  void setswitch (name sid, asignal level, bool& ok, SourcePos at = SourcePos());
    /* Sets the state of the named switch. 'ok' returns false if switch    */
    /* not found.                                                          */

  void setclock (name sid, int frequency, bool& ok, SourcePos at = SourcePos());
    /* Sets the frequency of the named clock. 'ok' returns false if clock  */
    /* not found.                                                          */

  void updateclocks (void);
    /* Updates clocks in the network, represents one "tick" of the simulation */

  void executedevices (bool& ok, bool tick = true);
    /* Executes all devices in the network to simulate one complete clock  */
    /* cycle. 'ok' is returned false if network fails to stabilise (i.e.   */
    /* it is oscillating). */

  devicekind devkind (name id);
    /* Returns the kind of device corresponding to the given name.         */
    /* 'baddevice' is returned if the name is not a legal device.          */

  name getname(devicekind k) const;
    /* Returns the name id of the given devicekind                         */

  void writedevice (devicekind k);
    /* Prints out the given device kind.                                   */

  void debug (bool on);
    /* Used to set debugging switch.                                       */

  devices (names* names_mod, network* net_mod);
    /* Called to initialise module.                                        */

  ~devices();
};

#endif /* devices_h */
