// -*- mode: C++; indent-tabs-mode: nil; -*-
// Slit.hh for Blackbox - an X11 Window manager
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

#ifndef   __Slit_hh
#define   __Slit_hh

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

#include <list>

#include "Screen.hh"
#include "Basemenu.hh"

// forward declaration
class Slit;
class Slitmenu;

class Slitmenu : public Basemenu {
private:
  class Directionmenu : public Basemenu {
  private:
    Directionmenu(const Directionmenu&);
    Directionmenu& operator=(const Directionmenu&);

  protected:
    virtual void itemSelected(int button, unsigned int index);

  public:
    Directionmenu(Slitmenu *sm);
  };

  class Placementmenu : public Basemenu {
  private:
    Placementmenu(const Placementmenu&);
    Placementmenu& operator=(const Placementmenu&);

  protected:
    virtual void itemSelected(int buton, unsigned int index);

  public:
    Placementmenu(Slitmenu *sm);
  };

  Directionmenu *directionmenu;
  Placementmenu *placementmenu;

  Slit *slit;

  friend class Directionmenu;
  friend class Placementmenu;
  friend class Slit;

  Slitmenu(const Slitmenu&);
  Slitmenu& operator=(const Slitmenu&);

protected:
  virtual void itemSelected(int button, unsigned int index);
  virtual void internal_hide(void);

public:
  Slitmenu(Slit *sl);
  virtual ~Slitmenu(void);

  inline Basemenu *getDirectionmenu(void) { return directionmenu; }
  inline Basemenu *getPlacementmenu(void) { return placementmenu; }

  void reconfigure(void);
};


class Slit : public TimeoutHandler {
public:
  struct SlitClient {
    Window window, client_window, icon_window;

    Rect rect;
  };

private:
  typedef std::list<SlitClient*> SlitClientList;

  bool on_top, hidden, do_auto_hide;
  Display *display;

  Blackbox *blackbox;
  BScreen *screen;
  BTimer *timer;
  Strut strut;

  SlitClientList clientList;
  Slitmenu *slitmenu;

  struct SlitFrame {
    Pixmap pixmap;
    Window window;

    int x_hidden, y_hidden;
    Rect rect;
  } frame;

  void updateStrut(void);

  friend class Slitmenu;
  friend class Slitmenu::Directionmenu;
  friend class Slitmenu::Placementmenu;

  Slit(const Slit&);
  Slit& operator=(const Slit&);

public:
  Slit(BScreen *scr);
  virtual ~Slit(void);

  inline bool isOnTop(void) const { return on_top; }
  inline bool isHidden(void) const { return hidden; }
  inline bool doAutoHide(void) const { return do_auto_hide; }

  inline Slitmenu *getMenu(void) { return slitmenu; }

  inline Window getWindowID(void) const { return frame.window; }

  inline int getX(void) const
  { return ((hidden) ? frame.x_hidden : frame.rect.x()); }
  inline int getY(void) const
  { return ((hidden) ? frame.y_hidden : frame.rect.y()); }

  inline unsigned int getWidth(void) const { return frame.rect.width(); }
  inline unsigned int getExposedWidth(void) const {
    if (screen->getSlitDirection() == Vertical && do_auto_hide)
      return screen->getBevelWidth();
    return frame.rect.width();
  }
  inline unsigned int getHeight(void) const { return frame.rect.height(); }
  inline unsigned int getExposedHeight(void) const {
    if (screen->getSlitDirection() == Horizontal && do_auto_hide)
      return screen->getBevelWidth();
    return frame.rect.height();
  }

  void addClient(Window w);
  void removeClient(SlitClient *client, bool remap = True);
  void removeClient(Window w, bool remap = True);
  void reconfigure(void);
  void updateSlit(void);
  void reposition(void);
  void shutdown(void);
  void toggleAutoHide(void);

  void buttonPressEvent(XButtonEvent *e);
  void enterNotifyEvent(XCrossingEvent * /*unused*/);
  void leaveNotifyEvent(XCrossingEvent * /*unused*/);
  void configureRequestEvent(XConfigureRequestEvent *e);
  void unmapNotifyEvent(XUnmapEvent *e);

  virtual void timeout(void);

  enum { Vertical = 1, Horizontal };
  enum { TopLeft = 1, CenterLeft, BottomLeft, TopCenter, BottomCenter,
         TopRight, CenterRight, BottomRight };
};


#endif // __Slit_hh
