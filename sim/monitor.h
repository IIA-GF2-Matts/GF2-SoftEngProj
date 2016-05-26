
#ifndef GF2_MONITOR_H
#define GF2_MONITOR_H

#include <vector>

#include "../com/names.h"
#include "../com/sourcepos.h"
#include "network.h"
#include "devices.h"

const int maxmonitors = 1000;      /* max number of monitor points */
const int maxcycles = 100000;        /* max number of cycles per run */


/** The data associated with a monitor
 *
 * @author Gee, Diesel
 */
struct moninfo {
  name devid;
  outplink op;
  SourcePos definedAt;

  name aliasDev;
  name aliasPin;
  std::vector<asignal> sig;
};
typedef std::vector<moninfo> monitortable;


/** Stores information about monitor points in a network.
 *
 * @author Gee, Diesel
 */
class monitor {
  names*   nmz;     // version of names class to use.
  network* netz;    // version of the network class to use.

  monitortable mtab;                 // table of monitored signals

  SourcePos& getdefinedpos(moninfo& m);
  asignal getmonsignal (const moninfo& mon) const;
  void getmonname (moninfo& mon, name& dev, name& outp, bool alias = true);

 public:

  /** Returns the output link associated with this monitor
   *
   * @param[in]  n     The index of the monitor.
   * @return     The outplink corresponding to the signal monitored by monitor n.
   */
  outplink getoutplink(int n);

  /** Gets the definedAt property of monitor n
   *
   * @param[in]  n     The index of the monitor.
   * @return     The source position the monitor was defined at.
   */
  SourcePos& getdefinedpos(int n);

  /** Sets a monitor in the network by placing an entry in the monitor table.
   *
   * @param[in]  dev          The name id of the device
   * @param[in]  outp         The name if of the output pin of the device
   * @param      ok           Returns false if errors occured making the monitor.
   * @param[in]  aliasDevice  The name id of the monitors alias device.
   * @param[in]  aliasOutp    The name id of the monitors alias pin
   * @param[in]  p            The source position the monitor is being defined from.
   */
  void makemonitor (name dev, name outp, bool& ok
          , name aliasDevice = blankname, name aliasOutp = blankname, SourcePos p = SourcePos());

  /** Removes a monitor point from the netowrk
   *
   * @param[in]  dev          The name id of the device being monitored
   * @param[in]  outp         The name if of the output pin of the device
   * @param      ok           Returns false if errors occured removing the monitor.
   */
  void remmonitor (name dev, name outp, bool& ok);

  /** Returns number of signals currently monitored.
   *
   * @return     The number of signals currently monitored.
   */
  int moncount (void) const;

  /** Returns the number of simulation cycles in the monitor history.
   *
   * @return     The number of simulation cycles recorded.
   */
  int cycles() const;

  /** Returns signal level of the n'th monitor point.
   *
   * @param[in]  n     The index of the monitor
   * @return     The current signal level of the monitor.
   */
  asignal getmonsignal (int n) const;

  /** Access recorded signal trace
   *
   * @param[in]  m     The index of the monitor
   * @param[in]  c     The cycle number to check
   * @param      s     Returns the signal level.
   * @return     True if successful, false otherwise.
   */
  bool getsignaltrace(int m, int c, asignal &s);

  /** Returns name of n'th monitor
   *
   * @param[in]  n      The index of the monitor
   * @param      dev    Returns the device name of the monitor point
   * @param      outp   Returns the output pin name of the monitor point
   * @param[in]  alias  If true (default), dev and outp are set to the alias
   *                    names if they exist.
   */
  void getmonname (int n, name& dev, name& outp, bool alias = true);

  /** Resets monitor signals and clears the history.
   */
  void resetmonitor (void);

  /** Records the state of all monitor points to the history
   */
  void recordsignals (void);

  /** Displays the state of monitored signals
   *  Used by the CLI.
   */
  void displaysignals (void);

  /** Gets the index of the monitor measuring the given signal
   *
   * @param[in]  dev        The name id of the device
   * @param[in]  pin        The name if of the output pin
   * @param[in]  inclAlias  If true, the function will also check for aliases
   *                        matching dev and pin.
   *
   * @return     The index of the monitor, or -1 if no monitor matched the inputs.
   */
  int findmonitor (name dev, name pin, bool inclAlias = false);

  /** Initialises the monitors class
   *
   * @param      names_mod    The name table instance to use
   * @param      network_mod  The network instance to use.
   */
  monitor (names* names_mod, network* network_mod);

  /** Clears resources allocated by monitor
   */
  ~monitor();
};

#endif /* GF2_MONITOR_H */
