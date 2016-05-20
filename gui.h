#ifndef gui_h
#define gui_h

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include "names.h"
#include "devices.h"
#include "monitor.h"

#include "guicanvas.h"

enum {
  MY_SPINCNTRL_ID = wxID_HIGHEST + 1,
  MY_TEXTCTRL_ID,
  MY_BUTTON_ID,
}; // widget identifiers

class MyFrame: public wxFrame
{
 public:
  MyFrame(wxWindow *parent, const wxString& title, const wxPoint& pos, const wxSize& size,
	  names *names_mod = NULL, devices *devices_mod = NULL, monitor *monitor_mod = NULL,
	  long style = wxDEFAULT_FRAME_STYLE); // constructor
 private:
  MyGLCanvas *canvas;                     // OpenGL drawing area widget to draw traces
  wxSpinCtrl *spin;                       // control widget to select the number of cycles
  names *nmz;                             // pointer to names class
  devices *dmz;                           // pointer to devices class
  monitor *mmz;                           // pointer to monitor class
  int cyclescompleted;                    // how many simulation cycles have been completed
  void runnetwork(int ncycles);           // function to run the logic network
  void OnExit(wxCommandEvent& event);     // event handler for exit menu item
  void OnAbout(wxCommandEvent& event);    // event handler for about menu item
  void OnButton(wxCommandEvent& event);   // event handler for push button
  void OnSpin(wxSpinEvent& event);        // event handler for spin control
  void OnText(wxCommandEvent& event);     // event handler for text entry field
  DECLARE_EVENT_TABLE()
};

#endif /* gui_h */
