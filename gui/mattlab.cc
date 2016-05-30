
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
    m_locale = new wxLocale(wxLANGUAGE_DEFAULT, wxLOCALE_DONT_LOAD_DEFAULT);
    m_locale->AddCatalogLookupPathPrefix(wxPathOnly(argv[0]) + "/intl");
    bool translationSuccess = !m_locale->AddCatalog(wxT("mattlab"));


    // Construct the GUI
    MyFrame *frame = new MyFrame(NULL, wxDefaultPosition,  wxSize(800, 600));

    frame->Show(true);

    // Todo: There must be a way of checking if Locale is any English dialect rather than the current
    // loading of a blank translation file for english.
    // Create error message if language catalog was not found.
    if (translationSuccess) {
        wxMessageBox(wxT("No translations available for current language. Reverting to English."),
            wxT("Language not available"), wxOK|wxCENTRE, frame);
    }

    if (argc >= 2) {
      frame->openFile(argv[1]);
    }

    return(true); // enter the GUI event loop
}


