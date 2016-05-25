#include <iostream>
#include <sstream>
#include "network.h"
#include "sourcepos.h"
#include "errorhandler.h"
#include "importeddevice.h"

using namespace std;

/***********************************************************************
 *
 * Returns list of devices.
 *
 */
devlink network::devicelist (void)
{
  return devs;
}

/***********************************************************************
 *
 * Finds the device that creates the output link ol.
 *
 */
devlink network::findoutputdevice(const outplink ol)
{
  devlink d = devs;
  outplink o;

  while (d != NULL) {
    o = d->olist;

    while (o != NULL) {
      if (o == ol) return d;
      o = o->next;
    }

    d = d->next;
  }

  return NULL;
}

// find all the user defined switches in the network
std::vector<devlink> network::findswitches() {
  std::vector<devlink> ret;

  devlink d = devs;

  while (d != NULL) {
    // ensures switches returned aren't special logic line switches
    // Todo: it might be cleaner to return all switches, and the
    // caller filter the results, since the zero and one handles
    // in devices could then be checked directly
    if (d->kind == aswitch && nmz->namestr(d->id) != "0" && nmz->namestr(d->id) != "1")
      ret.push_back(d);
    d = d->next;
  }
  return ret;
}


// find all the output signals
std::vector<outputsignal> network::findoutputsignals(){
  std::vector<outputsignal> ret;
  devlink d = devs;
  outplink o;

  // loop through all devices
  while (d != NULL) {
    // get device's outputs
    o = d->olist;
    while (o != NULL) {
      // add them to the collection
      ret.push_back({d->id, o->id});
      o = o->next;
    }
    d = d->next;
  }
  return ret;
}


/***********************************************************************
 *
 * Returns link to device with specified name. Returns NULL if not
 * found.
 *
 */
devlink network::finddevice (name id)
{
  devlink d;
  bool found;
  found = false;
  d = devs;
  while ((d != NULL) && (! found)) {
    found = (d->id == id);
    if (! found)
      d = d->next;
  }
  return d;
}


/***********************************************************************
 *
 * Returns link to input of device pointed to by dev with specified
 * name.  Returns NULL if not found.
 *
 */
inplink network::findinput (devlink dev, name id)
{
  inplink i;
  bool found;
  i = dev->ilist;
  found = false;
  while ((i != NULL) && (! found)) {
    found = (i->id == id);
    if (! found)
        i = i->next;
  }
  return i;
}


/***********************************************************************
 *
 * Returns link to output of device pointed to by dev with specified
 * name.  Returns NULL if not found.
 *
 */
outplink network::findoutput (devlink dev, name id)
{
  outplink o;
  bool found;
  o = dev->olist;
  found = false;
  while ((o != NULL) && (! found)) {
    found = (o->id == id);
    if (! found)
        o = o->next;
  }
  return o;
}


/***********************************************************************
 *
 * Adds a device to the device list with given name and returns a link
 * to it via 'dev'.
 *
 */
void network::adddevice (devicekind dkind, name did, devlink& dev, SourcePos at)
{
  dev = new devicerec;
  dev->id = did;
  dev->definedAt = at;
  dev->kind = dkind;
  dev->ilist = NULL;
  dev->olist = NULL;
  if (dkind != aclock) {        // device goes at head of list
    if (lastdev == NULL)
        lastdev = dev;
    dev->next = devs;
    devs = dev;
  } else {                      // aclock devices must go last
    dev->next = NULL;
    if (lastdev == NULL) {
      devs = dev;
      lastdev = dev;
    } else {
      lastdev->next = dev;
      lastdev = dev;
    }
  }
}


/***********************************************************************
 *
 * Adds an input to the device pointed to by 'dev' with the specified
 * name.
 *
 */
void network::addinput (devlink dev, name iid, SourcePos at)
{
  inplink i = new inputrec;
  i->id = iid;
  i->definedAt = at;
  i->connect = NULL;
  i->next = dev->ilist;
  dev->ilist = i;
}


/***********************************************************************
 *
 * Adds an output to the device pointed to by 'dev' with the specified
 * name.
 *
 */
void network::addoutput (devlink dev, name oid, SourcePos at)
{
  outplink o = new outputrec;
  o->id = oid;
  o->definedAt = at;
  o->sig = low;
  o->next = dev->olist;
  dev->olist = o;
}


/***********************************************************************
 *
 * Makes a connection between the 'inp' input of device 'idev' and the
 * 'outp' output of device 'odev'. 'ok' is set true if operation
 * succeeds.
 *
 */
void network::makeconnection (name idev, name inp, name odev, name outp, bool& ok)
{
  devlink din, dout;
  outplink o;
  inplink i;
  din = finddevice (idev);
  dout = finddevice (odev);
  ok = ((din != NULL) && (dout != NULL));
  if (ok) {
    o = findoutput (dout, outp);
    i = findinput (din, inp);
    ok = ((o != NULL) && (i != NULL));
    if (ok)
      i->connect = o;
  }
}


/***********************************************************************
 *
 * Checks that all inputs are connected to an output.
 *
 */
void network::checknetwork (errorcollector& col)
{
  devlink d;
  inplink i;

  for (d = devs; d != NULL; d = d->next) {
    if (d->kind == aswitch) {
      if (d->swstate == floating) {
        std::ostringstream oss;
        oss << "Input " << nmz->namestr(d->id) << ".InitialValue "
            << "has not been assigned a value.";
        col.report(mattsemanticerror(oss.str(), d->definedAt));
      }
    }
    else if (d->kind == aclock) {
      if (d->frequency == 0) {
        std::ostringstream oss;
        oss << "Input " << nmz->namestr(d->id) << ".Period "
            << "has not been assigned a value.";
        col.report(mattsemanticerror(oss.str(), d->definedAt));
      }
    }
    else {
      for (i = d->ilist; i != NULL; i = i->next) {
        if (i->connect == NULL) {
          std::ostringstream oss;
          oss << "Input " << nmz->namestr(d->id);
          if (i->id != blankname)
            oss << "." << nmz->namestr(i->id);
          oss << " has not been assigned a value.";

          col.report(mattsemanticerror(oss.str(), d->definedAt));
        }
      }
    }
  }
}


/***********************************************************************
 *
 * The constructor for the network module.
 * Remember the version of the names module that is used here and
 * shared with other modules.
 * Initialise the list of devices.
 *
 */
network::network (names* names_mod)
{
  nmz = names_mod;
  devs = NULL;
  lastdev = NULL;
}


/***********************************************************************
 *
 * The network destructor.
 * Frees memory allocated by the network
 * This frees all devicerec, inputrec and outputrec in the lists
 * Recursive methods are defined locally to delete the lists.
 *
 */

void delInpList(inputrec* inpr) {
  if (!inpr) return;

  delInpList(inpr->next);
  delete inpr;
}
void delOutpList(outputrec* outpr) {
  if (!outpr) return;

  delOutpList(outpr->next);
  delete outpr;
}

void delDevList(devicerec* dr) {
  if (!dr) return;

  delDevList(dr->next);
  dr->next = NULL;

#ifdef EXPERIMENTAL
  if (dr->kind == imported) {
    delete dr->device;
  }
#endif

  delInpList(dr->ilist);
  delOutpList(dr->olist);

  delete dr;
}

network::~network ()
{
  delDevList(devs);
}


