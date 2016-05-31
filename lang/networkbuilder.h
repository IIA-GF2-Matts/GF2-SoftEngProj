

#ifndef GF2_NETWORKBUILDER_H
#define GF2_NETWORKBUILDER_H

#include <ostream>

#include "../com/names.h"
#include "../com/errorhandler.h"
#include "../sim/network.h"
#include "../sim/devices.h"
#include "../sim/monitor.h"

#include "scanner.h"
#include "parser.h"


/** Signal Legality enum
 * Used by the network builder to represent the semantic legality of signals
 * and identify the cause of illegitimate signals
 */
typedef enum {
    LEGAL_SIGNAL = 0,
    ILLEGAL_DEVICE,
    ILLEGAL_PIN
} signal_legality;


/** Network Builder
 *
 * This is responsible for applying semantic checks on parser output,
 * and subsequently adding devices to the network.
 *
 * @author Judge
 */
class networkbuilder
{
private:
    network* _netz; ///< The network which devices should be added to
    devices* _devz; ///< Representing the interface with devices on the network
    monitor* _mons; ///< Responsible for interfacing with monitor points on the network
    names* _nms; ///< The names table
    errorcollector& _errs; ///< The error collector to which errors will be reported


     /** Method to generate the error message when a pin / property has been predefined
     *
     * @param[in]  dvl      The devlink to the device owning the property being assigned to
     * @param[in]  key      The key of the property
     *
     * @return              The error message string
     */
    std::string getPredefinedError(devlink dvl, name key);


    /** Method to generate the note message when a pin / property has been predefined
     *
     * @param[in]  prevval  The token to the previous assignment of the property
     *
     * @return              The note message string
     */
    template<typename T>
    std::string getPredefinedNote(T prevval);


    /** Method to generate the error message when a signal pin has not been defined
     *
     * @param[in]  sig      The signal with the unknown pin
     *
     * @return     The error message
     */
    std::string getUnknownPinError(Signal& sig);


    /** Determine if a given property key is legal for a given device
     *
     * @param[in]  dl       The devlink to the device owning the property
     * @param[in]  keyname  The key of the property
     *
     * @return    true if legal, false if not
     */
    bool isLegalProperty(devlink dl, name keyname);


    /** Creates a network link between a devices input pin and a signal output
     *  Checks the legality of such a connection prior to linking
     *  Reports errors if connection found to be illegitimate
     *
     * @param[in]  dvl      The devlink to the device owning the input pin being connected
     * @param[in]  keytk    The token to the key representing the input pin
     * @param[in]  signal   The signal to which the input pin is to be connected
     *
     * @return
     */
    void assignPin(devlink dvl, Token keytk, Signal sig);


    /** Assigns a value to a device's property
     *  Checks and reports invalid values
     *  Note: the validity of the key should be checked first
     *
     * @param[in]  dvl      The devlink to the device owning the property being assigned to
     * @param[in]  keytk    The token to the key of the property
     * @param[in]  valuetk  The token to the value of the property
     *
     * @return
     */
    void assignProperty(devlink dvl, Token keytk, Token valuetk);


    /** Checks if a signal already exists (whether the device and pin of a signal are defined)
     *
     * @param[in]  sig      The signal to check
     *
     * @return    a signal_legality value representing the legality of the signal
     */
    signal_legality isBadSignal(Signal& sig);

    /** Checks if a given key is legal for a device
     *  Reports errors if the key is found to be illegal
     *
     * @param[in]  dvl      The devlink to the device owning the key
     * @param[in]  keyTo    The token to the key of the property
     *
     * @return    true if the key is legal
     */
    bool checkKey(devlink dvl, Token& keyTok);

    /** Checks if a name follows the correct format for a gate input (I##, where ## is 1-maxn)
     *
     * @param[in]  n        The name to be tested
     * @param[in]  maxn     Maximum number of inputs to the gate
     *
     * @return    true if the name is a legal gate input name
     */
    bool isLegalGateInputNamestring(name n, int maxn);


public:
    /** The constructor for the network builder
     *
     * @param[in]  netz     The instance of the network to use and add devices to
     * @param[in]  devz     The instance of devices class to use
     * @param[in]  mons     The instance of monitors class to use
     * @param[in]  nms      The instance of the name table to use
     * @param[in]  errc     The error reporter to use
     *
     * @return    an instance of network builder
     */
    networkbuilder(network* netz, devices* devz, monitor* mons, names* nms, errorcollector& errc);


    /** The destructor for the network builder
     */
    ~networkbuilder();


    /** Defines and adds a new device to the network if determined to be a legal action
     *  Reports errors to the error collector if determined to be illegal
     *
     * @param[in]  devName  Token to the device name
     * @param[in]  type     Token to the device type
     *
     * @return
     */
	void defineDevice(Token& devName, Token& type);

    /** Sets an input property value of a device if determined to be a legal action
     *  Reports errors to the error collector if determined to be illegal
     *
     * @param[in]  devName  Token to the device name
     * @param[in]  keyTok   Token to the key of the property
     * @param[in]  valTok   Token to the value of the property
     *
     * @return
     */
    void setInputValue(Token& devName, Token& keyTok, Token& valTok);

    /** Sets an input pin of a device to a signal if determined to be a legal action
     *  Reports errors to the error collector if determined to be illegal
     *
     * @param[in]  devName  Token to the device name
     * @param[in]  keyTok   Token to the key of the property
     * @param[in]  valSig   Signal to which the input pin should be connected
     *
     * @return
     */
    void setInputSignal(Token& devName, Token& keyTok, Signal& valSig);


    /** Creates a new monitor point if determined to be a legal action
     *  Reports errors to the error collector if determined to be illegal
     *
     * @param[in]  monSig   Signal to be monitored
     *
     * @return
     */
    void defineMonitor(Signal& monSig);

    /** Creates a new monitor point with an alias if determined to be a legal action
     *  Reports errors to the error collector if determined to be illegal
     *
     * @param[in]  monSig   Signal to be monitored
     * @param[in]  aliSig   How the signal should be represented to the user
     *
     * @return
     */
    void defineMonitor(Signal& monSig, Signal& aliSig);

    /** Imports and adds a new device to the network from a secondary file if determined to be a legal action
     *  Reports errors to the error collector if determined to be illegal
     *
     * @param[in]  devName  Token to the device name
     * @param[in]  fileStr  Token to the file in which the device is defined
     *
     * @return
     */
    void importDevice(Token& devName, Token& fileStr);
};


#endif
