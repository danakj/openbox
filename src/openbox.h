// openbox.h for Openbox
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

#ifndef   __openbox_hh
#define   __openbox_hh

#include <X11/Xlib.h>
#include <X11/Xresource.h>

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

#include "Resource.h"
#include "LinkedList.h"
#include "BaseDisplay.h"
#include "Timer.h"

//forward declaration
class BScreen;
class Openbox;
class BImageControl;
class OpenboxWindow;
class Basemenu;
class Toolbar;
#ifdef    SLIT
class Slit;
#endif // SLIT

template <class Z>
class DataSearch {
private:
  Window window;
  Z *data;

public:
  DataSearch(Window w, Z *d): window(w), data(d) {}

  inline const Window &getWindow(void) const { return window; }
  inline Z *getData(void) { return data; }
};


class Openbox : public BaseDisplay, public TimeoutHandler {
private:
  typedef struct MenuTimestamp {
    char *filename;
    time_t timestamp;
  } MenuTimestamp;

  struct resource {
    Time double_click_interval;

    char *menu_file, *style_file;
    char *titlebar_layout;
    int colors_per_channel;
    timeval auto_raise_delay;
    unsigned long cache_life, cache_max;
  } resource;

  typedef DataSearch<OpenboxWindow> WindowSearch;
  LinkedList<WindowSearch> *windowSearchList, *groupSearchList;
  typedef DataSearch<Basemenu> MenuSearch;
  LinkedList<MenuSearch> *menuSearchList;
  typedef DataSearch<Toolbar> ToolbarSearch;
  LinkedList<ToolbarSearch> *toolbarSearchList;

#ifdef    SLIT
  typedef DataSearch<Slit> SlitSearch;
  LinkedList<SlitSearch> *slitSearchList;
#endif // SLIT

  LinkedList<MenuTimestamp> *menuTimestamps;
  LinkedList<BScreen> *screenList;

  OpenboxWindow *focused_window, *masked_window;
  BTimer *timer;

#ifdef    HAVE_GETPID
  Atom openbox_pid;
#endif // HAVE_GETPID

  Bool no_focus, reconfigure_wait, reread_menu_wait;
  Time last_time;
  Window masked;
  char *rc_file, **argv;
  int argc;
  obResource config;


protected:
  void load_rc(void);
  void save_rc(void);
  void reload_rc(void);
  void real_rereadMenu(void);
  void real_reconfigure(void);

  virtual void process_event(XEvent *);


public:
  Openbox(int, char **, char * = 0, char * = 0);
  virtual ~Openbox(void);

#ifdef    HAVE_GETPID
  inline const Atom &getOpenboxPidAtom(void) const { return openbox_pid; }
#endif // HAVE_GETPID

  Basemenu *searchMenu(Window);

  OpenboxWindow *searchGroup(Window, OpenboxWindow *);
  OpenboxWindow *searchWindow(Window);
  inline OpenboxWindow *getFocusedWindow(void) { return focused_window; }

  BScreen *getScreen(int);
  BScreen *searchScreen(Window);

  inline obResource &getConfig() {
    return config;
  }
  inline const Time &getDoubleClickInterval(void) const
    { return resource.double_click_interval; }
  inline const Time &getLastTime(void) const { return last_time; }

  Toolbar *searchToolbar(Window);

  inline const char *getStyleFilename(void) const
    { return resource.style_file; }
  inline const char *getMenuFilename(void) const
    { return resource.menu_file; }

  inline const int &getColorsPerChannel(void) const
    { return resource.colors_per_channel; }

  inline const timeval &getAutoRaiseDelay(void) const
    { return resource.auto_raise_delay; }

  inline const char *getTitleBarLayout(void) const
    { return resource.titlebar_layout; }

  inline const unsigned long &getCacheLife(void) const
    { return resource.cache_life; }
  inline const unsigned long &getCacheMax(void) const
    { return resource.cache_max; }

  inline void maskWindowEvents(Window w, OpenboxWindow *bw)
    { masked = w; masked_window = bw; }
  inline void setNoFocus(Bool f) { no_focus = f; }

  void setFocusedWindow(OpenboxWindow *w);
  void shutdown(void);
  void load_rc(BScreen *);
  void saveStyleFilename(const char *);
  void saveMenuFilename(const char *);
  void saveMenuSearch(Window, Basemenu *);
  void saveWindowSearch(Window, OpenboxWindow *);
  void saveToolbarSearch(Window, Toolbar *);
  void saveGroupSearch(Window, OpenboxWindow *);
  void removeMenuSearch(Window);
  void removeWindowSearch(Window);
  void removeToolbarSearch(Window);
  void removeGroupSearch(Window);
  void restart(const char * = 0);
  void reconfigure(void);
  void rereadMenu(void);
  void checkMenu(void);

  virtual Bool handleSignal(int);

  virtual void timeout(void);

#ifdef    SLIT
  Slit *searchSlit(Window);

  void saveSlitSearch(Window, Slit *);
  void removeSlitSearch(Window);
#endif // SLIT

#ifndef   HAVE_STRFTIME

  enum { B_AmericanDate = 1, B_EuropeanDate };
#endif // HAVE_STRFTIME
};


#endif // __openbox_hh
