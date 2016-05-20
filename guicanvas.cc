

#include "guicanvas.h"
#include <GL/glut.h>
#include "names.h"
#include "monitor.h"


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
  zoom = 1.0;
  cyclesdisplayed = -1;
  cycle_no = 0;
}

void MyGLCanvas::Render(wxString example_text, int cycles)
  // Draws canvas contents - the following example writes the string "example text" onto the canvas
  // and draws a signal trace. The trace is artificial if the simulator has not yet been run.
  // When the simulator is run, the number of cycles is passed as a parameter and the first monitor
  // trace is displayed.
{
  float x, y;
  unsigned int i, j, k;
  asignal s;
  name mon_name;
  name mon_name_2;
  wxString mon_name_text;


  if (cycles >= 0) {
    cyclesdisplayed = cycles;
    cycle_no += cycles;
  }

  SetCurrent(*context);
  if (!init) {
    InitGL();
    init = true;
  }
  glClear(GL_COLOR_BUFFER_BIT);

  int w, h;
  GetClientSize(&w, &h);
  float dy = 10.0;              // plot lines at +- dx
  float plot_height = 4 * dy;   // height allocated 2x plot height

  float label_width = 100.0;    // x allowed for labels at start
  float end_width = w - 10;     // dist between end and side
  wxString number;

  // x axis number spacing
  int num_spacing;
  if (cyclesdisplayed > 0) {
    num_spacing = 1 + cyclesdisplayed/20;
  }

  // example_text.Printf("%d", mmz->moncount());

  if ((cyclesdisplayed >= 0) && (mmz->moncount() > 0)) { // draw the first monitor signal, get trace from monitor class

  float dx = (end_width - label_width) / cyclesdisplayed; // dx between points

    for (j = 0; j<mmz->moncount(); j++) {
      // draw x axis
      glColor3f(0.4, 0.4, 0.4);
      glBegin(GL_LINE_STRIP);
      y = h - (j+1)*plot_height - dy;
      glVertex2f(label_width-5, y);
      glVertex2f(end_width, y);
      glEnd();
      for (i=0; i<=cyclesdisplayed; i++) {
        x = dx*i + label_width;
        glBegin(GL_LINE_STRIP);
        glVertex2f(x, y-3);
        glVertex2f(x, y+3);
        glEnd();
        if (i%num_spacing == 0){
          glRasterPos2f(x-4, y-12);
          number = to_string(i+ cycle_no-cyclesdisplayed);
          for (k=0; k<number.Len(); k++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, number[k]);
        }

      }

      // draw trace
      glColor3f(0.0, 1.0, 0.0);
      glBegin(GL_LINE_STRIP);
      for (i=0; i<cyclesdisplayed; i++) {
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
      mmz->getmonname(j, mon_name, mon_name_2);
      mon_name_text = mon_name ->c_str();
      for (i = 0; i < mon_name_text.Len(); i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, mon_name_text[i]);

      if (mon_name_2 != blankname) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '.');

        mon_name_text = mon_name_2->c_str();
        for (i = 0; i < mon_name_text.Len(); i++)
          glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, mon_name_text[i]);
      }
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
    glRasterPos2f(label_width, h/2);
    for (i = 0; i < mon_name_text.Len(); i++)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, mon_name_text[i]);

    string logo[5] = {
      "    _[]_[]_[]_[]_[]_[]_[]_[]_",
      "   |                         |",
      "    )     M A T T L A B      |",
      "   |                         |",
      "    `[]`[]`[]`[]`[]`[]`[]`[]` "
    };
    for (j=0; j<5; j++) {
      glRasterPos2f(label_width, h/2 - 20*(j+2));
      for (i=0; i<logo[j].length(); i++)
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, logo[j][i]);
    }

  }

  // // Example of how to use GLUT to draw text on the canvas
  // glColor3f(1.0, 0.0, 0.0);
  // glRasterPos2f(10, 100);
  // for (i = 0; i < example_text.Len(); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, example_text[i]);

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
  glTranslated(0.0, pan_y, 0.0);
  //glScaled(zoom, zoom, zoom);
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
  if (event.ButtonUp()) text.Printf("Mouse button %d released at %d %d", event.GetButton(), event.m_x, h-event.m_y);
  if (event.Dragging()) {
    // pan_x += event.m_x - last_x;
    // pan_y -= event.m_y - last_y;
    last_x = event.m_x;
    last_y = event.m_y;
    init = false;
    text.Printf("Mouse dragged to %d %d, pan now %d %d", event.m_x, h-event.m_y, pan_x, pan_y);
  }
  if (event.Leaving()) text.Printf("Mouse left window at %d %d", event.m_x, h-event.m_y);
  if (event.GetWheelRotation() < 0) {
    pan_y -= 5*(double)event.GetWheelRotation()/(event.GetWheelDelta());
    init = false;
    text.Printf("Negative mouse wheel rotation, pan_y now %d", pan_y);
  }
  if (event.GetWheelRotation() > 0) {
    pan_y -= 5*(double)event.GetWheelRotation()/(event.GetWheelDelta());
    if (pan_y < 0) pan_y = 0;
    init = false;
    text.Printf("Positive mouse wheel rotation, pan_y now %d", pan_y);
  }

  if (event.GetWheelRotation() || event.ButtonDown() || event.ButtonUp() || event.Dragging() || event.Leaving()) Render(text);
}
