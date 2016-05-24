
#include "gui.h"
#include "guicanvas.h"
#include "logo32.xpm"
#include "scanner.h"
#include "parser.h"
#include <sstream>
#include <wx/filedlg.h>
#include "guierrordialog.h"
#include "rearrangectrl_matt.h"

#include <iostream>

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
    EVT_MENU(wxID_ZOOM_IN, MyFrame::OnZoomIn)
    EVT_MENU(wxID_ZOOM_OUT, MyFrame::OnZoomOut)
    EVT_MENU(MY_ZOOM_RESET_ID, MyFrame::OnZoomReset)
    // Colours
    EVT_MENU(BLUE_ID, MyFrame::OnColourBlue)
    EVT_MENU(GREEN_ID, MyFrame::OnColourGreen)
    EVT_MENU(BW_ID, MyFrame::OnColourBW)
    EVT_MENU(PINK_ID, MyFrame::OnColourPink)
    
    EVT_CHECKLISTBOX(MY_SWITCH_LIST_ID, MyFrame::OnSwitchListEvent)
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

    wxMenu *viewMenu = new wxMenu;
    wxMenu *colourMenu = new wxMenu;
    colourMenu->AppendRadioItem(BLUE_ID, "Cool Blue");
    colourMenu->AppendRadioItem(GREEN_ID, "Retro Green");
    colourMenu->AppendRadioItem(BW_ID, "Simple B+W");
    colourMenu->AppendRadioItem(PINK_ID, "Candy Pink");
    viewMenu->Append(wxID_ANY, "&Colour", colourMenu);

    // zoom section
    viewMenu->AppendSeparator();
    viewMenu->Append(wxID_ZOOM_IN, "&Zoom in\tCtrl+=");
    viewMenu->Append(wxID_ZOOM_OUT, "&Zoom out\tCtrl+-");
    viewMenu->Append(MY_ZOOM_RESET_ID, "&Reset zoom\tCtrl+0");

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(viewMenu, "&View");
    SetMenuBar(menuBar);

    wxBoxSizer *topsizer = new wxBoxSizer(wxHORIZONTAL);
    canvas = new MyGLCanvas(this, order, wxID_ANY, NULL, NULL);
    topsizer->Add(canvas, 1, wxEXPAND | wxALL, 10);

    wxBoxSizer *button_sizer = new wxBoxSizer(wxVERTICAL);
    runbutton = new wxButton(this, MY_BUTTON_ID, "Run");
    runbutton->Enable(false);
    button_sizer->Add(runbutton, 0, wxBOTTOM|wxALL, 10);
    button_sizer->Add(new wxStaticText(this, wxID_ANY, "Cycles"), 0, wxTOP|wxLEFT|wxRIGHT, 10);
    spin = new wxSpinCtrl(this, MY_SPINCNTRL_ID, wxString("10"));
    spin->SetRange(1, 100);
    button_sizer->Add(spin, 0 , wxALL, 10);

    // Switches
    controls_sizer = new wxBoxSizer(wxVERTICAL);
    wxArrayString switchItems;
    switchItems.Add("Switch1");
    switchlist = new wxCheckListBox(this, MY_SWITCH_LIST_ID, wxDefaultPosition, wxDefaultSize, switchItems);


    // Monitors
    // Lol. It's not my fault, wxWidgets should allow clearing and resetting of the control.
    wxArrayString monitorItems;
    wxArrayInt monitorOrder;
    monitorlist = new wxRearrangeCtrlMatt(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, monitorOrder, monitorItems);

    controls_sizer->Add(switchlist, 0, wxALL|wxALIGN_TOP|wxEXPAND, 10);
    controls_sizer->Add(monitorlist, 0, wxALL|wxALIGN_TOP|wxEXPAND, 10);
    controls_sizer->Add(button_sizer, 0, wxALL|wxALIGN_BOTTOM, 0);
    // Todo: borders around separate control groups (e.g. around monitor controls, switch controls, other controls)?
    // Todo: add monitor dialogue button.
    // Todo: add text to monitor and switch controls.


    // button_sizer->Add(new wxTextCtrl(this, MY_TEXTCTRL_ID, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER), 0 , wxALL, 10);
    topsizer->Add(controls_sizer, 0, wxALIGN_CENTER|wxEXPAND);

    SetSizeHints(800, 500);
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
    wxFileDialog openFileDialog(this, _("Open Mattlab file"), "", "",
                   "Mattlab files (*.matt)|*.matt|All Files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    openFile(openFileDialog.GetPath());
}

void MyFrame::OnAbout(wxCommandEvent &event)
    // Event handler for the about menu item
{
    wxMessageDialog about(this, "Mattlab Digital Logic Simulator\n\n"
        "Produced for Engineering IIA project GF2 2016 by:\n"
        "Matt March (mdm46)\nMatt Judge (mcj33)\nMatt Diesel (md639)", "About Mattlab", wxICON_INFORMATION | wxOK);
    about.ShowModal();
}

void MyFrame::OnButton(wxCommandEvent &event)
    // Event handler for the push button
{
    if (!fileOpen) return;

    runnetwork(spin->GetValue());
    canvas->Render(mmz->cycles());
}

void MyFrame::OnSpin(wxSpinEvent &event)
    // Event handler for the spin control
{
    canvas->Render();
}

void MyFrame::OnText(wxCommandEvent &event)
    // Event handler for the text entry field
{
    canvas->Render();
}

void MyFrame::OnZoomIn(wxCommandEvent &event)
    // Event handler for zooming in
{
    canvas->zoomIn(-0.2);
}


void MyFrame::OnZoomOut(wxCommandEvent &event)
    // Event handler for zooming in
{
    canvas->zoomOut(0.2);
}

void MyFrame::OnZoomReset(wxCommandEvent &event)
    // Event handler for zooming in
{
    canvas->zoomOut(0, true);
}

void MyFrame::OnSwitchListEvent(wxCommandEvent &event)
    // Event handler for (un)checking switch list items
{
    if (!fileOpen) return;

    int n = event.GetInt();
    bool statehigh = switchlist->IsChecked(n);
    devlink sw = switches[n];
    wxASSERT_MSG(sw, "A runtime error occurred; the switch could not be found");
    bool ok = true;
    if (statehigh)
        dmz->setswitch(sw->id, high, ok);
    else
        dmz->setswitch(sw->id, low, ok);
    wxASSERT_MSG(ok, "A runtime error occurred; the switch could not be set");
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
    if (!ok) mmz->resetmonitor();
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

    fscanner scan(nmz);
    scan.open(std::string(file.mb_str()));

    parser pmz(netz, dmz, mmz, &scan, nmz);

    pmz.readin();

    if (pmz.errors().errCount()) {
        ErrorDialog dlg(this, wxID_ANY, pmz.errors());

        dlg.ShowModal();

        delNetwork();
        return;
    }
    else if (pmz.errors().warnCount()) {
        ErrorDialog dlg(this, wxID_ANY, pmz.errors());

        dlg.ShowModal();
    }


    canvas->setNetwork(mmz, nmz);
    fname = file;
    fileOpen = true;
    updateTitle();

    // Update controls
    runbutton->Enable(true);

    // Add monitors to list
    wxArrayString monitorItems;
    wxArrayInt monitorOrder;

    name D, P;
    std::ostringstream oss;
    for (int n = 0; n < mmz->moncount(); n++) {
        mmz->getmonname(n, D, P);

        oss.str("");
        oss << nmz->namestr(D);
        if (P != blankname) {
            oss << "." << nmz->namestr(P);
        }

        monitorItems.Add(oss.str());
        monitorOrder.Add(n);
    }
    if (mmz->moncount())
        monitorlist->GetList()->Reset(monitorOrder, monitorItems);
    else
        monitorlist->GetList()->Clear();

    for ( int i = 0; i < mmz->moncount(); i++ ) {
        monitorlist->GetList()->Check(i, true);
    }


    // Add switches to the list
    switches = netz->findswitches();
    wxArrayString switchItems;

    switchlist->Clear();
    int n = 0;
    for (auto sw : switches) {
        wxString s(nmz->namestr(sw->id).c_str());
        switchlist->InsertItems(++n, &s, n);

        switchlist->Check(n-1, sw->swstate == high);
    }
}

void MyFrame::closeFile() {
    cyclescompleted = 0;
    fname = "";
    fileOpen = false;
    updateTitle();

    // Update Controls
    runbutton->Enable(false);
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

void MyFrame::colourChange(int index) {
    canvas->colourSelector(index);
    canvas->Render();
}

void MyFrame::OnColourBlue(wxCommandEvent &event) {
    colourChange(0);
}

void MyFrame::OnColourGreen(wxCommandEvent &event) {
    colourChange(1);
}

void MyFrame::OnColourBW(wxCommandEvent &event) {
    colourChange(2);
}

void MyFrame::OnColourPink(wxCommandEvent &event) {
    colourChange(3);
}
