// Toolbar.h for Openbox
// Copyright (c) 2001 Sean 'Shaleh' Perry <shaleh@debian.org>
// Copyright (c) 1997 - 2000 Brad Hughes (bhughes@tcac.net)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef   __Toolbar_hh
#define   __Toolbar_hh

#include <X11/Xlib.h>

#include "Basemenu.h"
#include "Timer.h"
#include "Resource.h"
#include "Screen.h"

// forward declaration
class Toolbar;

class Toolbarmenu : public Basemenu {
private:
  class Placementmenu : public Basemenu {
  private:
    Toolbarmenu &toolbarmenu;

  protected:
    virtual void itemSelected(int, int);

  public:
    Placementmenu(Toolbarmenu &);
  };

  Toolbar &toolbar;
  Placementmenu *placementmenu;

  friend class Placementmenu;
  friend class Toolbar;


protected:
  virtual void itemSelected(int, int);
  virtual void internal_hide();

public:
  Toolbarmenu(Toolbar &);
  ~Toolbarmenu();

  inline Basemenu *getPlacementmenu() { return placementmenu; }

  void reconfigure();
};


class Toolbar : public TimeoutHandler {
private:
  bool m_ontop, m_editing, m_hidden, m_autohide;
  int m_width_percent, m_placement;
  Display *display;

  struct frame {
    unsigned long button_pixel, pbutton_pixel;
    Pixmap base, label, wlabel, clk, button, pbutton;
    Window window, workspace_label, window_label, clock, psbutton, nsbutton,
      pwbutton, nwbutton;

    int x, y, x_hidden, y_hidden, hour, minute, grab_x, grab_y;
    unsigned int width, height, window_label_w, workspace_label_w, clock_w,
      button_w, bevel_w, label_h;
  } frame;

  class HideHandler : public TimeoutHandler {
  public:
    Toolbar *toolbar;
    virtual void timeout();
  } hide_handler;

  Openbox &openbox;
  Resource &config;
  BImageControl *image_ctrl;
  BScreen &screen;
  BTimer *clock_timer, *hide_timer;
  Toolbarmenu *toolbarmenu;

  char *new_workspace_name;
  size_t new_name_pos;

  friend class HideHandler;
  friend class Toolbarmenu;
  friend class Toolbarmenu::Placementmenu;


public:
  Toolbar(BScreen &, Resource &);
  virtual ~Toolbar();

  inline Toolbarmenu *getMenu() { return toolbarmenu; }

  inline const Window &getWindowID() const { return frame.window; }

  inline unsigned int getWidth() const { return frame.width; }
  inline unsigned int getHeight() const { return frame.height; }
  unsigned int getExposedHeight() const;
  
  int getX() const;
  int getY() const;
  
  void buttonPressEvent(XButtonEvent *);
  void buttonReleaseEvent(XButtonEvent *);
  void enterNotifyEvent(XCrossingEvent *);
  void leaveNotifyEvent(XCrossingEvent *);
  void exposeEvent(XExposeEvent *);
  void keyPressEvent(XKeyEvent *);

  void redrawWindowLabel(Bool = False);
  void redrawWorkspaceLabel(Bool = False);
  void redrawPrevWorkspaceButton(Bool = False, Bool = False);
  void redrawNextWorkspaceButton(Bool = False, Bool = False);
  void redrawPrevWindowButton(Bool = False, Bool = False);
  void redrawNextWindowButton(Bool = False, Bool = False);
  void edit();
  void reconfigure();
  void load();
  void mapToolbar();
  void unMapToolbar();
#ifdef    HAVE_STRFTIME
  void checkClock(Bool = False);
#else //  HAVE_STRFTIME
  void checkClock(Bool = False, Bool = False);
#endif // HAVE_STRFTIME

  virtual void timeout();

  inline bool onTop() const { return m_ontop; }
  void setOnTop(bool);

  inline bool autoHide() const { return m_autohide; }
  void setAutoHide(bool);
  
  inline int widthPercent() const { return m_width_percent; }
  void setWidthPercent(int);
  
  inline int placement() const { return m_placement; }
  void setPlacement(int);

  inline bool isEditing() const { return m_editing; }
  inline bool isHidden() const { return m_hidden; }
  
  enum { TopLeft = 1, BottomLeft, TopCenter,
         BottomCenter, TopRight, BottomRight };
};


#endif // __Toolbar_hh
