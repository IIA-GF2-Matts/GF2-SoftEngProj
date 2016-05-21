

#ifdef USE_GUI
#include <wx/wx.h>
#endif

#include "names.h"
#include "devices.h"
#include "monitor.h"
#include "network.h"
#include "parser.h"

#ifndef logsim_h
#define logsim_h



#ifdef USE_GUI
class MyApp : public wxApp {
#else
class MyApp {
#endif

 public:

#ifndef USE_GUI
  int argc;
  char const** argv;
#endif

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
