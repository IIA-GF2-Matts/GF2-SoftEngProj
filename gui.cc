
#include "gui.h"
#include "guicanvas.h"
#include "logo32.xpm"
#include "scanner.h"
#include "parser.h"
#include <iostream>
#include <sstream>

using namespace std;

#include "guicanvas.cc"

// MyFrame ///////////////////////////////////////////////////////////////////////////////////////


BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MyFrame::OnExit)
    EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
    EVT_MENU(ID_FILEOPEN, MyFrame::OnOpen)
    EVT_BUTTON(MY_BUTTON_ID, MyFrame::OnButton)
    EVT_SPINCTRL(MY_SPINCNTRL_ID, MyFrame::OnSpin)
    EVT_TEXT_ENTER(MY_TEXTCTRL_ID, MyFrame::OnText)
END_EVENT_TABLE()

MyFrame::MyFrame(wxWindow *parent, const wxPoint& pos, const wxSize& size, long style):
    wxFrame(parent, wxID_ANY, "Mattlab", pos, size, style)
    // Constructor - initialises pointers to names, devices and monitor classes, lays out widgets
    // using sizers
{
    SetIcon(wxIcon(logo32));

    hasNetwork = false;
    fileOpen = false;
    cyclescompleted = 0;

    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append(ID_FILEOPEN, "&Open\tCtrl+O");
    fileMenu->Append(wxID_ABOUT, "&About");
    fileMenu->Append(wxID_EXIT, "&Quit");
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&File");
    SetMenuBar(menuBar);

    wxBoxSizer *topsizer = new wxBoxSizer(wxHORIZONTAL);
    canvas = new MyGLCanvas(this, wxID_ANY, NULL, NULL);
    topsizer->Add(canvas, 1, wxEXPAND | wxALL, 10);

    wxBoxSizer *button_sizer = new wxBoxSizer(wxVERTICAL);
    button_sizer->Add(new wxButton(this, MY_BUTTON_ID, "Run"), 0, wxALL, 10);
    button_sizer->Add(new wxStaticText(this, wxID_ANY, "Cycles"), 0, wxTOP|wxLEFT|wxRIGHT, 10);
    spin = new wxSpinCtrl(this, MY_SPINCNTRL_ID, wxString("10"));
    button_sizer->Add(spin, 0 , wxALL, 10);

    // button_sizer->Add(new wxTextCtrl(this, MY_TEXTCTRL_ID, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER), 0 , wxALL, 10);
    topsizer->Add(button_sizer, 0, wxALIGN_CENTER);

    SetSizeHints(400, 400);
    SetSizer(topsizer);
}

void MyFrame::OnExit(wxCommandEvent &event)
    // Event handler for the exit menu item
{
    Close(true);
}

void MyFrame::OnOpen(wxCommandEvent &event)
    // Event handler for the File->Open menu item
{
    openFile("test.matt");
}

void MyFrame::OnAbout(wxCommandEvent &event)
    // Event handler for the about menu item
{
    wxMessageDialog about(this, "Example wxWidgets GUI\nAndrew Gee\nJune 2014", "About Logsim", wxICON_INFORMATION | wxOK);
    about.ShowModal();
}

void MyFrame::OnButton(wxCommandEvent &event)
    // Event handler for the push button
{
    int n, ncycles;

    cyclescompleted = 0;
    mmz->resetmonitor ();
    runnetwork(spin->GetValue());
    canvas->Render("Run button pressed", cyclescompleted);
}

void MyFrame::OnSpin(wxSpinEvent &event)
    // Event handler for the spin control
{
    wxString text;

    text.Printf("New spinctrl value %d", event.GetPosition());
    canvas->Render(text);
}

void MyFrame::OnText(wxCommandEvent &event)
    // Event handler for the text entry field
{
    wxString text;

    text.Printf("New text entered %s", event.GetString().c_str());
    canvas->Render(text);
}

void MyFrame::runnetwork(int ncycles)
    // Function to run the network, derived from corresponding function in userint.cc
{
    bool ok = true;
    int n = ncycles;

    while ((n > 0) && ok) {
        dmz->executedevices (ok);
        if (ok) {
            n--;
            mmz->recordsignals ();
        } else
            cout << "Error: network is oscillating" << endl;
    }
    if (ok) cyclescompleted += ncycles;
    else cyclescompleted = 0;
}


void MyFrame::initNetwork() {
    if (hasNetwork) {
        return; // Network already initialised
    }

    nmz = new names();
    netz = new network(nmz);
    dmz = new devices(nmz, netz);
    mmz = new monitor(nmz, netz);

    hasNetwork = true;
    fileOpen = false;
    updateTitle();
}

void MyFrame::delNetwork() {
    if (!hasNetwork) {
        return; // Nothing to do.
    }

    hasNetwork = false;
    closeFile();

    delete mmz;
    delete dmz;
    delete netz;
    delete nmz;
}

void MyFrame::openFile(wxString file) {
    if (fileOpen) {
        delNetwork();
    }

    if (!hasNetwork) {
        initNetwork();
    }

    fscanner scan;
    scan.open(std::string(file.mb_str()));

    parser pmz(netz, dmz, mmz, scan, nmz);

    pmz.readin();

    if (pmz.errors().errors.size()) {
        // Errors occurred
        delNetwork();
        return;
    }


    canvas->setNetwork(mmz, nmz);
    fname = file;
    fileOpen = true;
    updateTitle();
}

void MyFrame::closeFile() {
    cyclescompleted = 0;
    fname = "";
    fileOpen = false;
    updateTitle();
}

void MyFrame::updateTitle() {
    // "Mattlab"
    std::ostringstream oss;

    oss << "Mattlab";

    if (fileOpen) {
        oss << " - [" << fname << "]";
    }

    SetTitle(oss.str());
}
