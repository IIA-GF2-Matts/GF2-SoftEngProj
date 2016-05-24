
#ifndef GF2_GUIMONITORDIALOG_H
#define GF2_GUIMONITORDIALOG_H

#include <wx/wx.h>
#include <string>

#include "network.h"
#include "names.h"


class MonitorDialog: public wxDialog {
public:
    MonitorDialog(wxWindow* parent, wxWindowID id, names* nmz, 
    	const std::vector<outputsignal>& signals, std::vector<bool>& monitored);
    virtual ~MonitorDialog();
    void onOk(wxCommandEvent& pEvent);

    wxCheckListBox* switchlist;

private:
	names* _nmz;
	const std::vector<outputsignal>& _signals;
	std::vector<bool>& _monitor;


    bool process_ = false;
};

#endif /* GF2_GUIMONITORDIALOG_H */