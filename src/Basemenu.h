// Basemenu.h for Openbox
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

#ifndef   __Basemenu_hh
#define   __Basemenu_hh

#include <X11/Xlib.h>

class Openbox;
class BImageControl;
class BScreen;
class Basemenu;
class BasemenuItem;
#include <vector>
typedef std::vector<BasemenuItem *> menuitemList;

class Basemenu {
private:
  menuitemList menuitems;
  Openbox &openbox;
  Basemenu *parent;
  BImageControl *image_ctrl;
  BScreen &screen;

  Bool moving, visible, movable, torn, internal_menu, title_vis, shifted,
    hide_tree;
  Display *display;
  int which_sub, which_press, which_sbl, alignment;

  struct _menu {
    Pixmap frame_pixmap, title_pixmap, hilite_pixmap, sel_pixmap;
    Window window, frame, title;

    char *label;
    int x, y, x_move, y_move, x_shift, y_shift, sublevels, persub, minsub,
      grab_x, grab_y;
    unsigned int width, height, title_h, frame_h, item_w, item_h, bevel_w,
      bevel_h;
  } menu;


protected:
  inline void setTitleVisibility(Bool b) { title_vis = b; }
  inline void setMovable(Bool b) { movable = b; }
  inline void setHideTree(Bool h) { hide_tree = h; }
  inline void setMinimumSublevels(int m) { menu.minsub = m; }

  virtual void itemSelected(int, int) = 0;
  virtual void drawItem(int, Bool = False, Bool = False,
			int = -1, int = -1, unsigned int = 0,
			unsigned int = 0);
  virtual void redrawTitle();
  virtual void internal_hide(void);


public:
  Basemenu(BScreen &);
  virtual ~Basemenu(void);

  inline const Bool &isTorn(void) const { return torn; }
  inline const Bool &isVisible(void) const { return visible; }

  inline BScreen &getScreen(void) { return screen; }

  inline const Window &getWindowID(void) const { return menu.window; }

  inline const char *getLabel(void) const { return menu.label; }

  int insert(const char *, int = 0, const char * = (const char *) 0, int = -1);
  int insert(const char **, int = -1, int = 0);
  int insert(const char *, Basemenu *, int = -1);
  int remove(int);

  inline int getX(void) const { return menu.x; }
  inline int getY(void) const { return menu.y; }
  inline unsigned int getCount(void) { return menuitems.size(); }
  inline int getCurrentSubmenu(void) const { return which_sub; }
  inline BasemenuItem *find(int index) { return menuitems[index]; }

  inline unsigned int getWidth(void) const { return menu.width; }
  inline unsigned int getHeight(void) const { return menu.height; }
  inline unsigned int getTitleHeight(void) const { return menu.title_h; }

  inline void setInternalMenu(void) { internal_menu = True; }
  inline void setAlignment(int a) { alignment = a; }
  inline void setTorn(void) { torn = True; }
  inline void removeParent(void)
    { if (internal_menu) parent = (Basemenu *) 0; }

  bool hasSubmenu(int);
  bool isItemSelected(int);
  bool isItemEnabled(int);

  void buttonPressEvent(XButtonEvent *);
  void buttonReleaseEvent(XButtonEvent *);
  void motionNotifyEvent(XMotionEvent *);
  void enterNotifyEvent(XCrossingEvent *);
  void leaveNotifyEvent(XCrossingEvent *);
  void exposeEvent(XExposeEvent *);
  void reconfigure(void);
  void setLabel(const char *n);
  void move(int, int);
  void update(void);
  void setItemSelected(int, bool);
  void setItemEnabled(int, bool);

  virtual void drawSubmenu(int);
  virtual void show(void);
  virtual void hide(void);

  enum { AlignDontCare = 1, AlignTop, AlignBottom };
  enum { Right = 1, Left };
  enum { Empty = 0, Square, Triangle, Diamond };
};


class BasemenuItem {
private:
  Basemenu *s;
  const char **u, *l, *e;
  int f, enabled, selected;

  friend class Basemenu;

protected:

public:
  BasemenuItem(const char *lp, int fp, const char *ep = (const char *) 0):
    s(0), u(0), l(lp), e(ep), f(fp), enabled(1), selected(0) {}

  BasemenuItem(const char *lp, Basemenu *mp): s(mp), u(0), l(lp), e(0), f(0),
					      enabled(1), selected(0) {}

  BasemenuItem(const char **up, int fp): s(0), u(up), l(0), e(0), f(fp),
					 enabled(1), selected(0) {}

  inline const char *exec(void) const { return e; }
  inline const char *label(void) const { return l; }
  inline const char **ulabel(void) const { return u; }
  inline const int &function(void) const { return f; }
  inline Basemenu *submenu(void) { return s; }

  inline const int &isEnabled(void) const { return enabled; }
  inline void setEnabled(int e) { enabled = e; }
  inline const int &isSelected(void) const { return selected; }
  inline void setSelected(int s) { selected = s; }
};


#endif // __Basemenu_hh
