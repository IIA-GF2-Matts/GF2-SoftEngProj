

///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/rearrangectrl.cpp
// Purpose:     implementation of classes in wx/rearrangectrl.h
// Author:      Vadim Zeitlin
// Created:     2008-12-15
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// Modified version to allow dynamic adding of items.

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// for compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include <wx/button.h>
    #include <wx/stattext.h>
    #include <wx/sizer.h>
#endif // WX_PRECOMP

#include "rearrangectrl_matt.h"

// ============================================================================
// wxRearrangeListMatt implementation
// ============================================================================

extern
WXDLLIMPEXP_DATA_CORE(const char) wxRearrangeListMattNameStr[] = "wxRearrangeListMatt";

BEGIN_EVENT_TABLE(wxRearrangeListMatt, wxCheckListBox)
    EVT_CHECKLISTBOX(wxID_ANY, wxRearrangeListMatt::OnCheck)
END_EVENT_TABLE()

bool wxRearrangeListMatt::Create(wxWindow *parent,
                             wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& size,
                             const wxArrayInt& order,
                             const wxArrayString& items,
                             long style,
                             const wxValidator& validator,
                             const wxString& name)
{
    // construct the array of items in the order in which they should appear in
    // the control
    const size_t count = items.size();
    wxArrayString itemsInOrder = OrderList(order, items);

    // do create the real control
    if ( !wxCheckListBox::Create(parent, id, pos, size, itemsInOrder,
                                 style, validator, name) ) {
        return false;
    }

    // and now check all the items which should be initially checked
    for ( int n = 0; n < count; n++ ) {
        Check(n, true);
    }

    m_order = order;

    return true;
}

wxArrayString wxRearrangeListMatt::OrderList(const wxArrayInt& order, const wxArrayString& items) {
    const size_t count = items.size();

    wxArrayString itemsInOrder;
    itemsInOrder.reserve(count);
    size_t n;
    for ( n = 0; n < count; n++ )
    {
        int idx = order[n];
        itemsInOrder.push_back(items[idx]);
    }

    return itemsInOrder;
}

bool wxRearrangeListMatt::Reset(const wxArrayInt& order, const wxArrayString& items) {
    const size_t count = items.size();
    wxArrayString itemsInOrder = OrderList(order, items);

	Clear();
	InsertItems(itemsInOrder, 0);
	m_order = order;
}

bool wxRearrangeListMatt::CanMoveCurrentUp() const
{
    const int sel = GetSelection();
    return sel != wxNOT_FOUND && sel != 0;
}

bool wxRearrangeListMatt::CanMoveCurrentDown() const
{
    const int sel = GetSelection();
    return sel != wxNOT_FOUND && static_cast<unsigned>(sel) != GetCount() - 1;
}

bool wxRearrangeListMatt::MoveCurrentUp()
{
    const int sel = GetSelection();
    if ( sel == wxNOT_FOUND || sel == 0 )
        return false;

    Swap(sel, sel - 1);
    SetSelection(sel - 1);

    return true;
}

bool wxRearrangeListMatt::MoveCurrentDown()
{
    const int sel = GetSelection();
    if ( sel == wxNOT_FOUND || static_cast<unsigned>(sel) == GetCount() - 1 )
        return false;

    Swap(sel, sel + 1);
    SetSelection(sel + 1);

    return true;
}

void wxRearrangeListMatt::Swap(int pos1, int pos2)
{
    // update the internally stored order
    wxSwap(m_order[pos1], m_order[pos2]);


    // and now also swap all the attributes of the items

    // first the label
    const wxString stringTmp = GetString(pos1);
    SetString(pos1, GetString(pos2));
    SetString(pos2, stringTmp);

    // then the checked state
    const bool checkedTmp = IsChecked(pos1);
    Check(pos1, IsChecked(pos2));
    Check(pos2, checkedTmp);

    // and finally the client data, if necessary
    switch ( GetClientDataType() )
    {
        case wxClientData_None:
            // nothing to do
            break;

        case wxClientData_Object:
            {
                wxClientData * const dataTmp = DetachClientObject(pos1);
                SetClientObject(pos1, DetachClientObject(pos2));
                SetClientObject(pos2, dataTmp);
            }
            break;

        case wxClientData_Void:
            {
                void * const dataTmp = GetClientData(pos1);
                SetClientData(pos1, GetClientData(pos2));
                SetClientData(pos2, dataTmp);
            }
            break;
    }
}

void wxRearrangeListMatt::OnCheck(wxCommandEvent& event)
{
    // update the internal state to match the new item state
    const int n = event.GetInt();

    // m_order[n] = ~m_order[n];
}
