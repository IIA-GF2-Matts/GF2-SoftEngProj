#ifndef network_h
#define network_h

#include <vector>
#include "../com/names.h"
#include "../com/sourcepos.h"
#include "../com/errorhandler.h"

struct importeddevice;

/* Network specification */

typedef enum {falling, low, rising, high, floating} asignal;
typedef enum {aswitch, aclock, andgate, nandgate, orgate,
	      norgate, xorgate, dtype, jk, siggen, aselect, imported, baddevice} devicekind;

/** Stores a signals deivce . outputpin pair
 *
 * @author Judge
 */
struct outputsignal {
  name devicename;
  name pinname;
};


/** List of output pins for a device
 *
 * @author Gee
 */
struct outputrec {
  name       id;
  SourcePos  definedAt;
  asignal    sig;
  outputrec* next;
};
typedef outputrec* outplink;


/** List of input pins for a device
 *
 * @author Gee
 */
struct inputrec {
  name      id;
  SourcePos  definedAt;
  outplink  connect;
  inputrec* next;
};
typedef inputrec* inplink;


/** List of devices and their properties in a network
 *
 * @author Gee
 */
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
#ifdef EXPERIMENTAL
  importeddevice* device;  // used when kind == imported
#endif

  SourcePos setAt;
};
typedef devicerec* devlink;


/** Stores a list of devices and provides methods for manipulating it.
 *
 * @author Gee
 */
class network {
  names* nmz;  // the instatiation of the names class that we are going to use.

 public:
  /** Returns list of devices
   *
   * @return     The pointer to the first device in the list.
   */
  devlink devicelist (void);

  /** Finds the device that creates the output link ol
   *
   * @param[in]  ol    The outplink to search for
   * @return     The devlink of the device owning ol, or NULL if it could not be
   *             found.
   */
  devlink findoutputdevice(const outplink ol);

  /** Returns the collection of switches in the network.
   *
   * @return     A vector containing the devlinks of all SWITCH devices in the
   *             network.
   */
  std::vector<devlink> findswitches();

  /** Find all the output signals
   *
   * @return     The vector of all the output signal names in the network
   */
  std::vector<outputsignal> findoutputsignals();

  /** Returns link to device with specified name.
   *
   * @param[in]  id    The name id of the device to search for.
   * @return     The devlink of the device, or NULL if it could not be found.
   */
  devlink finddevice (name id);

  /** Returns the link to input of a device
   *
   * @param[in]  dev   The device name id
   * @param[in]  id    The input pin name id
   * @return     The inplink of the input on device dev, or NULL if no input
   *             with that name exists.
   */
  inplink findinput (devlink dev, name id);

  /** Returns the link to an ouput of a device
   *
   * @param[in]  dev   The device name id
   * @param[in]  id    The output pin name id
   * @return     The outplink to the output pin on device dev, or NULL if no
   *             output with that name exists.
   */
  outplink findoutput (devlink dev, name id);

  /** Adds a device to the device list with given name and returns a link to it
   *
   * @param[in]  dkind  The device type
   * @param[in]  did    The device name id
   * @param      dev    Returns the device link if successful.
   * @param[in]  at     The source position the device is being added from.
   */
  void adddevice (devicekind dkind, name did, devlink& dev, SourcePos at = SourcePos());

  /** Adds an input to a device
   *
   * @param[in]  dev   The device name id
   * @param[in]  iid   The input pin name id
   * @param[in]  at    The source position the input is being defined from
   */
  void addinput (devlink dev, name iid, SourcePos at = SourcePos());

  /** Adds an output to a device
   *
   * @param[in]  dev   The device name id
   * @param[in]  oid   The output pin name id
   * @param[in]  at    The source position the device is being defined from.
   */
  void addoutput (devlink dev, name oid, SourcePos at = SourcePos());

  /** Creates a connection to the input of one device, from the output of another.
   *
   * @param[in]  idev  The device name id whose input is to be connected
   * @param[in]  inp   The idev input pin name id to be connected
   * @param[in]  odev  The device name id whose output is to be connected
   * @param[in]  outp  The odev output pin name id to be connected
   * @param      ok    Returns true if the connection was made successfully.
   */
  void makeconnection (name idev, name inp, name odev, name outp, bool& ok);

  /** Checks a network for errors
   *  This function checks for errors that can't be detected during parsing,
   *  such as inputs left floating or properties left undefined.
   *
   * @param      col   A reference to the errorcollector to report errors to.
   */
  void checknetwork (errorcollector& col);

  /** Initialises the network
   *
   * @param      names_mod  The names table instance to use.
   */
  network (names* names_mod);

  /** Clears resources allocated by network
   */
  ~network();

 private:
  devlink devs;          // the list of devices
  devlink lastdev;       // last device in list of devices

};

#endif /* network_h */
