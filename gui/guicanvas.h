
#ifndef GF2_GUICANVAS_H
#define GF2_GUICANVAS_H

#include <wx/wx.h>
#include <wx/glcanvas.h>

#include "../sim/monitor.h"
#include "../com/names.h"


class MyGLCanvas: public wxGLCanvas
{
 public:
  MyGLCanvas(wxWindow *parent, std::vector<int> &order, std::vector<bool> &monitorDisplayed, wxWindowID id = wxID_ANY, monitor* monitor_mod = NULL, names* names_mod = NULL,
         const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0,
         const wxString& name = "MyGLCanvas", const wxPalette &palette=wxNullPalette); // constructor
  void Render(int cycles = -1); // function to draw canvas contents

  void zoomIn(double zoom_amount);
  void zoomOut(double zoom_amount, bool fully=false);
  void colourSelector(int colourInd);
  void resetCycles();
  std::vector<int>& order;
  std::vector<bool>& monitorDisplayed;

void setNetwork(monitor* mmz, names* nmz);
 private:
  wxGLContext *context;              // OpenGL rendering context
  bool init;                         // has the OpenGL context been initialised?
  int pan_x;                         // the current x pan
  int pan_y;                         // the current y pan
  float dx;                          // x spacing
  double zoom;                       // the current zoom
  int cyclesdisplayed;               // how many simulation cycles have been displayed
  int cycles_on_screen;              // number of cycles currently on screen with current zoom
  int zoomrange[2];                  // limits of current zoom
  int cycle_no;                      // total number of cycles run through
  monitor *mmz;                      // pointer to monitor class, used to extract signal traces
  names *nmz;                        // pointer to names class, used to extract signal names

  float trace_RGB[3];                // RGB values for trace lines
  float lines_RGB[3];                // RGB values for axis lines
  float background_RGB[3];           // RGB values for background

  int dy;                            // plot lines at +- dx
  int plot_height;                   // height allocated 2x plot height

  int label_width;                   // x allowed for labels at start
  int end_gap;                       // dist between end and side
  bool on_title;                     // stores whether system state is on title screen
  bool zoom_changed;                 // stores whether the zoom has been changed since previous Render

  void drawText(wxString text, int pos_x, int pos_y, void* font, int line_spacing = 18);
  void titleScreen(wxString message_text);
  void setLineColour(float RGB[3]);
  void drawPlot(asignal s, int plot_num, int mon_num, int zoomrange[2], int cycle_no, int cyclesdisplayed, int num_spacing);


  void InitGL();                     // function to initialise OpenGL context
  void OnSize(wxSizeEvent& event);   // event handler for when canvas is resized
  void OnPaint(wxPaintEvent& event); // event handler for when canvas is exposed
  void OnMouse(wxMouseEvent& event); // event handler for mouse events inside canvas
  DECLARE_EVENT_TABLE()
};


#endif
