
#ifndef GF2_GUIERRORDIALOG_H
#define GF2_GUIERRORDIALOG_H

#include <wx/wx.h>
#include <string>
#include "errorhandler.h"


class ErrorDialog: public wxDialog {
public:
    ErrorDialog(wxWindow* parent, wxWindowID id, const errorcollector& errc);
    virtual ~ErrorDialog();
    void onCancel(wxCommandEvent& pEvent);
    void onOk(wxCommandEvent& pEvent);
    void onCheck(wxCommandEvent& pEvent);

private:
    const errorcollector& errs;
    bool process_ = false;
    wxTextCtrl* nameTextCtrl_;
};

#endif /* GF2_GUIERRORDIALOG_H */
