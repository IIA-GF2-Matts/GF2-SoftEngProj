

#include <wx/wx.h>
#include <wx/statbmp.h>
#include <wx/artprov.h>
#include <wx/textctrl.h>
#include <string>
#include "guierrordialog.h"
#include "errorhandler.h"

ErrorDialog::ErrorDialog(wxWindow* parent, 
        wxWindowID id, 
        const errorcollector& errc)
    : wxDialog(parent, id, "ERROR! YOU IDIOT!", 
            wxDefaultPosition, wxSize(500, 300)), errs(errc) {


    // Setup the Panel and Widgets.
    wxPanel* panel = new wxPanel(this, wxID_ANY);

    wxBoxSizer* topColSizer = new wxBoxSizer( wxHORIZONTAL );
    wxBoxSizer* rowsizer = new wxBoxSizer( wxVERTICAL );

    // Todo: Choose between them
    // wxBitmap bmp = wxArtProvider::GetBitmap(wxART_ERROR, wxART_MESSAGE_BOX);
    wxBitmap bmp = wxArtProvider::GetBitmap(wxART_WARNING, wxART_MESSAGE_BOX);
    wxStaticBitmap* errIcon = new wxStaticBitmap(panel, wxID_ANY, bmp);


    wxPanel* rpanel = new wxPanel(panel, wxID_ANY);

    wxButton* okButton = new wxButton(rpanel, wxID_ANY, "Ok");
    wxStaticText* errorMessage = new wxStaticText(rpanel, wxID_ANY, "There were N errors and N warnings processing the input file.");


    wxTextCtrl* errorList = new wxTextCtrl(rpanel, wxID_ANY
                , wxEmptyString, wxDefaultPosition, wxDefaultSize
                , wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);

    wxFont font1(10, wxFONTFAMILY_TELETYPE, 
        wxFONTSTYLE_NORMAL, wxFONTSTYLE_NORMAL, 0, "Consolas");
    errorList->SetFont(font1);

    std::ostream os(errorList);
    errs.print(os);


    topColSizer->Add(errIcon, 0, 
            wxALIGN_TOP | wxALIGN_CENTER_HORIZONTAL | wxALL, 16);


    topColSizer->Add(rpanel, 0, wxALL | wxEXPAND, 0);

    rowsizer->Add(errorMessage, 0, 
            wxALL, 15);

    rowsizer->Add(errorList, 1, wxEXPAND | wxALL, 10);

    rowsizer->Add(okButton, 0, 
            wxALL | wxALIGN_RIGHT | wxALIGN_BOTTOM, 10);



    panel->SetSizer(topColSizer);
    rpanel->SetSizer(rowsizer);

    // Bind the widgets and event handlers.
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ErrorDialog::onOk, 
            this, okButton->GetId());

    Center();
}

ErrorDialog::~ErrorDialog() {}


void ErrorDialog::onOk(wxCommandEvent& WXUNUSED(pEvent))    {
    EndModal(wxID_OK);
    Destroy();
}
