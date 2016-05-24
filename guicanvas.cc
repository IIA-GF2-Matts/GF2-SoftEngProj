
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

MyGLCanvas::MyGLCanvas(wxWindow *parent, wxWindowID id, monitor* monitor_mod, names* names_mod, const wxPoint& pos,
               const wxSize& size, long style, const wxString& name, const wxPalette& palette):
  wxGLCanvas(parent, id, wxglcanvas_attrib_list, pos, size, style, name, palette)
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

  // select cool blue colour scheme
  colourSelector(0);

  zoom_changed = false;

}

void MyGLCanvas::setNetwork(monitor* mons, names* nms) {
  mmz = mons;
  nmz = nms;
}

void MyGLCanvas::Render(wxString example_text, int cycles) {
  // Draws canvas contents - the following example writes the string "example text" onto the canvas
  // and draws a signal trace. The trace is artificial if the simulator has not yet been run.
  // When the simulator is run, the number of cycles is passed as a parameter

  // Todo: remove example text after debugging is finished.

  int x, y;
  unsigned int j, k;
  asignal s;
  name mon_name_dev;
  name mon_name_pin;
  wxString mon_name_text;


  if (cycles >= 0) {
    cyclesdisplayed = cycles;
    cycle_no = cycles; // cycle_no follows actual number of cycles when array vector is full and cycles displayed const.
    // Todo: implement for limit on data vector
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


  if ((cyclesdisplayed >= 0) && (mmz->moncount() > 0)) {
    on_title = false;
    if ((int)(cyclesdisplayed/zoom) == 0)
      cycles_on_screen = 1;
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

    // stop from panning over end of screen.
    if (zoomrange[0] + cycles_on_screen > cyclesdisplayed)
      zoomrange[0] = cyclesdisplayed - cycles_on_screen;
    zoomrange[1] = zoomrange[0] + cycles_on_screen;

    // x axis number spacing
    int num_spacing = (1 + cycles_on_screen/20) * to_string(cycle_no).length()/2;


    // draw each plot
    for (j = 0; j<mmz->moncount(); j++) {
      drawPlot(s, j, zoomrange, cycle_no, cyclesdisplayed, num_spacing);
    }

    // draw vertical line
    setLineColour(lines_RGB);
    glBegin(GL_LINE_STRIP);
    glVertex2f(label_width-5, h-dy);
    glVertex2f(label_width-5, h-(2*dy + plot_height*mmz->moncount()));
    glEnd();


  } else { // draw title screen
    titleScreen();
  }

  // todo: remove after debgging
  // Draw example text
  glColor3f(1.0, 0.0, 0.0);
  drawText(example_text, 10, 100, GLUT_BITMAP_HELVETICA_12);
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
  y = h - (plot_num+1)*plot_height - dy;
  glVertex2f(label_width-5, y);
  glVertex2f(end_width, y);
  glEnd();
  for (i=zoomrange[0]; i<=zoomrange[1]; i++) {
    x = dx*(i-zoomrange[0]) + label_width;
    glBegin(GL_LINE_STRIP);
    glVertex2f(x, y-3);
    glVertex2f(x, y+3);
    glEnd();
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
    if (mmz->getsignaltrace(plot_num, i, s)) {
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

  glRasterPos2f(10, h - 5 - (plot_num+1)*plot_height);
  mmz->getmonname(plot_num, mon_name_dev, mon_name_pin);
  mon_name_text = mon_name_dev ->c_str();
  if (mon_name_pin != blankname) {
    mon_name_text += ".";
    mon_name_text += mon_name_pin ->c_str();
  }
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
  // glScaled(zoom, 1, 1);
}

void MyGLCanvas::OnPaint(wxPaintEvent& event)
  // Event handler for when the canvas is exposed
{
  int w, h;
  wxString text;

  wxPaintDC dc(this); // required for correct refreshing under MS windows
  GetClientSize(&w, &h);
  text.Printf("Canvas redrawn by OnPaint event handler, canvas size is %d by %d", w, h);
  Render(text);
}

void MyGLCanvas::OnSize(wxSizeEvent& event)
  // Event handler for when the canvas is resized
{
  init = false;; // this will force the viewport and projection matrices to be reconfigured on the next paint
}

void MyGLCanvas::OnMouse(wxMouseEvent& event)
  // Event handler for mouse events inside the GL canvas
{
  wxString text;
  int w, h;;
  static int last_x, last_y;

  GetClientSize(&w, &h);
  if (event.ButtonDown()) {
    last_x = event.m_x;
    last_y = event.m_y;
    text.Printf("Mouse button %d pressed at %d %d", event.GetButton(), event.m_x, h-event.m_y);
  }
  if (event.ButtonUp()) {
    text.Printf("Mouse button %d released at %d %d", event.GetButton(), event.m_x, h-event.m_y);

  }
  if (event.Dragging()) {
    if (!on_title){
      // x panning
      pan_x += last_x - event.m_x;
      // check for scrolling off left
      if (pan_x < 0) pan_x = 0;
      // check for scrolling off right
      int max_pan = (cyclesdisplayed - cycles_on_screen)*dx + dx;
      // + dx ensures that it can reach last value in all situations.
      // second check is performed when setting zoomrange. Both these tests combined ensure that the pan value cannot get
      // too high and that can always reach the last value and not get further.
      if (pan_x > max_pan)       
        pan_x = max_pan;
      text.Printf("dx is %f, max_pan is %d, pan_x is %d", dx, max_pan, pan_x);

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
  }

  if (event.Leaving()) text.Printf("Mouse left window at %d %d", event.m_x, h-event.m_y);

  if (event.GetWheelRotation() < 0) {
    zoom = zoom * (1.0 - (double)event.GetWheelRotation()/(20*event.GetWheelDelta()));
    if (zoom > cyclesdisplayed/2) zoom = cyclesdisplayed/2; // Max zoom 
    text.Printf("Negative mouse wheel rotation, zoom now %f", zoom);
  }
  if (event.GetWheelRotation() > 0) {
    zoom = zoom / (1.0 + (double)event.GetWheelRotation()/(20*event.GetWheelDelta()));
    if (zoom < 1) zoom = 1;     // Don't allow zoom out from full trace.
    text.Printf("Positive mouse wheel rotation, zoom now %f", zoom);
  }
  if (event.GetWheelRotation()) {
    init = false;
    zoom_changed = true;
  }

  if (event.GetWheelRotation() || event.ButtonDown() || event.ButtonUp() || event.Dragging() || event.Leaving()) Render(text);
}

void MyGLCanvas::titleScreen(){
  // Todo: redesign title screen: add getting started message etc.
  on_title = true;
  int w, h;
  GetClientSize(&w, &h);

  wxString title_text = "Welcome to MattLab Logic Simulator";
  setLineColour(trace_RGB);
  drawText(title_text, label_width/2, h-plot_height, GLUT_BITMAP_HELVETICA_18);

  wxString logo =
    " _[]_[]_[]_[]_[]_[]_[]_[]_  \n"
    "|                         | \n"
    " )     M A T T L A B      | \n"
    "|                         | \n"
    " `[]`[]`[]`[]`[]`[]`[]`[]`  ";
  drawText(logo, label_width/2, h-2*plot_height, GLUT_BITMAP_9_BY_15);
}



void MyGLCanvas::drawText(wxString text, int pos_x, int pos_y, void* font) {
  glRasterPos2f(pos_x, pos_y);
  for (int k=0; k<text.Len(); k++){
    if (text[k] == '\n'){
      pos_y -= 18;                  // set up for title screen logo
      glRasterPos2f(pos_x, pos_y);
    }
    else
      glutBitmapCharacter(font, text[k]);
  }
}

void MyGLCanvas::setLineColour(float RGB[3]) {
  glColor3f(RGB[0], RGB[1], RGB[2]);
}

void MyGLCanvas::colourSelector(int colourInd) {
  float traceColours[3][3] = {
    {0.32, 0.55, 0.87},       // Cool blue
    {0.00, 0.90, 0.00},       // Retro Green
    {0.00, 0.00, 0.00}        // Simple B+W
  };
  float axisColours[3][3] = {
    {0.58, 0.59, 0.60},       // Cool blue
    {0.40, 0.40, 0.40},       // Retro Green
    {0.50, 0.50, 0.50}        // Simple B+W
  };
  float backColours[3][3] = {
    {0.21, 0.21, 0.21},       // Cool blue
    {0.15, 0.15, 0.15},       // Retro Green
    {1.00, 1.00, 1.00}        // Simple B+W
  };
  int i;
  for (i=0; i<3; i++) trace_RGB[i] = traceColours[colourInd][i];
  for (i=0; i<3; i++) lines_RGB[i] = axisColours[colourInd][i];
  for (i=0; i<3; i++) background_RGB[i] = backColours[colourInd][i];
  init = false;
}