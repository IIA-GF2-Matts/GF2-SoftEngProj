#ifndef gui_h
#define gui_h

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/checklst.h>
#include <wx/arrstr.h>
#include <wx/statbox.h>
// #include <wx/arrint.h>
#include <vector>

#include "../com/names.h"
#include "../sim/devices.h"
#include "../sim/monitor.h"

#include "rearrangectrl_matt.h"
#include "guicanvas.h"


#define ID_FILEOPEN 2001
#define ID_ADDMONITOR 2002


enum {
  MY_SPINCNTRL_ID = wxID_HIGHEST + 1,
  MY_TEXTCTRL_ID,
  MY_RUN_BUTTON_ID,
  MY_CONTINUE_BUTTON_ID,
  MY_ZOOM_RESET_ID,
  MY_SWITCH_LIST_ID,
  MY_MONITOR_LIST_ID,
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
  wxButton *continuebutton;
  wxButton *btnAdd;
  wxButton *btnUp;
  wxButton *btnDown;
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

  // Menu bar items which need to be kept to be enabled disabled...
  wxMenuItem *addMonitorMenuBar;

  std::vector<devlink> switches;
  std::vector<outputsignal> signals;
  std::vector<bool> monitored;
  std::vector<bool> monitorDisplayed;
  std::vector<int> monitorOrder;

  bool runnetwork(int ncycles);           // function to run the logic network
  void OnExit(wxCommandEvent& event);     // event handler for exit menu item
  void OnAbout(wxCommandEvent& event);    // event handler for about menu item
  void OnOpen(wxCommandEvent& event);     // Event handler for file->Open
  void OnRunButton(wxCommandEvent& event);   // event handler for run button
  void OnContinueButton(wxCommandEvent &event);   // event handler for continue button
  void OnSpin(wxSpinEvent& event);        // event handler for spin control
  void OnText(wxCommandEvent& event);     // event handler for text entry field

  void OnZoomIn(wxCommandEvent& event);     // event handler for zooming in
  void OnZoomOut(wxCommandEvent& event);     // event handler for zooming out
  void OnZoomReset(wxCommandEvent& event);     // event handler for resetting zooming

  void OnSwitchListEvent(wxCommandEvent& event);     // event handler for (un)checking switch list items
  void OnMonitorListEvent(wxCommandEvent& event);     // event handler for (un)checking monitor display list items
  void OnAddMonitor(wxCommandEvent& event);     // Event handler for Add monitor button
  void OnMonitorUp(wxCommandEvent& event);
  void OnMonitorDown(wxCommandEvent& event);
  void RefreshMonitors();

  void toggleButtonsEnabled(bool enabled);

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
