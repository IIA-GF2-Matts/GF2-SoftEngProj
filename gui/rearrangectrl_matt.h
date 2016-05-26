///////////////////////////////////////////////////////////////////////////////
// Name:        wx/rearrangectrl.h
// Purpose:     various controls for rearranging the items interactively
// Author:      Vadim Zeitlin
// Created:     2008-12-15
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////


// This is a modified version for use with GF2 that allows dynamic adding of
// items.


#ifndef _WX_REARRANGECTRLMATT_H_
#define _WX_REARRANGECTRLMATT_H_

#include <wx/checklst.h>

// #if wxUSE_REARRANGECTRL

#include <wx/panel.h>
#include <wx/dialog.h>

#include <wx/arrstr.h>

extern WXDLLIMPEXP_DATA_CORE(const char) wxRearrangeListMattNameStr[];
extern WXDLLIMPEXP_DATA_CORE(const char) wxRearrangeDialogMattNameStr[];

// ----------------------------------------------------------------------------
// wxRearrangeListMatt: a (check) list box allowing to move items around
// ----------------------------------------------------------------------------

// This class works allows to change the order of the items shown in it as well
// as to check or uncheck them individually. The data structure used to allow
// this is the order array which contains the items indices indexed by their
// position with an added twist that the unchecked items are represented by the
// bitwise complement of the corresponding index (for any architecture using
// two's complement for negative numbers representation (i.e. just about any at
// all) this means that a checked item N is represented by -N-1 in unchecked
// state).
//
// So, for example, the array order [1 -3 0] used in conjunction with the items
// array ["first", "second", "third"] means that the items are displayed in the
// order "second", "third", "first" and the "third" item is unchecked while the
// other two are checked.
class WXDLLIMPEXP_CORE wxRearrangeListMatt : public wxCheckListBox
{
public:
    // ctors and such
    // --------------

    // default ctor, call Create() later
    wxRearrangeListMatt() { }

    // ctor creating the control, the arguments are the same as for
    // wxCheckListBox except for the extra order array which defines the
    // (initial) display order of the items as well as their statuses, see the
    // description above
    wxRearrangeListMatt(wxWindow *parent,
                    wxWindowID id,
                    const wxPoint& pos,
                    const wxSize& size,
                    const wxArrayInt& order,
                    const wxArrayString& items,
                    long style = 0,
                    const wxValidator& validator = wxDefaultValidator,
                    const wxString& name = wxRearrangeListMattNameStr)
    {
        Create(parent, id, pos, size, order, items, style, validator, name);
    }

    // Create() function takes the same parameters as the base class one and
    // the order array determining the initial display order
    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos,
                const wxSize& size,
                const wxArrayInt& order,
                const wxArrayString& items,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxRearrangeListMattNameStr);


    // items order
    // -----------

    // get the current items order; the returned array uses the same convention
    // as the one passed to the ctor
    const wxArrayInt& GetCurrentOrder() const { return m_order; }

    // return true if the current item can be moved up or down (i.e. just that
    // it's not the first or the last one)
    bool CanMoveCurrentUp() const;
    bool CanMoveCurrentDown() const;

    // move the current item one position up or down, return true if it was moved
    // or false if the current item was the first/last one and so nothing was done
    bool MoveCurrentUp();
    bool MoveCurrentDown();

    bool Reset(const wxArrayInt& order, const wxArrayString& items);

private:
    // swap two items at the given positions in the listbox
    void Swap(int pos1, int pos2);

    wxArrayString OrderList(const wxArrayInt& order,
                            const wxArrayString& items);

    // the current order array
    wxArrayInt m_order;


    DECLARE_EVENT_TABLE()
    wxDECLARE_NO_COPY_CLASS(wxRearrangeListMatt);
};

#endif // _WX_REARRANGECTRLMATT_H_

