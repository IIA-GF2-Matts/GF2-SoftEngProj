
#ifndef GF2_GUIERRORDIALOG_H
#define GF2_GUIERRORDIALOG_H

#include <string>
#include <wx/wx.h>
#include "../com/errorhandler.h"


class ErrorDialog: public wxDialog {
public:
    ErrorDialog(wxWindow* parent, wxWindowID id, const errorcollector& errc);
    virtual ~ErrorDialog();
    void onOk(wxCommandEvent& pEvent);

private:
    const errorcollector& errs;
    bool process_ = false;
    wxTextCtrl* nameTextCtrl_;
};


#endif /* GF2_GUIERRORDIALOG_H */
