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
  virtual void internal_hide(void);

public:
  Toolbarmenu(Toolbar &);
  ~Toolbarmenu(void);

  inline Basemenu *getPlacementmenu(void) { return placementmenu; }

  void reconfigure(void);
};


class Toolbar : public TimeoutHandler {
private:
  Bool on_top, editing, hidden, do_auto_hide, do_hide;
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

    virtual void timeout(void);
  } hide_handler;

  Openbox &openbox;
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
  Toolbar(BScreen &);
  virtual ~Toolbar(void);

  inline Toolbarmenu *getMenu(void) { return toolbarmenu; }

  inline const Bool &isEditing(void) const { return editing; }
  inline const Bool &isOnTop(void) const { return on_top; }
  inline const Bool &isHidden(void) const { return hidden; }
  inline const Bool &doAutoHide(void) const { return do_auto_hide; }

  inline const Window &getWindowID(void) const { return frame.window; }

  inline const unsigned int &getWidth(void) const { return frame.width; }
  inline const unsigned int &getHeight(void) const { return frame.height; }
  inline const unsigned int getExposedHeight(void) const {
    if (do_hide) return 0;
    else if (do_auto_hide) return frame.bevel_w;
    else return frame.height;
  }
  
  inline const int &getX(void) const
  { return ((hidden) ? frame.x_hidden : frame.x); }
  //  const int getY(void) const;
  inline const int getY(void) const { 
    if (do_hide) return screen.size().h();
    else if (hidden) return frame.y_hidden;
    else return frame.y;
  }

  
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
  void edit(void);
  void reconfigure(void);
  void mapToolbar(void);
  void unMapToolbar(void);
#ifdef    HAVE_STRFTIME
  void checkClock(Bool = False);
#else //  HAVE_STRFTIME
  void checkClock(Bool = False, Bool = False);
#endif // HAVE_STRFTIME

  virtual void timeout(void);

  enum { TopLeft = 1, BottomLeft, TopCenter,
         BottomCenter, TopRight, BottomRight };
};


#endif // __Toolbar_hh
