// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
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

#include "openbox.hh"
#include "configuration.hh"
#include "timer.hh"
#include "xatom.hh"

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

namespace ob {

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

class Blackbox : public Openbox, public TimeoutHandler {
private:
  struct BCursor {
    Cursor session, move, ll_angle, lr_angle, ul_angle, ur_angle;
  };
  BCursor cursor;

  struct BResource {
    Time double_click_interval;

    std::string style_file;
    int colors_per_channel;
    timeval auto_raise_delay;
    unsigned long cache_life, cache_max;
    std::string titlebar_layout;
    unsigned int mod_mask;  // modifier mask used for window-mouse interaction


#ifdef    XINERAMA
    bool xinerama_placement, xinerama_maximize, xinerama_snap;
#endif // XINERAMA
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

  typedef std::list<BScreen*> ScreenList;
  ScreenList screenList;

  BScreen *active_screen;
  BlackboxWindow *focused_window, *changing_window;
  OBTimer *timer;
  Configuration config;
  XAtom *xatom;

  bool no_focus, reconfigure_wait;
  Time last_time;
  char **argv;
  std::string rc_file;

  Blackbox(const Blackbox&);
  Blackbox& operator=(const Blackbox&);

  void load_rc(void);
  void save_rc(void);
  void real_reconfigure(void);

  virtual void process_event(XEvent *);


public:
  Blackbox(int argc, char **m_argv, char *rc = 0);
  virtual ~Blackbox(void);

  BWindowGroup *searchGroup(Window window);
  BScreen *searchSystrayWindow(Window window);
  BlackboxWindow *searchWindow(Window window);
  BScreen *searchScreen(Window window);

#ifdef    XINERAMA
  inline bool doXineramaPlacement(void) const
    { return resource.xinerama_placement; }
  inline bool doXineramaMaximizing(void) const
    { return resource.xinerama_maximize; }
  inline bool doXineramaSnapping(void) const
    { return resource.xinerama_snap; }

  void saveXineramaPlacement(bool x);
  void saveXineramaMaximizing(bool x);
  void saveXineramaSnapping(bool x);
#endif // XINERAMA
  
  void saveSystrayWindowSearch(Window window, BScreen *screen);
  void saveWindowSearch(Window window, BlackboxWindow *data);
  void saveGroupSearch(Window window, BWindowGroup *data);
  void removeSystrayWindowSearch(Window window);
  void removeWindowSearch(Window window);
  void removeGroupSearch(Window window);

  inline XAtom *getXAtom(void) { return xatom; }

  inline BlackboxWindow *getFocusedWindow(void) { return focused_window; }
  inline BlackboxWindow *getChangingWindow(void) { return changing_window; }

  inline Configuration *getConfig() { return &config; }
  inline const Time &getDoubleClickInterval(void) const
  { return resource.double_click_interval; }
  inline const Time &getLastTime(void) const { return last_time; }

  inline const char *getStyleFilename(void) const
    { return resource.style_file.c_str(); }

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
  
  inline unsigned int getMouseModMask(void) const
    { return resource.mod_mask; }

  void setFocusedWindow(BlackboxWindow *win);
  void setChangingWindow(BlackboxWindow *win);
  void shutdown(void);
  void saveStyleFilename(const std::string& filename);
  void restart(const char *prog = 0);
  void reconfigure(void);

  bool validateWindow(Window window);

  virtual bool handleSignal(int sig);

  virtual void timeout(void);

  enum { B_AmericanDate = 1, B_EuropeanDate };
};

}

#endif // __blackbox_hh
