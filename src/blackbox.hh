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
}

#include <list>
#include <map>
#include <string>

#include "i18n.hh"
#include "BaseDisplay.hh"
#include "Timer.hh"

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
    Cursor session, move, ll_angle, lr_angle;
  };
  BCursor cursor;

  struct MenuTimestamp {
    std::string filename;
    time_t timestamp;
  };

  struct BResource {
    Time double_click_interval;

    std::string menu_file, style_file;
    int colors_per_channel;
    timeval auto_raise_delay;
    unsigned long cache_life, cache_max;
  } resource;

  typedef std::map<Window, BlackboxWindow*> WindowLookup;
  typedef WindowLookup::value_type WindowLookupPair;
  WindowLookup windowSearchList;

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
  BlackboxWindow *focused_window;
  BTimer *timer;

  bool no_focus, reconfigure_wait, reread_menu_wait;
  Time last_time;
  char **argv;
  std::string rc_file;

  Atom xa_wm_colormap_windows, xa_wm_protocols, xa_wm_state,
    xa_wm_delete_window, xa_wm_take_focus, xa_wm_change_state,
    motif_wm_hints;

  // NETAttributes
  Atom blackbox_attributes, blackbox_change_attributes, blackbox_hints;
#ifdef    HAVE_GETPID
  Atom blackbox_pid;
#endif // HAVE_GETPID

  // NETStructureMessages
  Atom blackbox_structure_messages, blackbox_notify_startup,
    blackbox_notify_window_add, blackbox_notify_window_del,
    blackbox_notify_window_focus, blackbox_notify_current_workspace,
    blackbox_notify_workspace_count, blackbox_notify_window_raise,
    blackbox_notify_window_lower;

  // message_types for client -> wm messages
  Atom blackbox_change_workspace, blackbox_change_window_focus,
    blackbox_cycle_window_focus;

#ifdef    NEWWMSPEC
  // root window properties
  Atom net_supported, net_client_list, net_client_list_stacking,
    net_number_of_desktops, net_desktop_geometry, net_desktop_viewport,
    net_current_desktop, net_desktop_names, net_active_window, net_workarea,
    net_supporting_wm_check, net_virtual_roots;

  // root window messages
  Atom net_close_window, net_wm_moveresize;

  // application window properties
  Atom net_properties, net_wm_name, net_wm_desktop, net_wm_window_type,
    net_wm_state, net_wm_strut, net_wm_icon_geometry, net_wm_icon, net_wm_pid,
    net_wm_handled_icons;

  // application protocols
  Atom net_wm_ping;
#endif // NEWWMSPEC

  Blackbox(const Blackbox&);
  Blackbox& operator=(const Blackbox&);

  void load_rc(void);
  void save_rc(void);
  void reload_rc(void);
  void real_rereadMenu(void);
  void real_reconfigure(void);

  void init_icccm(void);

  virtual void process_event(XEvent *);


public:
  Blackbox(char **m_argv, char *dpy_name = 0, char *rc = 0);
  virtual ~Blackbox(void);

  Basemenu *searchMenu(Window window);
  BWindowGroup *searchGroup(Window window);
  BlackboxWindow *searchWindow(Window window);
  BScreen *searchScreen(Window window);
  Toolbar *searchToolbar(Window);
  Slit *searchSlit(Window);

  void saveMenuSearch(Window window, Basemenu *data);
  void saveWindowSearch(Window window, BlackboxWindow *data);
  void saveGroupSearch(Window window, BWindowGroup *data);
  void saveToolbarSearch(Window window, Toolbar *data);
  void saveSlitSearch(Window window, Slit *data);
  void removeMenuSearch(Window window);
  void removeWindowSearch(Window window);
  void removeGroupSearch(Window window);
  void removeToolbarSearch(Window window);
  void removeSlitSearch(Window window);

  inline BlackboxWindow *getFocusedWindow(void) { return focused_window; }

  inline const Time &getDoubleClickInterval(void) const
  { return resource.double_click_interval; }
  inline const Time &getLastTime(void) const { return last_time; }

  inline const char *getStyleFilename(void) const
    { return resource.style_file.c_str(); }
  inline const char *getMenuFilename(void) const
    { return resource.menu_file.c_str(); }

  inline int getColorsPerChannel(void) const
    { return resource.colors_per_channel; }

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

  void setFocusedWindow(BlackboxWindow *w);
  void shutdown(void);
  void load_rc(BScreen *screen);
  void saveStyleFilename(const std::string& filename);
  void saveMenuFilename(const std::string& filename);
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

#ifdef    HAVE_GETPID
  inline Atom getBlackboxPidAtom(void) const { return blackbox_pid; }
#endif // HAVE_GETPID

  inline Atom getWMChangeStateAtom(void) const
    { return xa_wm_change_state; }
  inline Atom getWMStateAtom(void) const
    { return xa_wm_state; }
  inline Atom getWMDeleteAtom(void) const
    { return xa_wm_delete_window; }
  inline Atom getWMProtocolsAtom(void) const
    { return xa_wm_protocols; }
  inline Atom getWMTakeFocusAtom(void) const
    { return xa_wm_take_focus; }
  inline Atom getWMColormapAtom(void) const
    { return xa_wm_colormap_windows; }
  inline Atom getMotifWMHintsAtom(void) const
    { return motif_wm_hints; }

  // this atom is for normal app->WM hints about decorations, stacking,
  // starting workspace etc...
  inline Atom getBlackboxHintsAtom(void) const
    { return blackbox_hints;}

  // these atoms are for normal app->WM interaction beyond the scope of the
  // ICCCM...
  inline Atom getBlackboxAttributesAtom(void) const
    { return blackbox_attributes; }
  inline Atom getBlackboxChangeAttributesAtom(void) const
    { return blackbox_change_attributes; }

  // these atoms are for window->WM interaction, with more control and
  // information on window "structure"... common examples are
  // notifying apps when windows are raised/lowered... when the user changes
  // workspaces... i.e. "pager talk"
  inline Atom getBlackboxStructureMessagesAtom(void) const
    { return blackbox_structure_messages; }

  // *Notify* portions of the NETStructureMessages protocol
  inline Atom getBlackboxNotifyStartupAtom(void) const
    { return blackbox_notify_startup; }
  inline Atom getBlackboxNotifyWindowAddAtom(void) const
    { return blackbox_notify_window_add; }
  inline Atom getBlackboxNotifyWindowDelAtom(void) const
    { return blackbox_notify_window_del; }
  inline Atom getBlackboxNotifyWindowFocusAtom(void) const
    { return blackbox_notify_window_focus; }
  inline Atom getBlackboxNotifyCurrentWorkspaceAtom(void) const
    { return blackbox_notify_current_workspace; }
  inline Atom getBlackboxNotifyWorkspaceCountAtom(void) const
    { return blackbox_notify_workspace_count; }
  inline Atom getBlackboxNotifyWindowRaiseAtom(void) const
    { return blackbox_notify_window_raise; }
  inline Atom getBlackboxNotifyWindowLowerAtom(void) const
    { return blackbox_notify_window_lower; }

  // atoms to change that request changes to the desktop environment during
  // runtime... these messages can be sent by any client... as the sending
  // client window id is not included in the ClientMessage event...
  inline Atom getBlackboxChangeWorkspaceAtom(void) const
    { return blackbox_change_workspace; }
  inline Atom getBlackboxChangeWindowFocusAtom(void) const
    { return blackbox_change_window_focus; }
  inline Atom getBlackboxCycleWindowFocusAtom(void) const
    { return blackbox_cycle_window_focus; }

#ifdef    NEWWMSPEC
  // root window properties
  inline Atom getNETSupportedAtom(void) const
    { return net_supported; }
  inline Atom getNETClientListAtom(void) const
    { return net_client_list; }
  inline Atom getNETClientListStackingAtom(void) const
    { return net_client_list_stacking; }
  inline Atom getNETNumberOfDesktopsAtom(void) const
    { return net_number_of_desktops; }
  inline Atom getNETDesktopGeometryAtom(void) const
    { return net_desktop_geometry; }
  inline Atom getNETDesktopViewportAtom(void) const
    { return net_desktop_viewport; }
  inline Atom getNETCurrentDesktopAtom(void) const
    { return net_current_desktop; }
  inline Atom getNETDesktopNamesAtom(void) const
    { return net_desktop_names; }
  inline Atom getNETActiveWindowAtom(void) const
    { return net_active_window; }
  inline Atom getNETWorkareaAtom(void) const
    { return net_workarea; }
  inline Atom getNETSupportingWMCheckAtom(void) const
    { return net_supporting_wm_check; }
  inline Atom getNETVirtualRootsAtom(void) const
    { return net_virtual_roots; }

  // root window messages
  inline Atom getNETCloseWindowAtom(void) const
    { return net_close_window; }
  inline Atom getNETWMMoveResizeAtom(void) const
    { return net_wm_moveresize; }

  // application window properties
  inline Atom getNETPropertiesAtom(void) const
    { return net_properties; }
  inline Atom getNETWMNameAtom(void) const
    { return net_wm_name; }
  inline Atom getNETWMDesktopAtom(void) const
    { return net_wm_desktop; }
  inline Atom getNETWMWindowTypeAtom(void) const
    { return net_wm_window_type; }
  inline Atom getNETWMStateAtom(void) const
    { return net_wm_state; }
  inline Atom getNETWMStrutAtom(void) const
    { return net_wm_strut; }
  inline Atom getNETWMIconGeometryAtom(void) const
    { return net_wm_icon_geometry; }
  inline Atom getNETWMIconAtom(void) const
    { return net_wm_icon; }
  inline Atom getNETWMPidAtom(void) const
    { return net_wm_pid; }
  inline Atom getNETWMHandledIconsAtom(void) const
    { return net_wm_handled_icons; }

  // application protocols
  inline Atom getNETWMPingAtom(void) const
    { return net_wm_ping; }
#endif // NEWWMSPEC
};


#endif // __blackbox_hh
