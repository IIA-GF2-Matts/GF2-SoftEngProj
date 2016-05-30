

#include <string>
#include <sstream>
#include <vector>
#include <wx/wx.h>
#include "guimonitordialog.h"


MonitorDialog::MonitorDialog(wxWindow* parent,
        wxWindowID id,
        names* nms,
        const std::vector<outputsignal>& signals,
        std::vector<bool>& monitored,
        wxString title)
    : wxDialog(parent, id, title,
            wxDefaultPosition, wxSize(720, 400)), _nmz(nms), _signals(signals), _monitor(monitored) {


    // Setup the Panel and Widgets.
    wxPanel* panel = new wxPanel(this, wxID_ANY);

    wxBoxSizer* rowsizer = new wxBoxSizer( wxVERTICAL );

    wxArrayString switchItems;
    name D, P;
    std::ostringstream oss;
    for (auto s : signals) {
        oss.str("");
        oss << _nmz->namestr(s.devicename);
        if (s.pinname != blankname) {
            oss << "." << _nmz->namestr(s.pinname);
        }
        switchItems.Add(oss.str());

    }
    switchlist = new wxCheckListBox(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, switchItems);

    for (int n = 0; n < monitored.size(); n++) {
        if (monitored[n])
            switchlist->Check(n, true);
    }

    wxButton* okButton = new wxButton(panel, wxID_ANY, "Ok");


    rowsizer->Add(switchlist, 0,
            wxEXPAND | wxALL, 10);


    rowsizer->Add(okButton, 0,
            wxALL | wxALIGN_RIGHT | wxALIGN_BOTTOM, 10);


    panel->SetSizer(rowsizer);

    // Bind the widgets and event handlers.
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MonitorDialog::onOk,
            this, okButton->GetId());

    Center();
}

MonitorDialog::~MonitorDialog() {}


void MonitorDialog::onOk(wxCommandEvent& WXUNUSED(pEvent)) {

    for (int n = 0; n < _monitor.size(); n++) {
        _monitor[n] = switchlist->IsChecked(n);
    }
    EndModal(wxID_OK);
    Destroy();
}
