
#include "gui.h"
#include <GL/glut.h>
#include "mattlab.h"
#include <wx/intl.h>
#include "../com/localestrings.h"


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
    bool translationFailed = !m_locale->AddCatalog(wxT("mattlab"));

    // Load mattlang translations
    LocaleStrings::AddTranslations("", "mattlang");

    // Construct the GUI
    MyFrame *frame = new MyFrame(NULL, wxDefaultPosition,  wxSize(800, 600));

    frame->Show(true);


    // Create error message if language catalog was not found and locale is not english.
    if (translationFailed && m_locale->GetName().substr(0,2) != "en") {
        wxMessageBox(wxT("No translations available for current language. Reverting to English."),
            wxT("Language not available"), wxOK|wxCENTRE, frame);
    }

    if (argc >= 2) {
      frame->openFile(argv[1]);
    }

    return(true); // enter the GUI event loop
}


