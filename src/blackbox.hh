// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// blackbox.hh for Blackbox - an X11 Window manager
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

#ifndef   __blackbox_hh
#define   __blackbox_hh

extern "C" {
#include <X11/Xlib.h>

#ifdef    HAVE_STDIO_H
# include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef    TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
#else // !TIME_WITH_SYS_TIME
#  ifdef    HAVE_SYS_TIME_H
#    include <sys/time.h>
#  else // !HAVE_SYS_TIME_H
#    include <time.h>
#  endif // HAVE_SYS_TIME_H
#endif // TIME_WITH_SYS_TIME
}

#include <list>
#include <map>
#include <string>

#include "i18n.hh"
#include "BaseDisplay.hh"
#include "Configuration.hh"
#include "Timer.hh"
#include "XAtom.hh"

#define AttribShaded      (1l << 0)
#define AttribMaxHoriz    (1l << 1)
#define AttribMaxVert     (1l << 2)
#define AttribOmnipresent (1l << 3)
#define AttribWorkspace   (1l << 4)
#define AttribStack       (1l << 5)
#define AttribDecoration  (1l << 6)

#define StackTop          (0)
#define StackNormal       (1)
#define StackBottom       (2)

#define DecorNone         (0)
#define DecorNormal       (1)
#define DecorTiny         (2)
#define DecorTool         (3)

struct BlackboxHints {
  unsigned long flags, attrib, workspace, stack, decoration;
};

struct BlackboxAttributes {
  unsigned long flags, attrib, workspace, stack, decoration;
  int premax_x, premax_y;
  unsigned int premax_w, premax_h;
};

#define PropBlackboxHintsElements      (5)
#define PropBlackboxAttributesElements (9)


//forward declaration
class BScreen;
class Blackbox;
class BlackboxWindow;
class BWindowGroup;
class Basemenu;
class Toolbar;
class Slit;

extern I18n i18n;

class Blackbox : public BaseDisplay, public TimeoutHandler {
private:
  struct BCursor {
    Cursor session, move, ll_angle, lr_angle, ul_angle, ur_angle;
  };
  BCursor cursor;

  struct MenuTimestamp {
    std::string filename;
    time_t timestamp;
  };

  struct BResource {
    Time double_click_interval;

    std::string style_file;
    int colors_per_channel;
    timeval auto_raise_delay;
    unsigned long cache_life, cache_max;
    std::string titlebar_layout;
  } resource;

  typedef std::map<Window, BlackboxWindow*> WindowLookup;
  typedef WindowLookup::value_type WindowLookupPair;
  WindowLookup windowSearchList;

  typedef std::map<Window, BScreen*> WindowScreenLookup;
  typedef WindowScreenLookup::value_type WindowScreenLookupPair;
  WindowScreenLookup systraySearchList;

  typedef std::map<Window, BWindowGroup*> GroupLookup;
  typedef GroupLookup::value_type GroupLookupPair;
  GroupLookup groupSearchList;

  typedef std::map<Window, Basemenu*> MenuLookup;
  typedef MenuLookup::value_type MenuLookupPair;
  MenuLookup menuSearchList;

  typedef std::map<Window, Toolbar*> ToolbarLookup;
  typedef ToolbarLookup::value_type ToolbarLookupPair;
  ToolbarLookup toolbarSearchList;

  typedef std::map<Window, Slit*> SlitLookup;
  typedef SlitLookup::value_type SlitLookupPair;
  SlitLookup slitSearchList;

  typedef std::list<MenuTimestamp*> MenuTimestampList;
  MenuTimestampList menuTimestamps;

  typedef std::list<BScreen*> ScreenList;
  ScreenList screenList;

  BScreen *active_screen;
  BlackboxWindow *focused_window, *changing_window;
  BTimer *timer;
  Configuration config;
  XAtom *xatom;

  bool no_focus, reconfigure_wait, reread_menu_wait;
  Time last_time;
  char **argv;
  std::string menu_file, rc_file;

  Blackbox(const Blackbox&);
  Blackbox& operator=(const Blackbox&);

  void load_rc(void);
  void save_rc(void);
  void real_rereadMenu(void);
  void real_reconfigure(void);

  virtual void process_event(XEvent *);


public:
  Blackbox(char **m_argv, char *dpy_name = 0, char *rc = 0, char *menu = 0);
  virtual ~Blackbox(void);

  Basemenu *searchMenu(Window window);
  BWindowGroup *searchGroup(Window window);
  BScreen *searchSystrayWindow(Window window);
  BlackboxWindow *searchWindow(Window window);
  BScreen *searchScreen(Window window);
  Toolbar *searchToolbar(Window);
  Slit *searchSlit(Window);

  void saveMenuSearch(Window window, Basemenu *data);
  void saveSystrayWindowSearch(Window window, BScreen *screen);
  void saveWindowSearch(Window window, BlackboxWindow *data);
  void saveGroupSearch(Window window, BWindowGroup *data);
  void saveToolbarSearch(Window window, Toolbar *data);
  void saveSlitSearch(Window window, Slit *data);
  void removeMenuSearch(Window window);
  void removeSystrayWindowSearch(Window window);
  void removeWindowSearch(Window window);
  void removeGroupSearch(Window window);
  void removeToolbarSearch(Window window);
  void removeSlitSearch(Window window);

  inline XAtom *getXAtom(void) { return xatom; }

  inline BlackboxWindow *getFocusedWindow(void) { return focused_window; }
  inline BlackboxWindow *getChangingWindow(void) { return changing_window; }

  inline Configuration *getConfig() { return &config; }
  inline const Time &getDoubleClickInterval(void) const
  { return resource.double_click_interval; }
  inline const Time &getLastTime(void) const { return last_time; }

  inline const char *getStyleFilename(void) const
    { return resource.style_file.c_str(); }
  inline const char *getMenuFilename(void) const
    { return menu_file.c_str(); }

  inline int getColorsPerChannel(void) const
    { return resource.colors_per_channel; }

  inline std::string getTitlebarLayout(void) const
    { return resource.titlebar_layout; }

  inline const timeval &getAutoRaiseDelay(void) const
    { return resource.auto_raise_delay; }

  inline unsigned long getCacheLife(void) const
    { return resource.cache_life; }
  inline unsigned long getCacheMax(void) const
    { return resource.cache_max; }

  inline void setNoFocus(bool f) { no_focus = f; }

  inline Cursor getSessionCursor(void) const
    { return cursor.session; }
  inline Cursor getMoveCursor(void) const
    { return cursor.move; }
  inline Cursor getLowerLeftAngleCursor(void) const
    { return cursor.ll_angle; }
  inline Cursor getLowerRightAngleCursor(void) const
    { return cursor.lr_angle; }
  inline Cursor getUpperLeftAngleCursor(void) const
    { return cursor.ul_angle; }
  inline Cursor getUpperRightAngleCursor(void) const
    { return cursor.ur_angle; }

  void setFocusedWindow(BlackboxWindow *win);
  void setChangingWindow(BlackboxWindow *win);
  void shutdown(void);
  void saveStyleFilename(const std::string& filename);
  void addMenuTimestamp(const std::string& filename);
  void restart(const char *prog = 0);
  void reconfigure(void);
  void rereadMenu(void);
  void checkMenu(void);

  bool validateWindow(Window window);

  virtual bool handleSignal(int sig);

  virtual void timeout(void);

#ifndef   HAVE_STRFTIME
  enum { B_AmericanDate = 1, B_EuropeanDate };
#endif // HAVE_STRFTIME
};


#endif // __blackbox_hh
