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
#include "BaseDisplay.h"
#include "Timer.h"

#include <map>
#include <list>

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

  inline const Window &getWindow() const { return window; }
  inline Z *getData() { return data; }
};


class Openbox : public BaseDisplay, public TimeoutHandler {
private:
  typedef struct MenuTimestamp {
    virtual ~MenuTimestamp() {
      if (filename != (char *) 0)
        delete [] filename;
    }
    char *filename;
    time_t timestamp;
  } MenuTimestamp;

  struct resource {
    Time double_click_interval;

    char *style_file;
    char *titlebar_layout;
    int colors_per_channel;
    timeval auto_raise_delay;
    unsigned long cache_life, cache_max;
  } resource;

  typedef std::map<Window, OpenboxWindow*> WindowLookup;
  typedef WindowLookup::value_type WindowLookupPair;
  WindowLookup windowSearchList, groupSearchList;
  
  typedef std::map<Window, Basemenu*> MenuLookup;
  typedef MenuLookup::value_type MenuLookupPair;
  MenuLookup menuSearchList;

  typedef std::map<Window, Toolbar*> ToolbarLookup;
  typedef ToolbarLookup::value_type ToolbarLookupPair;
  ToolbarLookup toolbarSearchList;

#ifdef    SLIT
  typedef std::map<Window, Slit*> SlitLookup;
  typedef SlitLookup::value_type SlitLookupPair;
  SlitLookup slitSearchList; 
#endif // SLIT

  typedef std::list<MenuTimestamp*> MenuTimestampList;
  MenuTimestampList menuTimestamps;

  typedef std::list<BScreen*> ScreenList;
  ScreenList screenList;

  BScreen *current_screen;
  OpenboxWindow *masked_window;
  BTimer *timer;

#ifdef    HAVE_GETPID
  Atom openbox_pid;
#endif // HAVE_GETPID

  Bool no_focus, reconfigure_wait, reread_menu_wait;
  Time last_time;
  Window masked;
  char *menu_file, *rc_file, **argv;
  int argc;
  Resource config;


protected:
  void load();
  void save();
  void real_rereadMenu();
  void real_reconfigure();

  virtual void process_event(XEvent *);


public:
  Openbox(int, char **, char * = 0, char * = 0, char * = 0);
  virtual ~Openbox();

#ifdef    HAVE_GETPID
  inline const Atom &getOpenboxPidAtom() const { return openbox_pid; }
#endif // HAVE_GETPID

  Basemenu *searchMenu(Window);

  OpenboxWindow *searchGroup(Window, OpenboxWindow *);
  OpenboxWindow *searchWindow(Window);
  OpenboxWindow *focusedWindow();
  void focusWindow(OpenboxWindow *w);

  BScreen *getScreen(int);
  BScreen *searchScreen(Window);

  inline Resource &getConfig() {
    return config;
  }
  inline const Time &getDoubleClickInterval() const
    { return resource.double_click_interval; }
  inline const Time &getLastTime() const { return last_time; }

  Toolbar *searchToolbar(Window);

  inline const char *getStyleFilename() const
    { return resource.style_file; }
  inline const char *getMenuFilename() const
    { return menu_file; }
  void addMenuTimestamp(const char *filename);

  inline const int &getColorsPerChannel() const
    { return resource.colors_per_channel; }

  inline const timeval &getAutoRaiseDelay() const
    { return resource.auto_raise_delay; }

  inline const char *getTitleBarLayout() const
    { return resource.titlebar_layout; }

  inline const unsigned long &getCacheLife() const
    { return resource.cache_life; }
  inline const unsigned long &getCacheMax() const
    { return resource.cache_max; }

  inline OpenboxWindow *getMaskedWindow() const
    { return masked_window; }
  inline void maskWindowEvents(Window w, OpenboxWindow *bw)
    { masked = w; masked_window = bw; }
  inline void setNoFocus(Bool f) { no_focus = f; }

  void shutdown();
  void setStyleFilename(const char *);
  void saveMenuSearch(Window, Basemenu *);
  void saveWindowSearch(Window, OpenboxWindow *);
  void saveToolbarSearch(Window, Toolbar *);
  void saveGroupSearch(Window, OpenboxWindow *);
  void removeMenuSearch(Window);
  void removeWindowSearch(Window);
  void removeToolbarSearch(Window);
  void removeGroupSearch(Window);
  void restart(const char * = 0);
  void reconfigure();
  void rereadMenu();
  void checkMenu();

  virtual Bool handleSignal(int);

  virtual void timeout();

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
