
#include <iostream>
#include "../com/names.h"
#include "monitor.h"

using namespace std;


/** Returns the output link associated with this monitor
 *
 * @author Diesel
 */
outplink monitor::getoutplink(int n) {
  return mtab[n].op;
}


/** Gets the definedAt property of monitor n
 *
 * @author Diesel
 */
SourcePos& monitor::getdefinedpos(int n) {
  return getdefinedpos(mtab.at(n));
}

SourcePos& monitor::getdefinedpos(moninfo& m) {
  return m.definedAt;
}


/** Sets a monitor in the network by placing an entry in the monitor table.
 *
 * @author Gee
 */
void monitor::makemonitor (name dev, name outp, bool& ok, name aliasDevice, name aliasOutp, SourcePos p)
{
  devlink d;
  outplink o;
  ok = (mtab.size() < maxmonitors);
  if (ok) {
    d = netz->finddevice (dev);
    ok = (d != NULL);
    if (ok) {
      o = netz->findoutput (d, outp);
      ok = (o != NULL);
      if (ok) {
        moninfo newmon;
        newmon.devid = dev;
        newmon.op = o;
        newmon.definedAt = p;
        newmon.aliasDev = aliasDevice;
        newmon.aliasPin = aliasOutp;
        newmon.sig.reserve(50);

        mtab.push_back(newmon);
      }
    }
  }
}


/** Removes a monitor point from the netowrk
 *
 * @author Gee
 */
void monitor::remmonitor (name dev, name outp, bool& ok)
{
  int i, j;
  bool found;
  ok = !mtab.empty();
  if (ok) {
    found = false;
    for (i = 0; ((i < mtab.size()) && (! found)); i++)
      found = ((mtab[i].devid == dev) &&
         (mtab[i].op->id == outp));
    ok = found;
    if (found) { // Remove the monitor
      mtab.erase(mtab.begin() + i);
    }
  }
}


/** Returns number of signals currently monitored.
 *
 * @author Gee
 */
int monitor::moncount (void) const
{
  return (mtab.size());
}


/** Returns the number of simulation cycles in the monitor history.
 *
 * @author Diesel
 */
int monitor::cycles() const {
  if (mtab.empty()) return 0;
  return mtab[0].sig.size();
}


/** Returns signal level of the n'th monitor point.
 *
 * @author Gee
 */
asignal monitor::getmonsignal (int n) const {
  return getmonsignal(mtab[n]);
}

asignal monitor::getmonsignal(const moninfo& mon) const {
  if (!mon.op) return floating;

  return mon.op->sig;
}


/** Returns name of n'th monitor
 *
 * @author Gee
 */
void monitor::getmonname (int n, name& dev, name& outp, bool alias)
{
  getmonname(mtab[n], dev, outp, alias);
}

void monitor::getmonname (moninfo& mon, name& dev, name& outp, bool alias)
{
  // Check for alias
  if (alias && mon.aliasDev != blankname) {
    dev = mon.aliasDev;
    outp = mon.aliasPin;
  }
  else {
    dev = mon.devid;
    outp = mon.op->id;
  }
}


/** Resets monitor signals and clears the history.
 *
 * @author Diesel
 */
void monitor::resetmonitor (void)
{
  for (auto& it : mtab) {
    it.sig.clear();
  }
}


/** Records the state of all monitor points to the history
 *
 * @author Gee, Diesel
 */
void monitor::recordsignals (void)
{
  for (auto& m : mtab) {
    m.sig.push_back(getmonsignal(m));
    // limit size to maxcycles by removing first values
    if (m.sig.size() > maxcycles)
      m.sig.erase(m.sig.begin());
  }
}


/** Access recorded signal trace
 *
 * @author Gee, Diesel
 */
bool monitor::getsignaltrace(int m, int c, asignal &s)
{
  if ((m < moncount()) && (c < mtab[m].sig.size())) {
    s = mtab[m].sig[c];
    return true;
  }
  return false;
}


/** Displays the state of monitored signals
 *
 * @author Gee
 */
void monitor::displaysignals (void)
{
  const int margin = 20;
  int n, i;
  name dev, outp;
  int namesize;
  for (auto mon : mtab) {

    // Print monitor name
    getmonname(mon, dev, outp);
    namesize = nmz->namelength(dev);
    cout << nmz->namestr(dev);
    if (outp != blankname) {
      cout << "." << nmz->namestr(outp);
      namesize = namesize + nmz->namelength(outp) + 1;
    }

    if ((margin - namesize) > 0) {
      for (i = 0; i < (margin - namesize - 1); i++)
        cout << " ";
      cout << ":";
    }

    for (auto i : mon.sig) {
      switch (i) {
        case high:    cout << "-"; break;
        case low:     cout << "_"; break;
        case rising:  cout << "/"; break;
        case falling: cout << "\\"; break;
        case floating: cout << "?"; break;
      }
    }
    cout << endl;
  }
}


/** Gets the index of the monitor measuring the given signal
 *
 * @author Diesel
 */
int monitor::findmonitor (name dev, name pin, bool inclAlias) {
  for (int n = 0; n < mtab.size(); n++) {
    if (mtab[n].devid == dev && (!mtab[n].op || mtab[n].op->id == pin))
      return n;

    if (inclAlias
        && (mtab[n].aliasDev == dev && mtab[n].aliasPin == pin)) {
      return n;
    }
  }
  return -1;
}


/** Initialises the monitors class
 *
 * @author Gee
 */
monitor::monitor (names* names_mod, network* network_mod)
{
  nmz = names_mod;
  netz = network_mod;
  mtab.clear();
}


/** Clears resources allocated by monitor
 *
 * @author Diesel
 */
monitor::~monitor() {
}
