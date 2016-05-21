

#ifndef GF2_GUICANVAS_H
#define GF2_GUICANVAS_H

#include <wx/wx.h>
#include <wx/glcanvas.h>

#include "monitor.h"
#include "names.h"


class MyGLCanvas: public wxGLCanvas
{
 public:
  MyGLCanvas(wxWindow *parent, wxWindowID id = wxID_ANY, monitor* monitor_mod = NULL, names* names_mod = NULL,
         const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0,
         const wxString& name = "MyGLCanvas", const wxPalette &palette=wxNullPalette); // constructor
  void Render(wxString example_text = "", int cycles = -1); // function to draw canvas contents

  void drawText(wxString text, int pos_x, int pos_y, void* font);
  
void setNetwork(monitor* mmz, names* nmz);
 private:
  wxGLContext *context;              // OpenGL rendering context
  bool init;                         // has the OpenGL context been initialised?
  int pan_x;                         // the current x pan
  int pan_y;                         // the current y pan
  int selection_x[2];                // the currently selected range of values for zoom.
  bool zoomed;                       // keeps track of whether canvas is zoomed.
  float dx;
  double zoom;                       // the current zoom
  int cyclesdisplayed;               // how many simulation cycles have been displayed
  int cycle_no;
  monitor *mmz;                      // pointer to monitor class, used to extract signal traces
  names *nmz;                        // pointer to names class, used to extract signal names
  void InitGL();                     // function to initialise OpenGL context
  void OnSize(wxSizeEvent& event);   // event handler for when canvas is resized
  void OnPaint(wxPaintEvent& event); // event handler for when canvas is exposed
  void OnMouse(wxMouseEvent& event); // event handler for mouse events inside canvas
  DECLARE_EVENT_TABLE()
};


#endif
