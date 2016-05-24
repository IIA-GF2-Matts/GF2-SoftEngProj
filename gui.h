#ifndef gui_h
#define gui_h

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/checklst.h>
//#include <wx/rearrangectrl.h>
#include "rearrangectrl_matt.h"
#include <wx/arrstr.h>
// #include <wx/arrint.h>
#include <vector>

#include "names.h"
#include "devices.h"
#include "monitor.h"

#include "guicanvas.h"


#define ID_FILEOPEN 2001
#define ID_ADDMONITOR 2002


enum {
  MY_SPINCNTRL_ID = wxID_HIGHEST + 1,
  MY_TEXTCTRL_ID,
  MY_RUN_BUTTON_ID,
  MY_ZOOM_RESET_ID,
  MY_SWITCH_LIST_ID,

  BLUE_ID,
  GREEN_ID,
  BW_ID,
  PINK_ID
}; // widget identifiers

class MyFrame: public wxFrame
{
 public:
  MyFrame(wxWindow *parent, const wxPoint& pos, const wxSize& size,
	  long style = wxDEFAULT_FRAME_STYLE); // constructor
  
  void openFile(wxString fname);          // Opens a .matt file
 private:
  MyGLCanvas *canvas;                     // OpenGL drawing area widget to draw traces
  wxSpinCtrl *spin;                       // control widget to select the number of cycles
  wxBoxSizer* controls_sizer;
  wxButton *runbutton;
  wxCheckListBox *switchlist;             // widget to turn swicthes on/off
  wxRearrangeListMatt *monitorlist;       // widget to switch monitors
  names *nmz;                             // pointer to names class
  devices *dmz;                           // pointer to devices class
  monitor *mmz;                           // pointer to monitor class
  network *netz;
  int cyclescompleted;                    // how many simulation cycles have been completed
  bool hasNetwork;
  bool fileOpen;
  wxString fname;

  std::vector<devlink> switches;
  std::vector<outputsignal> signals;
  std::vector<bool> monitored;
  std::vector<int> monitorOrder;

  void runnetwork(int ncycles);           // function to run the logic network
  void OnExit(wxCommandEvent& event);     // event handler for exit menu item
  void OnAbout(wxCommandEvent& event);    // event handler for about menu item
  void OnOpen(wxCommandEvent& event);     // Event handler for file->Open
  void OnAddMonitor(wxCommandEvent& event);     // Event handler for Add monitor button
  void OnButton(wxCommandEvent& event);   // event handler for push button
  void OnSpin(wxSpinEvent& event);        // event handler for spin control
  void OnText(wxCommandEvent& event);     // event handler for text entry field
  
  void OnZoomIn(wxCommandEvent& event);     // event handler for zooming in
  void OnZoomOut(wxCommandEvent& event);     // event handler for zooming out
  void OnZoomReset(wxCommandEvent& event);     // event handler for resetting zooming

  void OnSwitchListEvent(wxCommandEvent& event);     // event handler for (un)checking switch list items

  void colourChange(int index);
  void OnColourBlue(wxCommandEvent& event);
  void OnColourGreen(wxCommandEvent& event);
  void OnColourBW(wxCommandEvent& event);
  void OnColourPink(wxCommandEvent& event);

  void initNetwork();                     // Initialises network elements
  void delNetwork();                      // Clears network elements
  void closeFile();                       // Closes the file.
  void updateTitle();                     // Updates the title of the frame

  DECLARE_EVENT_TABLE()
};

#endif /* gui_h */
