#ifndef gui_h
#define gui_h

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/checklst.h>
#include <wx/rearrangectrl.h>
#include <wx/arrstr.h>
// #include <wx/arrint.h>
#include "names.h"
#include "devices.h"
#include "monitor.h"

#include "guicanvas.h"


#define ID_FILEOPEN 2001


enum {
  MY_SPINCNTRL_ID = wxID_HIGHEST + 1,
  MY_TEXTCTRL_ID,
  MY_BUTTON_ID,
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
  wxButton *runbutton;
  wxCheckListBox *switchlist;             // widget to turn swicthes on/off
  wxRearrangeCtrl *monitorlist;           // widget to switch monitors
  names *nmz;                             // pointer to names class
  devices *dmz;                           // pointer to devices class
  monitor *mmz;                           // pointer to monitor class
  network *netz;
  int cyclescompleted;                    // how many simulation cycles have been completed
  bool hasNetwork;
  bool fileOpen;
  wxString fname;
  void runnetwork(int ncycles);           // function to run the logic network
  void OnExit(wxCommandEvent& event);     // event handler for exit menu item
  void OnAbout(wxCommandEvent& event);    // event handler for about menu item
  void OnOpen(wxCommandEvent& event);     // Event handler for file->Open
  void OnButton(wxCommandEvent& event);   // event handler for push button
  void OnSpin(wxSpinEvent& event);        // event handler for spin control
  void OnText(wxCommandEvent& event);     // event handler for text entry field

  void initNetwork();                     // Initialises network elements
  void delNetwork();                      // Clears network elements
  void closeFile();                       // Closes the file.
  void updateTitle();                     // Updates the title of the frame

  DECLARE_EVENT_TABLE()
};

#endif /* gui_h */
