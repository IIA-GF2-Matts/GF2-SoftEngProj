

// Todo: generally tidy up code, it's a mess of initial stuff and many changes.

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
  cyclesdisplayed = -1;
  cycle_no = 0;
}

void MyGLCanvas::setNetwork(monitor* mons, names* nms) {
  mmz = mons;
  nmz = nms;
}

void MyGLCanvas::Render(wxString example_text, int cycles) {
  // Draws canvas contents - the following example writes the string "example text" onto the canvas
  // and draws a signal trace. The trace is artificial if the simulator has not yet been run.
  // When the simulator is run, the number of cycles is passed as a parameter

  float x, y;
  unsigned int i, j, k;
  asignal s;
  name mon_name_dev;
  name mon_name_pin;
  wxString mon_name_text;
  int zoomrange[2];


  if (cycles >= 0) {
    cyclesdisplayed = cycles;
    cycle_no += cycles;
    zoomed = false;
  }

  SetCurrent(*context);
  if (!init) {
    InitGL();
    init = true;
  }
  glClear(GL_COLOR_BUFFER_BIT);

  int w, h;
  GetClientSize(&w, &h);
  float dy = 12.0;              // plot lines at +- dx
  float plot_height = 4 * dy;   // height allocated 2x plot height

  float label_width = 100.0;    // x allowed for labels at start
  float end_width = w - 10;     // dist between end and side
  wxString number;

  // x axis number spacing
  int num_spacing;
  if (cyclesdisplayed > 0) {
    num_spacing = 1 + cyclesdisplayed/20;
  }

  if ((cyclesdisplayed >= 0) && (mmz->moncount() > 0)) { // draw the first monitor signal, get trace from monitor class
    // Todo: this doesn't fix floating point error when changing from 0 to 1 cycles. Or don't allow an input of 0.
    if (cyclesdisplayed == 0)
      dx = 20;
    else //if (!zoomed)
      dx = (end_width - label_width) / cyclesdisplayed; // dx between points

    // Find start and end points of zoom
    if (zoomed) {
      zoomrange[0] = (selection_x[0]-label_width) / dx;
      if (zoomrange[0] < 0) zoomrange[0] = 0;
      zoomrange[1] = (selection_x[1]-label_width) / dx + 1;
      if (zoomrange[1] > cyclesdisplayed) zoomrange[1] = cyclesdisplayed;
      // recalculate dx
      dx = (end_width - label_width) / (zoomrange[1] - zoomrange[0]); 
    }
    else {
      zoomrange[0] = 0;
      zoomrange[1] = cyclesdisplayed;
    }


    for (j = 0; j<mmz->moncount(); j++) {
      // draw x axis
      glColor3f(0.4, 0.4, 0.4);
      glBegin(GL_LINE_STRIP);
      y = h - (j+1)*plot_height - dy;
      glVertex2f(label_width-5, y);
      glVertex2f(end_width, y);
      glEnd();
      for (i=zoomrange[0]; i<=zoomrange[1]; i++) {
        x = dx*i + label_width;
        glBegin(GL_LINE_STRIP);
        glVertex2f(x, y-3);
        glVertex2f(x, y+3);
        glEnd();
        if (i%num_spacing == 0){
          drawText(to_string(i+cycle_no-cyclesdisplayed), x-4, y-12, GLUT_BITMAP_HELVETICA_10);
        }

      }

      // draw trace
      glColor3f(0.0, 1.0, 0.0);
      glBegin(GL_LINE_STRIP);
      for (i=zoomrange[0]; i<zoomrange[1]; i++) {
        if (mmz->getsignaltrace(j, i, s)) {
          if (s==low) y = h - ((j+1) * plot_height + dy);
          if (s==high) y = h - ((j+1) * plot_height - dy);
          glVertex2f(dx*i + label_width, y);
          glVertex2f(dx*(i+1) + label_width, y);
        }
      }
      glEnd();
      // draw text label
      glRasterPos2f(10, h - 5 - (j+1)*plot_height);
      mmz->getmonname(j, mon_name_dev, mon_name_pin);
      mon_name_text = mon_name_dev ->c_str();
      if (mon_name_pin != blankname) {
        mon_name_text += ".";
        mon_name_text += mon_name_pin ->c_str();
      }
      drawText(mon_name_text, 10, h-5-(j+1)*plot_height, GLUT_BITMAP_HELVETICA_12);
      
    }

    // draw verticle line
    glColor3f(0.4, 0.4, 0.4);
    glBegin(GL_LINE_STRIP);
    glVertex2f(label_width-5, h-dy);
    glVertex2f(label_width-5, h-(2*dy + plot_height*mmz->moncount()));
    glEnd();


  } else { // draw title screen

    glBegin(GL_LINE_STRIP);
    for (i=0; i<10; i++) {
      if (i%2) y = h/2 + 40 - dy;
      else y = h/2 + 40 + dy;
      glVertex2f(20*i+label_width, y);
      glVertex2f(20*i+20.0 + label_width, y);
    }
    glEnd();
    mon_name_text = "Welcome to MattLab Logic Simulator";
    glColor3f(0.0, 1.0, 0.0);
    drawText(mon_name_text, w/4, h/2, GLUT_BITMAP_HELVETICA_18);

    string logo =
      "    _[]_[]_[]_[]_[]_[]_[]_[]_  \n"
      "   |                         | \n"
      "    )     M A T T L A B      | \n"
      "   |                         | \n"
      "    `[]`[]`[]`[]`[]`[]`[]`[]`  ";
    drawText(logo, w/4, h/2-40, GLUT_BITMAP_9_BY_15);
    // }

  }

  // Draw example text
  glColor3f(1.0, 0.0, 0.0);
  drawText(example_text, 10, 100, GLUT_BITMAP_HELVETICA_12);
  // We've been drawing to the back buffer, flush the graphics pipeline and swap the back buffer to the front
  glFlush();
  SwapBuffers();
}

void MyGLCanvas::InitGL()
  // Function to initialise the GL context
{
  int w, h;

  GetClientSize(&w, &h);
  SetCurrent(*context);
  glDrawBuffer(GL_BACK);
  glClearColor(0.15, 0.15, 0.15, 0.0);
  glViewport(0, 0, (GLint) w, (GLint) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, w, 0, h, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslated(pan_x, pan_y, 0.0);
  glScaled(zoom, zoom, zoom);
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
  // Todo: Resizing squashes x axis.
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
    // if (last_x > event.m_x) {
    //   selection_x[0] = event.m_x;
    //   selection_x[1] = last_x;
    // }
    // else {
    //   selection_x[0] = last_x;
    //   selection_x[1] = event.m_x;
    // }
    // zoomed = true;

  }
  if (event.Dragging()) {

    pan_x += event.m_x - last_x;
    // Todo: implement limits on pan_x
    pan_y -= event.m_y - last_y;
    if (pan_y < 0) pan_y = 0;
    // Todo: implement max pan_y
    last_x = event.m_x;
    last_y = event.m_y;
    init = false;
    // text.Printf("Mouse dragged from %d to %d", selection_x[0], selection_x[1]);
  }

  if (event.Leaving()) text.Printf("Mouse left window at %d %d", event.m_x, h-event.m_y);

  if (event.GetWheelRotation() < 0) {
    zoom = zoom * (1.0 - (double)event.GetWheelRotation()/(20*event.GetWheelDelta()));
    init = false;
    text.Printf("Negative mouse wheel rotation, zoom now %f", zoom);
  }
  if (event.GetWheelRotation() > 0) {
    zoom = zoom / (1.0 + (double)event.GetWheelRotation()/(20*event.GetWheelDelta()));
    init = false;
    text.Printf("Positive mouse wheel rotation, zoom now %f", zoom);
    
  // Now panning by dragging again so this bit is redundant...
  //   pan_y -= 5*(double)event.GetWheelRotation()/(event.GetWheelDelta());
  //   init = false;
  //   text.Printf("Negative mouse wheel rotation, pan_y now %d", pan_y);
  // }
  // if (event.GetWheelRotation() > 0) {
  //   pan_y -= 5*(double)event.GetWheelRotation()/(event.GetWheelDelta());
  //   if (pan_y < 0) pan_y = 0;
  //   init = false;
  //   text.Printf("Positive mouse wheel rotation, pan_y now %d", pan_y);
  // }

  if (event.GetWheelRotation() || event.ButtonDown() || event.ButtonUp() || event.Dragging() || event.Leaving()) Render(text);
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