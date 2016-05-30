
#include "gui.h"
#include <GL/glut.h>
#include "mattlab.h"
#include <wx/intl.h>


IMPLEMENT_APP(MyApp)


bool MyApp::OnInit()
  // This function is automatically called when the application starts
{
    // glutInit cannot cope with Unicode command line arguments, so we pass
    // it some fake ASCII ones instead
    char **tmp1; int tmp2 = 0; glutInit(&tmp2, tmp1);

    // sort out locale for translations before GUI contructed.
    m_locale = new wxLocale(wxLANGUAGE_DEFAULT);
    if (m_locale->IsOk()) {
        m_locale->AddCatalogLookupPathPrefix(wxPathOnly(argv[0]));
        if (m_locale->AddCatalog(wxT("mattlab"))) {
            std::cout << "Loaded catalog!" << std::endl;
        }
        else {
            std::cout << "Not found catalog!" << std::endl;
        }
    }
    else {
        std::cout << "Locale not working or recognised!?!?" << std::endl;
    }


    // Construct the GUI
    MyFrame *frame = new MyFrame(NULL, wxDefaultPosition,  wxSize(800, 600));

    frame->Show(true);

    if (argc >= 2) {
      frame->openFile(argv[1]);
    }

    return(true); // enter the GUI event loop
}


