#ifndef network_h
#define network_h

#include <vector>
#include "names.h"
#include "sourcepos.h"
#include "errorhandler.h"

/* Network specification */

typedef enum {falling, low, rising, high, floating} asignal;
typedef enum {aswitch, aclock, andgate, nandgate, orgate,
	      norgate, xorgate, dtype, baddevice} devicekind;

struct outputsignal {
  name devicename;
  name pinname;
};

struct outputrec {
  name       id;
  SourcePos  definedAt;
  asignal    sig;
  outputrec* next;
};
typedef outputrec* outplink;
struct inputrec {
  name      id;
  SourcePos  definedAt;
  outplink  connect;
  inputrec* next;
};
typedef inputrec* inplink;
struct devicerec {
  name id;
  SourcePos  definedAt;
  inplink ilist;
  outplink olist;
  devicerec* next;
  devicekind kind;
  /* the next elements are only used by some of the device kinds */
  asignal swstate;      // used when kind == aswitch
  int frequency;        // used when kind == aclock
  int counter;          // used when kind == aclock
  asignal memory;       // used when kind == dtype
  SourcePos setAt;
};
typedef devicerec* devlink;

class network {
  names* nmz;  // the instatiation of the names class that we are going to use.

 public:
  devlink devicelist (void);
    /* Returns list of devices                                             */

  devlink findoutputdevice(const outplink ol);
    /* Finds the device that creates the output link ol                    */

  std::vector<devlink> findswitches();
  // find all the user defined switches in the network

  std::vector<outputsignal> findoutputsignals();
  // find all the output signals 

  devlink finddevice (name id);
   /* Returns link to device with specified name. Returns NULL if not      */
   /* found.                                                               */

  inplink findinput (devlink dev, name id);
    /* Returns link to input of device pointed to by dev with specified    */
    /* name.  Returns NULL if not found.                                    */

  outplink findoutput (devlink dev, name id);
    /* Returns link to output of device pointed to by dev with specified   */
    /* name.  Returns NULL if not found.                                    */

  void adddevice (devicekind dkind, name did, devlink& dev, SourcePos at = SourcePos());
    /* Adds a device to the device list with given name and returns a link */
    /* to it via 'dev'.                                                    */

  void addinput (devlink dev, name iid, SourcePos at = SourcePos());
    /* Adds an input to the device pointed to by 'dev' with the specified  */
    /* name.                                                               */

  void addoutput (devlink dev, name oid, SourcePos at = SourcePos());
    /* Adds an output to the device pointed to by 'dev' with the specified */
    /* name.                                                               */

  void makeconnection (name idev, name inp, name odev, name outp, bool& ok);
    /* Makes a connection between the 'inp' input of device 'idev' and the */
    /* 'outp' output of device 'odev'. 'ok' is set true if operation       */
    /* succeeds.                                                           */

  void checknetwork (errorcollector& col);
    /* Checks that all inputs are connected to an output.                  */

  network (names* names_mod);
  /* Called on system initialisation.                                      */

  ~network();
    /* Frees memory allocated by network                                   */

 private:
  devlink devs;          // the list of devices
  devlink lastdev;       // last device in list of devices

};

#endif /* network_h */
