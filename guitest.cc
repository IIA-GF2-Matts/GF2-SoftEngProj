  #include "guitest.h"
#include "gui.h"
#include <GL/glut.h>

#include "userint.h"
#include "network.h"
#include "names.h"
#include "devices.h"
#include "monitor.h"

IMPLEMENT_APP(MyApp)
  
bool MyApp::OnInit()
  // This function is automatically called when the application starts
{
  bool f = true;
  int i = 0;

  // Construct the four classes required by the innards of the logic simulator
  nmz = new names();
  netz = new network(nmz);
  dmz = new devices(nmz, netz);
  mmz = new monitor(nmz, netz);


  // Store name iterators
  name SW1 = nmz->lookup("SW1");
  name LOW = nmz->lookup("LOW");
  
  name b0 = nmz->lookup("b0");
  name b1 = nmz->lookup("b1");
  name b2 = nmz->lookup("b2");
  
  name G0 = nmz->lookup("G0");
  name G1 = nmz->lookup("G1");

  name I1 = nmz->lookup("I1");
  name I2 = nmz->lookup("I2");
  name D = nmz->lookup("DATA");
  name CLK = nmz->lookup("CLK");
  name SET = nmz->lookup("SET");
  name CLEAR = nmz->lookup("CLEAR");
  name Q = nmz->lookup("Q");
  name QBAR = nmz->lookup("QBAR");


  dmz->makedevice(aswitch, LOW, 0, f);
  i++; if (!f) std::cout << i << std::endl;
  


  // Create Device
  dmz->makedevice(aclock, SW1, 1, f);
  i++; if (!f) std::cout << i << std::endl;
  dmz->makedevice(dtype, b0, 0, f);
  i++; if (!f) std::cout << i << std::endl;
  dmz->makedevice(dtype, b1, 0, f);
  i++; if (!f) std::cout << i << std::endl;
  dmz->makedevice(dtype, b2, 0, f);
  i++; if (!f) std::cout << i << std::endl;

  dmz->makedevice(xorgate, G0, 2, f);
  i++; if (!f) std::cout << i << std::endl;
  dmz->makedevice(xorgate, G1, 2, f);
  i++; if (!f) std::cout << i << std::endl;
  


  // Make connection
  netz->makeconnection(b0, D, b0, QBAR, f);
  i++; if (!f) std::cout << i << std::endl;
  netz->makeconnection(b0, CLK, SW1, blankname, f);
  i++; if (!f) std::cout << i << std::endl;
  netz->makeconnection(b0, SET, LOW, blankname, f);
  i++; if (!f) std::cout << i << std::endl;
  netz->makeconnection(b0, CLEAR, LOW, blankname, f);
  i++; if (!f) std::cout << i << std::endl;
  
  netz->makeconnection(b1, D, b1, QBAR, f);
  i++; if (!f) std::cout << i << std::endl;
  netz->makeconnection(b1, CLK, b0, QBAR, f);
  i++; if (!f) std::cout << i << std::endl;
  netz->makeconnection(b1, SET, LOW, blankname, f);
  i++; if (!f) std::cout << i << std::endl;
  netz->makeconnection(b1, CLEAR, LOW, blankname, f);
  i++; if (!f) std::cout << i << std::endl;
  
  netz->makeconnection(b2, D, b2, QBAR, f);
  i++; if (!f) std::cout << i << std::endl;
  netz->makeconnection(b2, CLK, b1, QBAR, f);
  i++; if (!f) std::cout << i << std::endl;
  netz->makeconnection(b2, SET, LOW, blankname, f);
  i++; if (!f) std::cout << i << std::endl;
  netz->makeconnection(b2, CLEAR, LOW, blankname, f);
  i++; if (!f) std::cout << i << std::endl;
  

  netz->makeconnection(G0, I1, b0, Q, f);
  i++; if (!f) std::cout << i << std::endl;
  netz->makeconnection(G0, I2, b1, Q, f);
  i++; if (!f) std::cout << i << std::endl;
  
  netz->makeconnection(G1, I1, b1, Q, f);
  i++; if (!f) std::cout << i << std::endl;
  netz->makeconnection(G1, I2, b2, Q, f);
  i++; if (!f) std::cout << i << std::endl;
  

  mmz->makemonitor(b0, Q, f);
  i++; if (!f) std::cout << i << std::endl;
  mmz->makemonitor(b1, Q, f);
  i++; if (!f) std::cout << i << std::endl;
  mmz->makemonitor(b2, Q, f);
  i++; if (!f) std::cout << i << std::endl;
  mmz->makemonitor(G0, blankname, f);
  i++; if (!f) std::cout << i << std::endl;
  mmz->makemonitor(G1, blankname, f);
  i++; if (!f) std::cout << i << std::endl;
  mmz->makemonitor(b2, Q, f);
  i++; if (!f) std::cout << i << std::endl;




  // glutInit cannot cope with Unicode command line arguments, so we pass
  // it some fake ASCII ones instead
  char **tmp1; int tmp2 = 0; glutInit(&tmp2, tmp1);
  // Construct the GUI
  MyFrame *frame = new MyFrame(NULL, "Logic simulator", wxDefaultPosition,  wxSize(800, 600), nmz, dmz, mmz);
  frame->Show(true);
  return(true); // enter the GUI event loop
}
