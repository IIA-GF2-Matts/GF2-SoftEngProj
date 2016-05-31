
#include <iostream>
#include <string>
#include "../com/localestrings.h"
#include "../com/names.h"
#include "importeddevice.h"
#include "devices.h"

using namespace std;

/** Used to print out signal values for debugging in showdevice.
 *
 * @author Gee
 */
void devices::outsig (asignal s)
{
  switch (s) {
    case high:    cout << t("high");    break;
    case low:     cout << t("low");     break;
    case rising:  cout << t("rising");  break;
    case falling: cout << t("falling"); break;
    case floating: cout << t("floating"); break;
    case indet:   cout << t("indeterminate"); break;
  }
}


/** Used to print out device details and signal values
 *  for debugging in executedevices.
 *
 * @author Gee
 */
void devices::showdevice (devlink d)
{
  inplink  i;
  outplink o;
  cout << "   " << t("Device") << ": " << nmz->namestr(d->id);
  cout << "   " << t("Kind") << ": ";
  writedevice (d->kind);
  cout << endl;
  cout << "   " << t("Inputs") << ":" << endl;
  for (i = d->ilist; i != NULL; i = i->next) {
    cout << "      " << nmz->namestr(i->id) << " ";
    outsig (i->connect->sig);
    cout << endl;
  }
  cout << "   " << t("Outputs") << ":";
  for (o = d->olist; o != NULL; o = o->next) {
    cout << "      " << nmz->namestr(o->id) << " ";
    outsig (o->sig);
    cout << endl;
  }
  cout << endl;
}


/** Sets the state of the named switch. 'ok' returns false if switch
 *  not found.
 *
 * @author Gee
 */
void devices::setswitch (name sid, asignal level, bool& ok, SourcePos at)
{
  devlink d;
  d = netz->finddevice (sid);
  ok = (d != NULL);
  if (ok) {
    ok = (d->kind == aswitch);
    if (ok) {
      d->swstate = level;
      d->setAt = at;
    }
  }
}


/** Used to make new imported devices.
 *  Called by makedevice().
 *
 * @author Diesel
 */
void devices::makeimported(name id, std::string fname, errorcollector& errs, SourcePos at)
{
  devlink d;
  netz->adddevice (imported, id, d);
  if (!d) {
    errs.report(mattruntimeerror(t("Error creating imported device in network."), at));
    return;
  }

  d->definedAt = at;

  d->device = new importeddevice(nmz, errs);
  d->device->scanAndParse(fname);

  // Add inputs
  for (auto inp : d->device->inputs) {
    netz->addinput(d, inp->id, inp->definedAt);
  }

  // Add outputs
  for (auto outp : d->device->outputs) {
    netz->addoutput(d, outp.first, outp.second->definedAt);
  }
}


/** Used to make new switch devices.
 *  Called by makedevice.
 *
 * @author Gee
 */
void devices::makeswitch (name id, int setting, bool& ok, SourcePos at)
{
  devlink d;
  ok = (setting <= 1);
  if (ok) {
    netz->adddevice (aswitch, id, d);

    ok = d != NULL;
    if (ok) {
      d->definedAt = at;
      netz->addoutput (d, blankname);
      if (setting == -1) {
        d->swstate = floating;
      }
      else {
        d->swstate = (setting == 0) ? low : high;
      }
    }
  }
}


/** Used to make new switch devices.
 *  Called by makedevice.
 *
 * @author Diesel
 */
void devices::makeselect(name id, int setting, bool& ok, SourcePos at)
{
  devlink d;

  netz->adddevice (aselect, id, d);

  ok = d != NULL;
  if (ok) {
    d->definedAt = at;
    netz->addoutput (d, blankname);
    netz->addinput (d, highpin);
    netz->addinput (d, lowpin);
    netz->addinput (d, swpin);
  }
}


/** Used to make new signal generators.
 *
 * @author Diesel
 */
void devices::makesiggen(name id, std::vector<bool> bits, int period, bool& ok, SourcePos at)
{
  devlink d;

  netz->adddevice (siggen, id, d);
  ok = d != NULL;
  if (ok) {
    d->definedAt = at;
    netz->addoutput (d, blankname);
    d->bitstrpos = 0;
    d->bitstr = bits;
    d->frequency = period;
  }
}


/** Sets the frequency of the named clock.
 *
 * @author Gee
 */
void devices::setclock (name sid, int frequency, bool& ok, SourcePos at)
{
  devlink d;
  d = netz->finddevice (sid);
  ok = (d != NULL);
  if (ok) {
    ok = (d->kind == aclock  || d->kind == siggen);
    if (ok) {
      d->frequency = frequency;
      d->setAt = at;
    }
  }
}


/** Used to make new clock devices.
 *  Called by makedevice.
 *
 * @author Gee
 */
void devices::makeclock (name id, int frequency, SourcePos at)
{
  devlink d;
  netz->adddevice (aclock, id, d);
  netz->addoutput (d, blankname);
  d->definedAt = at;
  d->frequency = frequency;
  d->counter = 0;
}


/** Used to make new AND, NAND, OR, NOR and XOR gates.
 *  Called by makedevice.
 *
 * @author Gee
 */
void devices::makegate (devicekind dkind, name did, int ninputs, bool& ok, SourcePos at)
{
  const int maxinputs = 16;
  devlink d;
  int n;
  namestring iname;
  ok = (ninputs <= maxinputs);
  if (ok) {
    netz->adddevice (dkind, did, d);
    d->definedAt = at;

    netz->addoutput (d, blankname);
    for (n = 1; n <= ninputs; n++) {
      iname = "I";
      if (n < 10) {
        iname += ((char) n) + '0';
      } else {
        iname += ((char) (n / 10)) + '0';
        iname += ((char) (n % 10)) + '0';
      }
      netz->addinput (d, nmz->lookup (iname));
    }
  }
}


/** Used to make new D-type bistable devices.
 *  Inputs: DATA, CLK, SET and CLR.
 *  Outputs: Q, QBAR.
 *  Called by makedevice.
 *
 * @author Gee
 */
void devices::makedtype (name id, bool& ok, SourcePos at)
{
  devlink d;
  netz->adddevice (dtype, id, d);
  netz->addinput (d, datapin);
  netz->addinput (d, clkpin);
  netz->addoutput (d, qpin);
  netz->addoutput (d, qbarpin);
  d->memory = low;
  d->definedAt = at;
}


/** Adds a device to the network of the specified kind and name.
 *
 * @author Gee
 */
void devices::makedevice (devicekind dkind, name did, int variant, bool& ok, SourcePos at)
{
  ok = true;
  switch (dkind) {
    case aswitch:
      makeswitch (did, variant, ok, at);
      break;
    case aclock:
      makeclock (did, variant, at);
      break;
    case andgate:
    case nandgate:
    case orgate:
    case norgate:
      makegate (dkind, did, variant, ok, at);
      break;
    case xorgate:
      makegate (dkind, did, 2, ok, at);
      break;
    case dtype:
      makedtype(did, ok, at);
      break;
    case aselect:
      makeselect (did, variant, ok, at);
      break;
    case imported:
      ok = false;
      // Must call makeimported directly.
      break;
    case siggen: {
      std::vector<bool> b;
      makesiggen(did, b, variant, ok, at);
      // must call makesiggen directly.
      break;
    }
    case baddevice:
    default:
      ok = false;
      break;
  }
}


/** Update signal `sig' in the direction of signal `target'.
 *  Set steadystate to false if this results in a change in sig.
 *
 * @author Gee
 */
void devices::signalupdate (asignal target, asignal& sig)
{
  if (target == indet) {
    if (sig != indet) {
      sig = indet;
      steadystate = false;
    }
    return;
  }

  asignal oldsig;
  oldsig = sig;
  switch (sig) {
    case falling:
    case low:
      sig = (target == high) ? rising : low;
      break;
    case rising:
    case high:
      sig = (target == low) ? falling : high;
      break;
    case floating:
      sig = (target == high) ? rising : falling;
      break;
    case indet:
    default:
      sig = target;
      break;
  }
  if (sig != oldsig)
    steadystate = false;
}


/** Returns the inverse of a signal.
 *
 * @author Gee
 */
asignal devices::inv (asignal s)
{
  if (s == indet) return indet;
  return ((s == high) ? low : high);
}


/** Used to simulate the operation of switch devices.
 *  Called by executedevices.
 *
 * @author Gee
 */
void devices::execswitch (devlink d)
{
  signalupdate (d->swstate, d->olist->sig);
}


/** Used to simulate the operation of select devices.
 *  Called by executedevices.
 *
 * @author Diesel
 */
void devices::execselect (devlink d, bool& ok)
{
  inplink il = netz->findinput(d, swpin);
  ok = il;
  if (ok) {
    inplink conil;
    if (il->connect->sig == high)
      conil = netz->findinput(d, highpin);
    else
      conil = netz->findinput(d, lowpin);

    ok = conil;
    if (ok) {
      signalupdate(conil->connect->sig, d->olist->sig);
    }
  }
}


/** Used to simulate the operation of siggen devices.
 *  Called by executedevices.
 *
 * @author Diesel
 */
void devices::execsiggen(devlink d)
{
  if (d->olist->sig == rising)
    signalupdate (high, d->olist->sig);
  else {
    if (d->olist->sig == falling)
      signalupdate (low, d->olist->sig);
  }
}


/** Used to simulate the operation of AND, OR, NAND and NOR gates.
 *  Called by executedevices.
 *  Meaning of arguments: gate output is 'y' iff all inputs are 'x'
 *
 * @author Gee
 */
void devices::execgate (devlink d, asignal x, asignal y)
{
  asignal newoutp;
  inplink inp = d->ilist;
  outplink outp = d->olist;
  newoutp = y;
  while ((inp != NULL) && (newoutp == y)) {
    if (inp->connect->sig == inv (x))
      newoutp = inv (y);
    inp = inp->next;
  }
  signalupdate (newoutp, outp->sig);
}


/** Used to simulate the operation of exclusive or gates.
 *  Called by executedevices.
 *
 * @author Gee
 */
void devices::execxorgate(devlink d)
{
  asignal newoutp;
  if (d->ilist->connect->sig == d->ilist->next->connect->sig)
    newoutp = low;
  else
    newoutp = high;
  signalupdate (newoutp, d->olist->sig);
}


/** Used to simulate the operation of D-type bistables.
 *
 *  Called by executedevices. The signal on the data input
 *  immediately BEFORE the clock edge is transferred to the
 *  Q output. We are effectively assuming a negligible but
 *  nonzero setup time, and a zero hold time.
 *
 * @author Gee
 */
void devices::execdtype (devlink d)
{
  asignal datainput, clkinput, setinput, clrinput;
  inplink i;
  outplink qout, qbarout;
  i = netz->findinput (d, datapin); datainput = i->connect->sig;
  i = netz->findinput (d, clkpin);  clkinput  = i->connect->sig;

  // SET and CLEAR default to low if not specified.
  i = netz->findinput (d, clrpin);
  if (!i)
    clrinput = low;
  else
    clrinput  = i->connect->sig;

  i = netz->findinput (d, setpin);
  if (!i)
    setinput = low;
  else
    setinput  = i->connect->sig;


  qout = netz->findoutput (d, qpin);
  qbarout = netz->findoutput (d, qbarpin);


  if ((clkinput == rising) && ((datainput == falling) || (datainput == rising)))
    d->memory = indet;
  if ((clkinput == rising) && (datainput == high))
    d->memory = high;
  if ((clkinput == rising) && (datainput == low))
    d->memory = low;
  if (setinput == high)
    d->memory = high;
  if (clrinput == high)
    d->memory = low;
  signalupdate (d->memory, qout->sig);
  signalupdate (inv (d->memory), qbarout->sig);
}


/** Used to simulate the operation of clock devices.
 *  Called by executedevices.
 *
 * @author Gee
 */
void devices::execclock(devlink d)
{
  if (d->olist->sig == rising)
    signalupdate (high, d->olist->sig);
  else {
    if (d->olist->sig == falling)
      signalupdate (low, d->olist->sig);
  }
}


/** Used to simulate the operation of an imported network.
 *  Called by executedevices.
 *
 * @author Diesel
 */
void devices::execimported(devlink d) {
  // Update input pins
  for (inplink il = d->ilist; il; il = il->next) {
    d->device->setInput(il->id, il->connect->sig);
  }

  d->device->execute();

  // Update output pins
  asignal s;
  for (outplink ol = d->olist; ol; ol = ol->next) {
    d->device->getOutput(ol->id, s);
    signalupdate(s, ol->sig);
  }
}


/** Increment the counters in the clock devices and initiate changes
 *  in their outputs when the end of their period is reached.
 *  Called by executedevices.
 *
 * @author Gee
 */
void devices::updateclocks (void)
{
  devlink d;
  for (d = netz->devicelist (); d != NULL; d = d->next) {
    if (d->kind == aclock) {
      if (d->counter == d->frequency) {
        d->counter = 0;
        if (d->olist->sig == high)
          d->olist->sig = falling;
        else
          d->olist->sig = rising;
      }
      (d->counter)++;
    }
    else if (d->kind == imported) {
      d->device->tick();
    }
    else if (d->kind == siggen) {
      if (d->frequency == 0 || d->counter == d->frequency) {
        d->counter = 0;
        if (++(d->bitstrpos) >= d->bitstr.size()) {
          d->bitstrpos = 0;
        }

        asignal newSig = d->bitstr[d->bitstrpos] ? high : low;
        if (newSig != d->olist->sig) {
          signalupdate(newSig, d->olist->sig);
        }

      }
      (d->counter)++;
    }
  }
}


/** Executes all devices in the network to simulate one complete clock
 *  cycle.
 *
 * @author Gee
 */
void devices::executedevices (bool& ok, bool tick)
{
  const int maxmachinecycles = 20;
  devlink d;
  int machinecycle;
  if (debugging)
    cout << t("Start of execution cycle") << endl;
  if (tick)
    updateclocks ();
  machinecycle = 0;
  do {
    machinecycle++;
    if (debugging)
      cout << t("machine cycle") << " # " << machinecycle << endl;
    steadystate = true;
    for (d = netz->devicelist (); d != NULL; d = d->next) {
      switch (d->kind) {
        case aswitch:  execswitch (d);           break;
        case aclock:   execclock (d);            break;
        case orgate:   execgate (d, low, low);   break;
        case norgate:  execgate (d, low, high);  break;
        case andgate:  execgate (d, high, high); break;
        case nandgate: execgate (d, high, low);  break;
        case xorgate:  execxorgate (d);          break;
        case dtype:    execdtype (d);            break;
        case aselect:  execselect(d, ok);        break;
        case imported: execimported(d);          break;
        case siggen:   execsiggen(d);            break;
        default:       ok = false;               break;
      }
      if (debugging)
        showdevice (d);
    }
  } while ((! steadystate) && (machinecycle < maxmachinecycles));
  if (debugging)
    cout << t("End of execution cycle") << endl;
  ok = steadystate;
}


/** Resets devices in the network
 *
 * @author Diesel
 */
void devices::resetdevices() {
  for (devlink d = netz->devicelist(); d; d = d->next) {
    switch (d->kind) {
      case aclock:
        d->counter = 0;
      case orgate:
      case norgate:
      case andgate:
      case nandgate:
        d->olist->sig = low;
        break;
      case dtype:
        d->memory = low;
        break;
      case imported:
        d->device->dmz->resetdevices();
        break;
      case aselect:
        break;
      case siggen:
        d->counter = 0;
        d->bitstrpos = 0;
        d->olist->sig = d->bitstr[0] ? high : low;
        break;
      default:
        break;
    }
  }
}


/** Returns the name id of the given devicekind
 *
 * @author Diesel
 */
name devices::getname (devicekind k) const
{
  return dtab[k];
}


/** Prints out the given device kind. [deprecated - use std::cout << *getname()]
 *  Used by showdevice.
 *
 * @author Gee
 */
void devices::writedevice (devicekind k)
{
  std::cout << nmz->namestr(dtab[k]);
}


/** Returns the kind of device corresponding to the given name.
 *
 * @author Gee
 */
devicekind devices::devkind (name id)
{
  devicekind d;
  d = aswitch;
  while ((d != baddevice) && (dtab[d] != id))
    d = static_cast<devicekind>(d + 1);
  return (d);
}


/** Set the state of the internal debugging flag.
 *
 * @author Gee
 */
void devices::debug (bool on)
{
  debugging = on;
}


/** Initialises the devices object
 *  Registers the names of all the possible devices.
 *
 * @author Gee
 */
devices::devices (names* names_mod, network* net_mod)
{
  nmz = names_mod;
  netz = net_mod;
  dtab[aswitch]   =  nmz->lookup("SWITCH");
  dtab[aclock]    =  nmz->lookup("CLOCK");
  dtab[andgate]   =  nmz->lookup("AND");
  dtab[nandgate]  =  nmz->lookup("NAND");
  dtab[orgate]    =  nmz->lookup("OR");
  dtab[norgate]   =  nmz->lookup("NOR");
  dtab[xorgate]   =  nmz->lookup("XOR");
  dtab[dtype]     =  nmz->lookup("DTYPE");
  dtab[baddevice] =  blankname;
  debugging = false;
  datapin = nmz->lookup("DATA");
  clkpin  = nmz->lookup("CLK");
  setpin  = nmz->lookup("SET");
  clrpin  = nmz->lookup("CLEAR");
  qpin    = nmz->lookup("Q");
  qbarpin = nmz->lookup("QBAR");
  highpin = nmz->lookup("HIGH");
  lowpin = nmz->lookup("LOW");
  swpin = nmz->lookup("SW");

  initvalnm = nmz->lookup("InitialValue");
  periodnm = nmz->lookup("Period");
  signm = nmz->lookup("SIG");

  // Note: Doesn't match name requirement for user.
  // so cannot be overwritten/reused in user code.
  zero    = nmz->lookup("0");
  one     = nmz->lookup("1");

  // Todo: Actually check ok.
  bool ok;
  makedevice(aswitch, zero, 0, ok);
  makedevice(aswitch, one, 1, ok);
}


/** Clears up resources allocated by devices
 */
devices::~devices() {
}
