
#include <iostream>

#include "logsim.h"
#include "userint.h"

#ifdef USE_GUI
#include "gui.h"
#include <GL/glut.h>
#endif

#ifdef USE_GUI
IMPLEMENT_APP(MyApp)
#endif


bool MyApp::OnInit()
  // This function is automatically called when the application starts
{

#ifdef USE_GUI

    // glutInit cannot cope with Unicode command line arguments, so we pass
    // it some fake ASCII ones instead
    char **tmp1; int tmp2 = 0; glutInit(&tmp2, tmp1);
    // Construct the GUI
    MyFrame *frame = new MyFrame(NULL, wxDefaultPosition,  wxSize(800, 600));

    frame->Show(true);
    
    if (argc >= 2) {
      frame->openFile(argv[1]);
    }

    return(true); // enter the GUI event loop

#else

    if (argc != 2) { // check we have one command line argument
      std::cout << "Usage:      " << argv[0] << " [filename]" << std::endl;
      exit(1);
    }

    nmz = new names();
    netz = new network(nmz);
    dmz = new devices(nmz, netz);
    mmz = new monitor(nmz, netz);
    smz.open(argv[1]);
    parser* pmz = new parser(netz, dmz, mmz, smz, nmz);

    if (pmz->readin ()) { // check the logic file parsed correctly
      // Construct the text-based interface
      userint umz(nmz, dmz, mmz);
      umz.userinterface();
    }

#endif /* USE_GUI */

  return(false); // exit the application
}


#ifndef USE_GUI

int main(int argc, char const *argv[])
{
  MyApp app;
  app.argc = argc;
  app.argv = argv;
  app.OnInit();

  return 0;
}

#endif
