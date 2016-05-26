
#include "guicanvas.h"
#include <GL/glut.h>
#include "names.h"
#include "monitor.h"
#include <iostream>

// MyGLCanvas ////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MyGLCanvas, wxGLCanvas)
  EVT_SIZE(MyGLCanvas::OnSize)
  EVT_PAINT(MyGLCanvas::OnPaint)
  EVT_MOUSE_EVENTS(MyGLCanvas::OnMouse)
END_EVENT_TABLE()

int wxglcanvas_attrib_list[5] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

MyGLCanvas::MyGLCanvas(wxWindow *parent, std::vector<int> &ord, wxWindowID id, monitor* monitor_mod, names* names_mod, const wxPoint& pos,
               const wxSize& size, long style, const wxString& name, const wxPalette& palette):
  wxGLCanvas(parent, id, wxglcanvas_attrib_list, pos, size, style, name, palette), order(ord)
  // Constructor - initialises private variables
{
  context = new wxGLContext(this);
  mmz = monitor_mod;
  nmz = names_mod;
  init = false;
  pan_x = 0;
  pan_y = 0;
  zoom = 1;
  cyclesdisplayed = -1;
  cycle_no = 0;

  dy = 12;              // plot lines at +- dx
  plot_height = 4 * dy;   // height allocated 2x plot height
  label_width = 100.0;    // x allowed for labels at start
  end_gap = 10;           // dist between end and side



  // select "Cool Blue" colour scheme
  colourSelector(0);

  zoom_changed = false;

}

void MyGLCanvas::setNetwork(monitor* mons, names* nms) {
  mmz = mons;
  nmz = nms;
}

void MyGLCanvas::Render(int cycles) {
  // Main function for drawing to the GUI GL Canvas
  // cycles is the number of cycles run since last refresh, now independant from mmz->cycles()

  unsigned int j;
  asignal s;

  if (cycles >= 0) {
    cyclesdisplayed = mmz->cycles();
    cycle_no += cycles; // cycle_no follows actual number of cycles when array vector is full and cycles displayed const.
  }

  SetCurrent(*context);
  if (!init) {
    InitGL();
    init = true;
  }
  glClear(GL_COLOR_BUFFER_BIT);

  int w, h;
  GetClientSize(&w, &h);
  int end_width = w - end_gap;

  // if any cycles have been run and at least one monitor display plots else display title screen.
  if ((cyclesdisplayed >= 0) && (mmz->moncount() > 0)) {
    // Assert to ensure that order is correct.
    wxASSERT_MSG((mmz->moncount() >= order.size()), "Number of monitors displayed more than number of existing monitors, this shouldn't happen.");

    on_title = false;           // enables zoom and pan controls.
    if ((int)(cyclesdisplayed/zoom) == 0)
      cycles_on_screen = 1;     // fix for floating point error with 1 cycle, high zoom.
    else
      cycles_on_screen = cyclesdisplayed / zoom;
    dx = (float)(end_width - label_width) / cycles_on_screen; // dx between points

    // if zoomed then scale pan_x based on last zoomrange, else set zoomrange based on pan_x
    if (zoom_changed) {
      pan_x = zoomrange[0] * dx;
      zoom_changed = false;
    }
    else 
      zoomrange[0] = pan_x / dx;

    // stop from panning over either end of screen.
    if (zoomrange[0] < 0)
      zoomrange[0] = 0;
    else if (zoomrange[0] + cycles_on_screen > cyclesdisplayed)
      zoomrange[0] = cyclesdisplayed - cycles_on_screen;
    zoomrange[1] = zoomrange[0] + cycles_on_screen;

    // x axis number spacing
    int num_spacing = (1 + cycles_on_screen/20) * std::ceil(to_string(cycle_no).length()/2.0);

    // draw each plot
    for (j = 0; j<order.size(); j++) {
      drawPlot(s, j, zoomrange, cycle_no, cyclesdisplayed, num_spacing);
    }

    // draw vertical line
    setLineColour(lines_RGB);
    glBegin(GL_LINE_STRIP);
    glVertex2f(label_width-5, h-dy);
    glVertex2f(label_width-5, h-(2*dy + plot_height*mmz->moncount()));
    glEnd();


  } else if ((cyclesdisplayed >= 0)) {
    on_title = true;
    pan_y = 0;
    titleScreen("No monitors selected for simulator. \n"
      "Select or add a monitor to view output.");
  }


  else { // draw title screen
    on_title = true;
    pan_y = 0;
    titleScreen("Select 'file' -> 'open' to begin...");
  }

  // We've been drawing to the back buffer, flush the graphics pipeline and swap the back buffer to the front
  glFlush();
  SwapBuffers();
}


void MyGLCanvas::drawPlot(asignal s, int plot_num, int zoomrange[2], int cycle_no, int cyclesdisplayed, int num_spacing){
  
  int w, h;
  GetClientSize(&w, &h);
  int end_width = w - end_gap;
  int x, y;
  int i;
  
  // draw x axis
  setLineColour(lines_RGB);
  glBegin(GL_LINE_STRIP);
  // set axis height
  y = h - (plot_num+1)*plot_height - dy;
  // draw x axis line
  glVertex2f(label_width-5, y);
  glVertex2f(end_width, y);
  glEnd();
  for (i=zoomrange[0]; i<=zoomrange[1]; i++) {
    x = dx*(i-zoomrange[0]) + label_width;
    // draw axis ticks
    glBegin(GL_LINE_STRIP);
    glVertex2f(x, y-3);
    glVertex2f(x, y+3);
    glEnd();
    // draw axis numbers
    if (i%num_spacing == 0){
      drawText(to_string(i+cycle_no-cyclesdisplayed), x-4, y-12, GLUT_BITMAP_HELVETICA_10);
    }

  }
  
  // draw trace
  int signal_y;
  setLineColour(trace_RGB);
  glLineWidth(2.0);
  glBegin(GL_LINE_STRIP);
  for (i=zoomrange[0]; i<zoomrange[1]; i++) {
    if (mmz->getsignaltrace(order[plot_num], i, s)) {
      if (s==low) signal_y = y;
      if (s==high) signal_y = y + 2*dy;
      glVertex2f(dx*(i-zoomrange[0]) + label_width, signal_y);
      glVertex2f(dx*(i-zoomrange[0]+1) + label_width, signal_y);
    }
  }
  glEnd();
  glLineWidth(1.0);

  // draw text label
  name mon_name_dev, mon_name_pin;
  wxString mon_name_text;
  // get monitor name indices
  mmz->getmonname(order[plot_num], mon_name_dev, mon_name_pin);
  // get name text and combine with "."
  mon_name_text = mon_name_dev ->c_str();
  if (mon_name_pin != blankname) {
    mon_name_text += ".";
    mon_name_text += mon_name_pin ->c_str();
  }
  // draw name
  drawText(mon_name_text, 10, h-5-(plot_num+1)*plot_height, GLUT_BITMAP_HELVETICA_12);
  
}


void MyGLCanvas::InitGL()
  // Function to initialise the GL context
{
  int w, h;

  GetClientSize(&w, &h);
  SetCurrent(*context);
  glDrawBuffer(GL_BACK);
  glClearColor(background_RGB[0], background_RGB[1], background_RGB[2], 0);
  glViewport(0, 0, (GLint) w, (GLint) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, w, 0, h, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslated(0.0, pan_y, 0.0);
}

void MyGLCanvas::OnPaint(wxPaintEvent& event)
  // Event handler for when the canvas is exposed
{
  int w, h;
  wxString text;

  wxPaintDC dc(this); // required for correct refreshing under MS windows
  GetClientSize(&w, &h);
  Render();
}

void MyGLCanvas::OnSize(wxSizeEvent& event)
  // Event handler for when the canvas is resized
{
  init = false;; // this will force the viewport and projection matrices to be reconfigured on the next paint
}

void MyGLCanvas::OnMouse(wxMouseEvent& event)
  // Event handler for mouse events inside the GL canvas
{
  // title screen disables mouse controls
  if (on_title) return;

  int w, h;;
  static int last_x, last_y;

  GetClientSize(&w, &h);
  if (event.ButtonDown()) {
    last_x = event.m_x;
    last_y = event.m_y;
  }
  if (event.Dragging()) {
    // x panning
    pan_x += last_x - event.m_x;
    // check for scrolling off left
    if (pan_x < 0) pan_x = 0;
    // check for scrolling off right
    int max_pan = (cyclesdisplayed - cycles_on_screen)*dx + dx;
    // + dx ensures that it can reach last value in all situations.
    // second check is performed when setting zoomrange in Render. Both these tests combined
    // ensure that the pan value cannot get too high and that can always reach the last value 
    // and not get further.
    if (pan_x > max_pan)       
      pan_x = max_pan;

    // y panning
    pan_y -= event.m_y - last_y;
    // check for scrolling off bottom
    if (pan_y > (mmz->moncount()+1)*plot_height - h) pan_y = (mmz->moncount()+1)*plot_height - h;
    // check for scrolling off top
    if (pan_y < 0) pan_y = 0;

    last_x = event.m_x;
    last_y = event.m_y;
    init = false;
  }

  if (event.GetWheelRotation() < 0) {
    zoomIn((double)event.GetWheelRotation()/(20*event.GetWheelDelta()));
  }
  if (event.GetWheelRotation() > 0) {
    zoomOut((double)event.GetWheelRotation()/(20*event.GetWheelDelta()));
  }
  if (event.GetWheelRotation()) {
    init = false;
    zoom_changed = true;
  }

  if (event.GetWheelRotation() || event.ButtonDown() || event.ButtonUp() || event.Dragging() || event.Leaving()) Render();
}

// Function to zoom in. Note that zoom in requires a negative value.
void MyGLCanvas::zoomIn(double zoom_amount){
  wxASSERT_MSG((zoom_amount < 0), "Input to zoomIn must be a negative amount.");
  zoom = zoom * (1 - zoom_amount);
  if (zoom > cyclesdisplayed/2) zoom = cyclesdisplayed/2; // Max zoom
  Render();
}

// Function to zoom out. This requires a positive value. Fully is true to zoom out completely (defaults to false)
void MyGLCanvas::zoomOut(double zoom_amount, bool fully){
  wxASSERT_MSG((zoom_amount >= 0), "Input to zoomOut must be a positive value.");
  double new_zoom = zoom / (1.0 + zoom_amount);
  if (fully || zoom < 1)
    zoom = 1;
  else 
    zoom = new_zoom;
  Render();
}

void MyGLCanvas::titleScreen(wxString message_text){
  
  on_title = true;
  int w, h;
  GetClientSize(&w, &h);

  setLineColour(trace_RGB);

  // Draw title
  wxString title_text = "Welcome to MattLab Logic Simulator";
  drawText(title_text, label_width/2, h-plot_height, GLUT_BITMAP_HELVETICA_18);

  // Draw logo
  wxString logo =
    " _[]_[]_[]_[]_[]_[]_[]_[]_  \n"
    "|                         | \n"
    " )     M A T T L A B      | \n"
    "|                         | \n"
    " `[]`[]`[]`[]`[]`[]`[]`[]`  ";
  drawText(logo, label_width/2, h-2*plot_height, GLUT_BITMAP_9_BY_15);

  // Draw message
  drawText(message_text, label_width/2, 80, GLUT_BITMAP_HELVETICA_12);
}


// Draw text to screen. line_spacing for new line characters defaults to 18.
void MyGLCanvas::drawText(wxString text, int pos_x, int pos_y, void* font, int line_spacing) {
  glRasterPos2f(pos_x, pos_y);
  for (int k=0; k<text.Len(); k++){
    if (text[k] == '\n'){
      pos_y -= line_spacing;
      glRasterPos2f(pos_x, pos_y);
    }
    else
      glutBitmapCharacter(font, text[k]);
  }
}

void MyGLCanvas::setLineColour(float RGB[3]) {
  glColor3f(RGB[0], RGB[1], RGB[2]);
}

// allows colour scheme to be selected. Currently a bit of a pain to add new colours and not
// the most efficient (could use pointers) but likely to be rarely changed.
void MyGLCanvas::colourSelector(int colourInd) {
  float traceColours[4][3] = {
    {0.32, 0.55, 0.87},       // Cool blue
    {0.00, 0.90, 0.00},       // Retro Green
    {0.00, 0.00, 0.00},       // Simple B+W
    {1.00, 1.00, 0.00}        // Candy Pink
  };
  float axisColours[4][3] = {
    {0.58, 0.59, 0.60},       // Cool blue
    {0.40, 0.40, 0.40},       // Retro Green
    {0.50, 0.50, 0.50},       // Simple B+W
    {1.00, 1.00, 1.00}        // Candy Pink
  };
  float backColours[4][3] = {
    {0.21, 0.21, 0.21},       // Cool blue
    {0.15, 0.15, 0.15},       // Retro Green
    {1.00, 1.00, 1.00},       // Simple B+W
    {1.00, 0.20, 0.56}        // Candy Pink
  };
  int i;
  for (i=0; i<3; i++) trace_RGB[i] = traceColours[colourInd][i];
  for (i=0; i<3; i++) lines_RGB[i] = axisColours[colourInd][i];
  for (i=0; i<3; i++) background_RGB[i] = backColours[colourInd][i];

  init = false;
}

void MyGLCanvas::resetCycles() {
  cycle_no = 0;
}