
#include <wx/wx.h>

#include "../com/names.h"
#include "../sim/devices.h"
#include "../sim/monitor.h"
#include "../sim/network.h"
#include "../lang/scanner.h"
#include "../lang/parser.h"


#ifndef logsim_h
#define logsim_h


class MyApp : public wxApp {
public:
  bool OnInit(); // automatically called when the application starts

private:
  names *nmz;    // pointer to the names class
  network *netz; // pointer to the network class
  devices *dmz;  // pointer to the devices class
  monitor *mmz;  // pointer to the monitor class
  fscanner *smz;  // The scanner class
  parser *pmz;   // pointer to the parser class
};


#endif /* logsim_h */
