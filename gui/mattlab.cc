
#include "gui.h"
#include <GL/glut.h>
#include "mattlab.h"


IMPLEMENT_APP(MyApp)


bool MyApp::OnInit()
  // This function is automatically called when the application starts
{
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
}


