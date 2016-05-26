
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
    EVT_BUTTON(MY_RUN_BUTTON_ID, MyFrame::OnRunButton)
    EVT_BUTTON(MY_CONTINUE_BUTTON_ID, MyFrame::OnContinueButton)
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

    // file menu
    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append(ID_FILEOPEN, "&Open\tCtrl+O");
    fileMenu->Append(ID_ADDMONITOR, "Monitor Signal List");
    fileMenu->Append(wxID_ABOUT, "&About");
    fileMenu->Append(wxID_EXIT, "&Quit");

    // view menu
    wxMenu *viewMenu = new wxMenu;
    // colour menu
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

    // top level menu
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(viewMenu, "&View");
    SetMenuBar(menuBar);

    // top level sizer
    wxBoxSizer *topsizer = new wxBoxSizer(wxHORIZONTAL);
    
    // canvas
    canvas = new MyGLCanvas(this, monitorOrder, wxID_ANY, NULL, NULL);
    topsizer->Add(canvas, 1, wxEXPAND | wxALL, 10);


    // run and continue buttons
    wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
    runbutton = new wxButton(this, MY_RUN_BUTTON_ID, "Run");
    runbutton->Enable(false);
    button_sizer->Add(runbutton, 0, wxALL, 10);
    continuebutton = new wxButton(this, MY_CONTINUE_BUTTON_ID, "Continue");
    button_sizer->Add(continuebutton, 0, wxALL, 10);

    // sizer for cycle selector
    wxBoxSizer *cycle_sizer = new wxBoxSizer(wxHORIZONTAL);
    cycle_sizer->Add(new wxStaticText(this, wxID_ANY, "Cycles:"), 0, wxALL, 15);
    spin = new wxSpinCtrl(this, MY_SPINCNTRL_ID, wxString("10"));
    spin->SetRange(1, 1000);
    cycle_sizer->Add(spin, 0 , wxALL, 10);
    // sizer for cycles and run buttons

    wxStaticBox *run_box = new wxStaticBox(this, wxID_ANY, "Run controls");
    wxStaticBoxSizer *run_sizer = new wxStaticBoxSizer(run_box, wxVERTICAL);
    run_sizer->Add(cycle_sizer, 0, wxALL, 10);
    run_sizer->Add(button_sizer, 0, wxALL|wxALIGN_CENTER, 10);

    // Switches
    wxArrayString switchItems;
    switchlist = new wxCheckListBox(this, MY_SWITCH_LIST_ID, wxDefaultPosition, wxDefaultSize, switchItems);
    // switch sizer
    wxStaticBox *switch_box = new wxStaticBox(this, wxID_ANY, "Switches");
    wxStaticBoxSizer *switch_sizer = new wxStaticBoxSizer(switch_box, wxVERTICAL);
    switch_sizer->Add(switchlist, 1, wxALL|wxEXPAND, 10);

    // Monitors
    wxArrayString monitorItems;
    wxArrayInt monitorOrder;
    // monitorlist = new wxRearrangeCtrlMatt(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, monitorOrder, monitorItems);

    wxPanel* listpanel = new wxPanel(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, wxRearrangeListMattNameStr);

    monitorlist = new wxRearrangeListMatt(listpanel, MONITOR_BOX_ID,
                                 wxDefaultPosition, wxDefaultSize,
                                 monitorOrder, monitorItems,
                                 0, wxDefaultValidator);
    btnAdd = new wxButton(listpanel, wxID_ADD);
    btnUp = new wxButton(listpanel, wxID_UP);
    btnDown = new wxButton(listpanel, wxID_DOWN);

    wxBoxSizer *monitor_btns_sizer = new wxBoxSizer(wxHORIZONTAL);
    monitor_btns_sizer->Add(btnAdd, 0, wxALL, 3);
    monitor_btns_sizer->Add(btnUp, 0, wxALL, 3);
    monitor_btns_sizer->Add(btnDown, 0, wxALL, 3);


    wxSizer * const sizerTop = new wxBoxSizer(wxVERTICAL);
    sizerTop->Add(monitorlist, 1, wxALL|wxEXPAND, 0);
    sizerTop->Add(monitor_btns_sizer, 0, wxALL|wxEXPAND, 0);
    listpanel->SetSizer(sizerTop);

    // wrap Diesel's monitor panel in a static box sizer.
    wxStaticBox *monitor_box = new wxStaticBox(this, wxID_ANY, "Monitors");
    wxStaticBoxSizer *monitor_sizer = new wxStaticBoxSizer(monitor_box, wxVERTICAL);
    monitor_sizer->Add(listpanel, 1, wxALL, 10);



    controls_sizer = new wxBoxSizer(wxVERTICAL);
    controls_sizer->Add(switch_sizer, 1, wxALL|wxEXPAND, 0);
    controls_sizer->Add(monitor_sizer, 1, wxALL|wxEXPAND, 0);
    controls_sizer->Add(run_sizer, 0, wxALL|wxEXPAND, 0);

    topsizer->Add(controls_sizer, 0, wxALL|wxEXPAND, 10);

    // disable all buttons
    toggleButtonsEnabled(false);

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

    continuebutton->Enable(false);
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

void MyFrame::OnRunButton(wxCommandEvent &event)
    // Event handler for the push button
{
    if (!fileOpen) return;
    // Todo: is this a true reset?
    mmz->resetmonitor();
    int spinValue = spin->GetValue();
    if (runnetwork(spinValue)) {
        canvas->resetCycles();
        canvas->Render(spinValue);
        continuebutton->Enable(true);
    }
}

void MyFrame::OnContinueButton(wxCommandEvent &event) 
{
    if (!fileOpen) return;

    int spinValue = spin->GetValue();
    if (runnetwork(spinValue)) {
        canvas->Render(spinValue);
    }
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

bool MyFrame::runnetwork(int ncycles)
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
    return ok;
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
    toggleButtonsEnabled(true);

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
    toggleButtonsEnabled(false);
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

// Toggles whether buttons in control panel are enabled or disabled
void MyFrame::toggleButtonsEnabled(bool enabled){
    btnAdd->Enable(enabled);
    btnDown->Enable(enabled);
    btnUp->Enable(enabled);
    runbutton->Enable(enabled);
    spin->Enable(enabled);
    // function only disables continuebutton
    if (!enabled) continuebutton->Enable(enabled);
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