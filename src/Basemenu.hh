// -*- mode: C++; indent-tabs-mode: nil; -*-
// Basemenu.hh for Blackbox - an X11 Window manager
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

#ifndef   __Basemenu_hh
#define   __Basemenu_hh

extern "C" {
#include <X11/Xlib.h>
}

#include <string>
#include <deque>

class Blackbox;
class BImageControl;
class BScreen;
class Basemenu;
class BasemenuItem;


class Basemenu {
private:
  typedef std::deque<BasemenuItem*> MenuItems;
  MenuItems menuitems;
  Blackbox *blackbox;
  Basemenu *parent;
  BImageControl *image_ctrl;
  BScreen *screen;

  bool moving, visible, movable, torn, internal_menu, title_vis, shifted,
    hide_tree;
  Display *display;
  int which_sub, which_press, which_sbl, alignment;

  struct _menu {
    Pixmap frame_pixmap, title_pixmap, hilite_pixmap, sel_pixmap;
    Window window, frame, title;

    std::string label;
    int x, y, x_move, y_move, x_shift, y_shift, sublevels, persub, minsub,
      grab_x, grab_y;
    unsigned int width, height, title_h, frame_h, item_w, item_h, bevel_w,
      bevel_h;
  } menu;

  Basemenu(const Basemenu&);
  Basemenu& operator=(const Basemenu&);

protected:
  BasemenuItem *find(int index);
  inline void setTitleVisibility(bool b) { title_vis = b; }
  inline void setMovable(bool b) { movable = b; }
  inline void setHideTree(bool h) { hide_tree = h; }
  inline void setMinimumSublevels(int m) { menu.minsub = m; }

  virtual void itemSelected(int button, unsigned int index) = 0;
  virtual void drawItem(int index, bool highlight = False, bool clear = False,
                        int x = -1, int y = -1,
                        unsigned int w = 0, unsigned int h = 0);
  virtual void redrawTitle(void);
  virtual void internal_hide(void);


public:
  Basemenu(BScreen *scrn);
  virtual ~Basemenu(void);

  inline bool isTorn(void) const { return torn; }
  inline bool isVisible(void) const { return visible; }

  inline BScreen *getScreen(void) { return screen; }

  inline Window getWindowID(void) const { return menu.window; }

  inline const char *getLabel(void) const { return menu.label.c_str(); }

  int insert(BasemenuItem *item, int pos);
  int insert(const std::string& label, int function = 0,
             const std::string& exec = "", int pos = -1);
  int insert(const std::string &label, Basemenu *submenu, int pos = -1);
  int remove(int index);

  void changeItemLabel(unsigned int index, const std::string& label);

  inline int getX(void) const { return menu.x; }
  inline int getY(void) const { return menu.y; }
  inline unsigned int getCount(void) { return menuitems.size(); }
  inline int getCurrentSubmenu(void) const { return which_sub; }

  inline unsigned int getWidth(void) const { return menu.width; }
  inline unsigned int getHeight(void) const { return menu.height; }
  inline unsigned int getTitleHeight(void) const
  { return menu.title_h; }

  inline void setInternalMenu(void) { internal_menu = True; }
  inline void setAlignment(int a) { alignment = a; }
  inline void setTorn(void) { torn = True; }
  inline void removeParent(void)
  { if (internal_menu) parent = (Basemenu *) 0; }

  bool hasSubmenu(int index);
  bool isItemSelected(int index);
  bool isItemEnabled(int index);

  void buttonPressEvent(XButtonEvent *be);
  void buttonReleaseEvent(XButtonEvent *be);
  void motionNotifyEvent(XMotionEvent *me);
  void enterNotifyEvent(XCrossingEvent *ce);
  void leaveNotifyEvent(XCrossingEvent *ce);
  void exposeEvent(XExposeEvent *ee);
  void reconfigure(void);
  void setLabel(const std::string& label);
  void move(int x, int y);
  void update(void);
  void setItemSelected(int index, bool sel);
  void setItemEnabled(int index, bool enable);

  virtual void drawSubmenu(int index);
  virtual void show(void);
  virtual void hide(void);

  enum { AlignDontCare = 1, AlignTop, AlignBottom };
  enum { Right = 1, Left };
  enum { Empty = 0, Square, Triangle, Diamond };
};


class BasemenuItem {
private:
  Basemenu *sub;
  std::string l, e;
  int f, enabled, selected;

  friend class Basemenu;

protected:

public:
  BasemenuItem(const std::string& lp, int fp = 0, const std::string& ep = ""):
    sub(0), l(lp), e(ep), f(fp), enabled(1), selected(0) {}

  BasemenuItem(const std::string& lp, Basemenu *mp): sub(mp), l(lp),
                                                     f(0), enabled(1),
                                                     selected(0) {}

  ~BasemenuItem(void);

  inline const char *exec(void) const { return e.c_str(); }
  inline const char *label(void) const { return l.c_str(); }
  inline int function(void) const { return f; }
  inline Basemenu *submenu(void) { return sub; }

  inline void newLabel(const std::string& label) { l = label; }

  inline int isEnabled(void) const { return enabled; }
  inline void setEnabled(int e) { enabled = e; }
  inline int isSelected(void) const { return selected; }
  inline void setSelected(int s) { selected = s; }
};


#endif // __Basemenu_hh
