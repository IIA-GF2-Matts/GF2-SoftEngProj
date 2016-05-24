#include <iostream>
#include "monitor.h"
#include "names.h"

using namespace std;


/***********************************************************************
 *
 * Sets a monitor on the 'outp' output of device 'dev' by placing an
 * entry in the monitor table. 'ok' is set true if operation succeeds.
 *
 */
void monitor::makemonitor (name dev, name outp, bool& ok, name aliasDevice, name aliasOutp)
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
        newmon.aliasDev = aliasDevice;
        newmon.aliasPin = aliasOutp;

        mtab.push_back(newmon);
      }
    }
  }
}


/***********************************************************************
 *
 * Removes the monitor set the 'outp' output of device 'dev'. 'ok' is
 * set true if operation succeeds.
 *
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


/***********************************************************************
 *
 * Returns number of signals currently monitored.
 *
 */
int monitor::moncount (void)
{
  return (mtab.size());
}


/***********************************************************************
 *
 * Returns number of data points recorded.
 *
 */
int monitor::cycles() const {
  if (mtab.empty()) return 0;
  return mtab[0].sig.size();
}


/***********************************************************************
 *
 * Returns signal level of n'th monitor point.
 *
 */
asignal monitor::getmonsignal (int n)
{
  return getmonsignal(mtab[n]);
}

asignal monitor::getmonsignal(moninfo& mon) {
  if (!mon.op) return floating;

  return mon.op->sig;
}


/***********************************************************************
 *
 * Returns name of n'th monitor.
 *
 */
void monitor::getmonname (int n, name& dev, name& outp)
{
  getmonname(mtab[n], dev, outp);
}

void monitor::getmonname (moninfo& mon, name& dev, name& outp)
{
  // Check for alias
  if (mon.aliasDev != blankname) {
    dev = mon.aliasDev;
    outp = mon.aliasPin;
  }
  else {
    dev = mon.devid;
    outp = mon.op->id;
  }
}


/***********************************************************************
 *
 * Initialises monitor memory in preparation for a new output sequence.
 *
 */
void monitor::resetmonitor (void)
{
  for (auto it : mtab) {
    it.sig.clear();
  }
}


/***********************************************************************
 *
 * Called every clock cycle to record the state of each monitored
 * signal.
 *
 */
void monitor::recordsignals (void)
{
  for (auto m : mtab)
    m.sig.push_back(getmonsignal(m));
}

/***********************************************************************
 *
 * Access recorded signal trace, returns false if invalid monitor
 * or cycle.
 *
 */
bool monitor::getsignaltrace(int m, int c, asignal &s)
{
  if ((m < moncount()) && (c < mtab[m].sig.size())) {
    s = mtab[m].sig[c];
    return true;
  }
  return false;
}

/***********************************************************************
 *
 * Displays state of monitored signals.
 *
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



int monitor::findmonitor (name dev, name pin) {
  // todo: populate findmonitor
  return -1;
}


/***********************************************************************
 *
 * Called to initialise the monitor module.
 * Remember the names of the shared names and network modules.
 *
 */
monitor::monitor (names* names_mod, network* network_mod)
{
  nmz = names_mod;
  netz = network_mod;
  mtab.clear();
}


/***********************************************************************
 *
 * Clears up resources used by monitor
 *
 */
monitor::~monitor() {
}