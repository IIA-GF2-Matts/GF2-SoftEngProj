
#include "gui.h"
#include "guicanvas.h"
#include "logo32.xpm"
#include "scanner.h"
#include "parser.h"
#include <sstream>
#include <algorithm>
#include <wx/filedlg.h>
#include "guierrordialog.h"
#include "rearrangectrl_matt.h"
#include "guimonitordialog.h"

#include <iostream>

using namespace std;

#include "guicanvas.cc"

// MyFrame ///////////////////////////////////////////////////////////////////////////////////////


BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MyFrame::OnExit)
    EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
    EVT_MENU(ID_FILEOPEN, MyFrame::OnOpen)
    EVT_MENU(ID_ADDMONITOR, MyFrame::OnAddMonitor)
    EVT_BUTTON(MY_RUN_BUTTON_ID, MyFrame::OnButton)
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

    // monitor manipulation
    EVT_BUTTON(wxID_ADD, MyFrame::OnAddMonitor)
    EVT_BUTTON(wxID_UP, MyFrame::OnMonitorUp)
    EVT_BUTTON(wxID_DOWN, MyFrame::OnMonitorDown)
    
    
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
    fileMenu->Append(ID_ADDMONITOR, "Monitor Signal List");
    fileMenu->Append(wxID_ABOUT, "&About");
    fileMenu->Append(wxID_EXIT, "&Quit");

    wxMenu *viewMenu = new wxMenu;
    wxMenu *colourMenu = new wxMenu;
    colourMenu->AppendRadioItem(BLUE_ID, "Cool Blue");
    colourMenu->AppendRadioItem(GREEN_ID, "Retro Green");
    colourMenu->AppendRadioItem(BW_ID, "Simple B+W");
    colourMenu->AppendRadioItem(PINK_ID, "Candy Pink");
    viewMenu->Append(wxID_ANY, "Colour Theme", colourMenu);

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
    canvas = new MyGLCanvas(this, monitorOrder, wxID_ANY, NULL, NULL);
    topsizer->Add(canvas, 1, wxEXPAND | wxALL, 10);

    wxBoxSizer *button_sizer = new wxBoxSizer(wxVERTICAL);
    runbutton = new wxButton(this, MY_RUN_BUTTON_ID, "Run");
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
    wxArrayString monitorItems;
    wxArrayInt monitorOrder;
    // monitorlist = new wxRearrangeCtrlMatt(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, monitorOrder, monitorItems);

    wxPanel* listpanel = new wxPanel(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, wxRearrangeListMattNameStr);

    monitorlist = new wxRearrangeListMatt(listpanel, wxID_ANY,
                                 wxDefaultPosition, wxDefaultSize,
                                 monitorOrder, monitorItems,
                                 0, wxDefaultValidator);
    wxButton * const btnAdd = new wxButton(listpanel, wxID_ADD);
    wxButton * const btnUp = new wxButton(listpanel, wxID_UP);
    wxButton * const btnDown = new wxButton(listpanel, wxID_DOWN);
    // arrange them in a sizer
    wxSizer * const sizerBtns = new wxBoxSizer(wxVERTICAL);
    sizerBtns->Add(btnAdd, wxSizerFlags().Centre().Border(wxBOTTOM));
    sizerBtns->Add(btnUp, wxSizerFlags().Centre().Border(wxBOTTOM | wxTOP));
    sizerBtns->Add(btnDown, wxSizerFlags().Centre().Border(wxTOP));

    wxSizer * const sizerTop = new wxBoxSizer(wxHORIZONTAL);
    sizerTop->Add(monitorlist, wxSizerFlags(1).Expand().Border(wxRIGHT));
    sizerTop->Add(sizerBtns, wxSizerFlags(0).Centre().Border(wxLEFT));
    listpanel->SetSizer(sizerTop);




    controls_sizer->Add(switchlist, 0, wxALL|wxALIGN_TOP|wxEXPAND, 10);
    controls_sizer->Add(listpanel, 0, wxALL|wxALIGN_TOP|wxEXPAND, 10);
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

void MyFrame::OnAddMonitor(wxCommandEvent &event)
    // Event handler for the Add monitor button
{
    MonitorDialog dlg(this, wxID_ANY, nmz, signals, monitored);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    // todo: update list.
    
    int x;
    bool ok;
    for (int i = 0; i < signals.size(); i++) {
        if (monitored[i]) {
            x = mmz->findmonitor(signals[i].devicename, signals[i].pinname);
            if (x == -1) {
                mmz->makemonitor(signals[i].devicename, signals[i].pinname, ok, blankname, blankname);
                //std::cout << "Created monitor " << oss.str() << std::endl;
                if (!ok) {
                    // Todo: Error message
                }
            }
        }
    }
    RefreshMonitors();
}


void MyFrame::RefreshMonitors() {

    wxArrayString monitorItems;
    wxArrayInt wxmonitorOrder;

    monitorOrder.clear();
    name D, P;
    std::ostringstream oss;
    int i;
    for (int n = 0; n < mmz->moncount(); n++) {
        mmz->getmonname(n, D, P);

        oss.str("");
        oss << nmz->namestr(D);
        if (P != blankname) {
            oss << "." << nmz->namestr(P);
        }

        monitorItems.Add(oss.str());
        wxmonitorOrder.Add(n);
        monitorOrder.push_back(n);

        // check in the monitored table
        // Todo: Not linear search.
        mmz->getmonname(n, D, P, false);
        for (i = 0; i < signals.size(); i++) {
            if (D == signals[i].devicename && P == signals[i].pinname) {
                monitored[i] = true;
                break;
            }
        }
    }

    if (mmz->moncount())
        monitorlist->Reset(wxmonitorOrder, monitorItems);
    else
        monitorlist->Clear();

    for ( int i = 0; i < mmz->moncount(); i++ ) {
        monitorlist->Check(i, true);
    }
    canvas->Render();
}

void MyFrame::OnMonitorUp(wxCommandEvent &event) {
    if (monitorlist->MoveCurrentUp()) {  // moves selection up one
        // get new position
        const int sel = monitorlist->GetSelection();
        // swap new position and old one in monitor order
        std::iter_swap(monitorOrder.begin() + sel,
            monitorOrder.begin() + sel + 1);
    }
    canvas->Render();
}

void MyFrame::OnMonitorDown(wxCommandEvent &event) {
    if (monitorlist->MoveCurrentDown()) {  // moves selection down one
        // get new position
        const int sel = monitorlist->GetSelection();
        // swap new position and old one in monitor order
        std::iter_swap(monitorOrder.begin() + sel,
            monitorOrder.begin() + sel - 1);
    }
    canvas->Render();
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

    // Get info
    switches = netz->findswitches();
    signals = netz->findoutputsignals();
    monitored.clear();
    monitored.resize(signals.size(), false);

    // Update controls
    runbutton->Enable(true);

    // Add monitors to list
    RefreshMonitors();

    // Add switches to the list
    wxArrayString switchItems;

    switchlist->Clear();

    for (auto sw : switches) {
        switchItems.Add(nmz->namestr(sw->id).c_str());
    }
    if (!switchItems.IsEmpty()) {
        switchlist->InsertItems(switchItems, 0);
        int n = 0;
        for (auto sw : switches) {
            switchlist->Check(n++, sw->swstate == high);
        }
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
