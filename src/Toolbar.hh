// -*- mode: C++; indent-tabs-mode: nil; -*-
// Toolbar.hh for Blackbox - an X11 Window manager
// Copyright (c) 2001 - 2002 Sean 'Shaleh' Perry <shaleh@debian.org>
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

extern "C" {
#include <X11/Xlib.h>
}

#include "Screen.hh"
#include "Basemenu.hh"
#include "Timer.hh"

// forward declaration
class Toolbar;

class Toolbarmenu : public Basemenu {
private:
  class Placementmenu : public Basemenu {
  private:
    Placementmenu(const Placementmenu&);
    Placementmenu& operator=(const Placementmenu&);
    Toolbar *toolbar;

  protected:
    virtual void itemSelected(int button, unsigned int index);
    virtual void setValues(void);

  public:
    Placementmenu(Toolbarmenu *tm);
    virtual void reconfigure(void);
  };

  Toolbar *toolbar;
  Placementmenu *placementmenu;

  friend class Placementmenu;
  friend class Toolbar;

  Toolbarmenu(const Toolbarmenu&);
  Toolbarmenu& operator=(const Toolbarmenu&);

protected:
  virtual void itemSelected(int button, unsigned int index);
  virtual void internal_hide(void);
  virtual void setValues(void);

public:
  Toolbarmenu(Toolbar *tb);
  ~Toolbarmenu(void);

  inline Basemenu *getPlacementmenu(void) { return placementmenu; }

  virtual void reconfigure(void);
};


class Toolbar : public TimeoutHandler {
private:
  bool on_top, editing, hidden, do_auto_hide;
  unsigned int width_percent;
  int placement;
  std::string toolbarstr;
  Display *display;

  struct ToolbarFrame {
    unsigned long button_pixel, pbutton_pixel;
    Pixmap base, label, wlabel, clk, button, pbutton;
    Window window, workspace_label, window_label, clock, psbutton, nsbutton,
      pwbutton, nwbutton;

    int x_hidden, y_hidden, hour, minute, grab_x, grab_y;
    unsigned int window_label_w, workspace_label_w, clock_w,
      button_w, bevel_w, label_h;

    Rect rect;
  } frame;

  class HideHandler : public TimeoutHandler {
  public:
    Toolbar *toolbar;

    virtual void timeout(void);
  } hide_handler;

  Blackbox *blackbox;
  BScreen *screen;
  Configuration *config;
  BTimer *clock_timer, *hide_timer;
  Toolbarmenu *toolbarmenu;
  Strut strut;

  std::string new_workspace_name;
  size_t new_name_pos;

  friend class HideHandler;
  friend class Toolbarmenu;
  friend class Toolbarmenu::Placementmenu;

  void redrawPrevWorkspaceButton(bool pressed = False, bool redraw = False);
  void redrawNextWorkspaceButton(bool pressed = False, bool redraw = False);
  void redrawPrevWindowButton(bool preseed = False, bool redraw = False);
  void redrawNextWindowButton(bool preseed = False, bool redraw = False);

  void updateStrut(void);

#ifdef    HAVE_STRFTIME
  void checkClock(bool redraw = False);
#else //  HAVE_STRFTIME
  void checkClock(bool redraw = False, bool date = False);
#endif // HAVE_STRFTIME

  Toolbar(const Toolbar&);
  Toolbar& operator=(const Toolbar&);

public:
  Toolbar(BScreen *scrn);
  virtual ~Toolbar(void);

  inline Toolbarmenu *getMenu(void) { return toolbarmenu; }

  inline bool isEditing(void) const { return editing; }
  inline bool isOnTop(void) const { return on_top; }
  inline bool isHidden(void) const { return hidden; }
  inline bool doAutoHide(void) const { return do_auto_hide; }
  inline unsigned int getWidthPercent(void) const { return width_percent; }
  inline int getPlacement(void) const { return placement; }
 
  void saveOnTop(bool);
  void saveAutoHide(bool);
  void saveWidthPercent(unsigned int);
  void savePlacement(int);

  void save_rc(void);
  void load_rc(void);

  void mapToolbar(void);
  void unmapToolbar(void);

  inline Window getWindowID(void) const { return frame.window; }

  inline const Rect& getRect(void) const { return frame.rect; }
  inline unsigned int getWidth(void) const { return frame.rect.width(); }
  inline unsigned int getHeight(void) const { return frame.rect.height(); }
  inline unsigned int getExposedHeight(void) const
  { return (screen->doHideToolbar() ? 0 :
            ((do_auto_hide) ? frame.bevel_w :
             frame.rect.height())); }
  inline int getX(void) const
  { return ((hidden) ? frame.x_hidden : frame.rect.x()); }
  inline int getY(void) const
  { return ((hidden) ? frame.y_hidden : frame.rect.y()); }

  void buttonPressEvent(const XButtonEvent *be);
  void buttonReleaseEvent(const XButtonEvent *re);
  void enterNotifyEvent(const XCrossingEvent * /*unused*/);
  void leaveNotifyEvent(const XCrossingEvent * /*unused*/);
  void exposeEvent(const XExposeEvent *ee);
  void keyPressEvent(const XKeyEvent *ke);

  void edit(void);
  void reconfigure(void);
  void toggleAutoHide(void);

  void redrawWindowLabel(bool redraw = False);
  void redrawWorkspaceLabel(bool redraw = False);

  virtual void timeout(void);

  enum { TopLeft = 1, BottomLeft, TopCenter,
         BottomCenter, TopRight, BottomRight };
};


#endif // __Toolbar_hh
