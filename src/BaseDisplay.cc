// BaseDisplay.cc for Openbox
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

// stupid macros needed to access some functions in version 2 of the GNU C
// library
#ifndef   _GNU_SOURCE
#define   _GNU_SOURCE
#endif // _GNU_SOURCE

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#ifdef    SHAPE
#  include <X11/extensions/shape.h>
#endif // SHAPE

#ifdef    HAVE_FCNTL_H
#  include <fcntl.h>
#endif // HAVE_FCNTL_H

#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#ifdef    HAVE_STRING_H
#  include <string.h>
#endif // HAVE_STRING_H

#ifdef    HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif // HAVE_UNISTD_H

#ifdef    HAVE_SYS_SELECT_H
#  include <sys/select.h>
#endif // HAVE_SYS_SELECT_H

#ifdef    HAVE_SIGNAL_H
#  include <signal.h>
#endif // HAVE_SIGNAL_H

#ifndef   SA_NODEFER
#  ifdef   SA_INTERRUPT
#    define SA_NODEFER SA_INTERRUPT
#  else // !SA_INTERRUPT
#    define SA_NODEFER (0)
#  endif // SA_INTERRUPT
#endif // SA_NODEFER

#ifdef    HAVE_SYS_WAIT_H
#  include <sys/types.h>
#  include <sys/wait.h>
#endif // HAVE_SYS_WAIT_H

#if defined(HAVE_PROCESS_H) && defined(__EMX__)
#  include <process.h>
#endif //   HAVE_PROCESS_H             __EMX__

#include "i18n.h"
#include "BaseDisplay.h"
#include "LinkedList.h"
#include "Timer.h"

// X error handler to handle any and all X errors while the application is
// running
static Bool internal_error = False;
static Window last_bad_window = None;

BaseDisplay *base_display;

static int handleXErrors(Display *d, XErrorEvent *e) {
#ifdef    DEBUG
  char errtxt[128];

  XGetErrorText(d, e->error_code, errtxt, 128);
  fprintf(stderr, i18n->getMessage(BaseDisplaySet, BaseDisplayXError,
		     "%s:  X error: %s(%d) opcodes %d/%d\n  resource 0x%lx\n"),
          base_display->getApplicationName(), errtxt, e->error_code,
          e->request_code, e->minor_code, e->resourceid);
#endif // DEBUG

  if (e->error_code == BadWindow) last_bad_window = e->resourceid;
  if (internal_error) abort();

  return(False);
}


// signal handler to allow for proper and gentle shutdown

#ifndef   HAVE_SIGACTION
static RETSIGTYPE signalhandler(int sig) {
#else //  HAVE_SIGACTION
static void signalhandler(int sig) {
#endif // HAVE_SIGACTION

  static int re_enter = 0;

  switch (sig) {
  case SIGCHLD:
    int status;
    waitpid(-1, &status, WNOHANG | WUNTRACED);

#ifndef   HAVE_SIGACTION
    // assume broken, braindead sysv signal semantics
    signal(SIGCHLD, (RETSIGTYPE (*)(int)) signalhandler);
#endif // HAVE_SIGACTION

    break;

  default:
    if (base_display->handleSignal(sig)) {

#ifndef   HAVE_SIGACTION
      // assume broken, braindead sysv signal semantics
      signal(sig, (RETSIGTYPE (*)(int)) signalhandler);
#endif // HAVE_SIGACTION

      return;
    }

    fprintf(stderr, i18n->getMessage(BaseDisplaySet, BaseDisplaySignalCaught,
				     "%s:  signal %d caught\n"),
	    base_display->getApplicationName(), sig);

    if (! base_display->isStartup() && ! re_enter) {
      internal_error = True;

      re_enter = 1;
      fprintf(stderr, i18n->getMessage(BaseDisplaySet, BaseDisplayShuttingDown,
				       "shutting down\n"));
      base_display->shutdown();
    }

    if (sig != SIGTERM && sig != SIGINT) {
      fprintf(stderr, i18n->getMessage(BaseDisplaySet, BaseDisplayAborting,
				       "aborting... dumping core\n"));
      abort();
    }

    exit(0);

    break;
  }
}


// convenience functions
#ifndef    __EMX__
void bexec(const char *command, char* displaystring) {
  if (! fork()) {
    setsid();
    putenv(displaystring);
    execl("/bin/sh", "/bin/sh", "-c", command, NULL);
    exit(0);
  }
}
#endif // !__EMX__

char *bstrdup(const char *s) {
  const int l = strlen(s) + 1;
  char *n = new char[l];
  strncpy(n, s, l);
  return n;
}

BaseDisplay::BaseDisplay(const char *app_name, char *dpy_name) {
  application_name = bstrdup(app_name);

  _startup = True;
  _shutdown = False;
  server_grabs = 0;
  last_bad_window = None;

  ::base_display = this;

#ifdef    HAVE_SIGACTION
  struct sigaction action;

  action.sa_handler = signalhandler;
  action.sa_mask = sigset_t();
  action.sa_flags = SA_NOCLDSTOP | SA_NODEFER;

  sigaction(SIGPIPE, &action, NULL);
  sigaction(SIGSEGV, &action, NULL);
  sigaction(SIGFPE, &action, NULL);
  sigaction(SIGTERM, &action, NULL);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGCHLD, &action, NULL);
  sigaction(SIGHUP, &action, NULL);
  sigaction(SIGUSR1, &action, NULL);
  sigaction(SIGUSR2, &action, NULL);
#else // !HAVE_SIGACTION
  signal(SIGPIPE, (RETSIGTYPE (*)(int)) signalhandler);
  signal(SIGSEGV, (RETSIGTYPE (*)(int)) signalhandler);
  signal(SIGFPE, (RETSIGTYPE (*)(int)) signalhandler);
  signal(SIGTERM, (RETSIGTYPE (*)(int)) signalhandler);
  signal(SIGINT, (RETSIGTYPE (*)(int)) signalhandler);
  signal(SIGUSR1, (RETSIGTYPE (*)(int)) signalhandler);
  signal(SIGUSR2, (RETSIGTYPE (*)(int)) signalhandler);
  signal(SIGHUP, (RETSIGTYPE (*)(int)) signalhandler);
  signal(SIGCHLD, (RETSIGTYPE (*)(int)) signalhandler);
#endif // HAVE_SIGACTION

  if (! (display = XOpenDisplay(dpy_name))) {
    fprintf(stderr, i18n->getMessage(BaseDisplaySet, BaseDisplayXConnectFail,
	       "BaseDisplay::BaseDisplay: connection to X server failed.\n"));
    ::exit(2);
  } else if (fcntl(ConnectionNumber(display), F_SETFD, 1) == -1) {
    fprintf(stderr,
	    i18n->getMessage(BaseDisplaySet, BaseDisplayCloseOnExecFail,
	       "BaseDisplay::BaseDisplay: couldn't mark display connection "
	       "as close-on-exec\n"));
    ::exit(2);
  }

  number_of_screens = ScreenCount(display);
  display_name = XDisplayName(dpy_name);

#ifdef    SHAPE
  shape.extensions = XShapeQueryExtension(display, &shape.event_basep,
                                          &shape.error_basep);
#else // !SHAPE
  shape.extensions = False;
#endif // SHAPE

  xa_wm_colormap_windows =
    XInternAtom(display, "WM_COLORMAP_WINDOWS", False);
  xa_wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
  xa_wm_state = XInternAtom(display, "WM_STATE", False);
  xa_wm_change_state = XInternAtom(display, "WM_CHANGE_STATE", False);
  xa_wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
  xa_wm_take_focus = XInternAtom(display, "WM_TAKE_FOCUS", False);
  motif_wm_hints = XInternAtom(display, "_MOTIF_WM_HINTS", False);

  openbox_hints = XInternAtom(display, "_BLACKBOX_HINTS", False);
  openbox_attributes = XInternAtom(display, "_BLACKBOX_ATTRIBUTES", False);
  openbox_change_attributes =
    XInternAtom(display, "_BLACKBOX_CHANGE_ATTRIBUTES", False);

  openbox_structure_messages =
    XInternAtom(display, "_BLACKBOX_STRUCTURE_MESSAGES", False);
  openbox_notify_startup =
    XInternAtom(display, "_BLACKBOX_NOTIFY_STARTUP", False);
  openbox_notify_window_add =
    XInternAtom(display, "_BLACKBOX_NOTIFY_WINDOW_ADD", False);
  openbox_notify_window_del =
    XInternAtom(display, "_BLACKBOX_NOTIFY_WINDOW_DEL", False);
  openbox_notify_current_workspace =
    XInternAtom(display, "_BLACKBOX_NOTIFY_CURRENT_WORKSPACE", False);
  openbox_notify_workspace_count =
    XInternAtom(display, "_BLACKBOX_NOTIFY_WORKSPACE_COUNT", False);
  openbox_notify_window_focus =
    XInternAtom(display, "_BLACKBOX_NOTIFY_WINDOW_FOCUS", False);
  openbox_notify_window_raise =
    XInternAtom(display, "_BLACKBOX_NOTIFY_WINDOW_RAISE", False);
  openbox_notify_window_lower =
    XInternAtom(display, "_BLACKBOX_NOTIFY_WINDOW_LOWER", False);

  openbox_change_workspace =
    XInternAtom(display, "_BLACKBOX_CHANGE_WORKSPACE", False);
  openbox_change_window_focus =
    XInternAtom(display, "_BLACKBOX_CHANGE_WINDOW_FOCUS", False);
  openbox_cycle_window_focus =
    XInternAtom(display, "_BLACKBOX_CYCLE_WINDOW_FOCUS", False);

#ifdef    NEWWMSPEC

  net_supported = XInternAtom(display, "_NET_SUPPORTED", False);
  net_client_list = XInternAtom(display, "_NET_CLIENT_LIST", False);
  net_client_list_stacking = XInternAtom(display, "_NET_CLIENT_LIST_STACKING", False);
  net_number_of_desktops = XInternAtom(display, "_NET_NUMBER_OF_DESKTOPS", False);
  net_desktop_geometry = XInternAtom(display, "_NET_DESKTOP_GEOMETRY", False);
  net_desktop_viewport = XInternAtom(display, "_NET_DESKTOP_VIEWPORT", False);
  net_current_desktop = XInternAtom(display, "_NET_CURRENT_DESKTOP", False);
  net_desktop_names = XInternAtom(display, "_NET_DESKTOP_NAMES", False);
  net_active_window = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
  net_workarea = XInternAtom(display, "_NET_WORKAREA", False);
  net_supporting_wm_check = XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", False);
  net_virtual_roots = XInternAtom(display, "_NET_VIRTUAL_ROOTS", False);

  net_close_window = XInternAtom(display, "_NET_CLOSE_WINDOW", False);
  net_wm_moveresize = XInternAtom(display, "_NET_WM_MOVERESIZE", False);

  net_properties = XInternAtom(display, "_NET_PROPERTIES", False);
  net_wm_name = XInternAtom(display, "_NET_WM_NAME", False);
  net_wm_desktop = XInternAtom(display, "_NET_WM_DESKTOP", False);
  net_wm_window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
  net_wm_state = XInternAtom(display, "_NET_WM_STATE", False);
  net_wm_strut = XInternAtom(display, "_NET_WM_STRUT", False);
  net_wm_icon_geometry = XInternAtom(display, "_NET_WM_ICON_GEOMETRY", False);
  net_wm_icon = XInternAtom(display, "_NET_WM_ICON", False);
  net_wm_pid = XInternAtom(display, "_NET_WM_PID", False);
  net_wm_handled_icons = XInternAtom(display, "_NET_WM_HANDLED_ICONS", False);

  net_wm_ping = XInternAtom(display, "_NET_WM_PING", False);

#endif // NEWWMSPEC

  cursor.session = XCreateFontCursor(display, XC_left_ptr);
  cursor.move = XCreateFontCursor(display, XC_fleur);
  cursor.ll_angle = XCreateFontCursor(display, XC_ll_angle);
  cursor.lr_angle = XCreateFontCursor(display, XC_lr_angle);
  cursor.ul_angle = XCreateFontCursor(display, XC_ul_angle);
  cursor.ur_angle = XCreateFontCursor(display, XC_ur_angle);

  XSetErrorHandler((XErrorHandler) handleXErrors);

  timerList = new LinkedList<BTimer>;

  screenInfoList = new LinkedList<ScreenInfo>;
  for (int i = 0; i < number_of_screens; i++) {
    ScreenInfo *screeninfo = new ScreenInfo(*this, i);
    screenInfoList->insert(screeninfo);
  }

#ifndef   NOCLOBBER
  NumLockMask = ScrollLockMask = 0;

  const XModifierKeymap* const modmap = XGetModifierMapping(display);
  if (modmap && modmap->max_keypermod > 0) {
    const int mask_table[] = {
      ShiftMask, LockMask, ControlMask, Mod1Mask,
      Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
    };
    const size_t size = (sizeof(mask_table) / sizeof(mask_table[0])) *
      modmap->max_keypermod;
    // get the values of the keyboard lock modifiers
    // Note: Caps lock is not retrieved the same way as Scroll and Num lock
    // since it doesn't need to be.
    const KeyCode num_lock_code = XKeysymToKeycode(display, XK_Num_Lock);
    const KeyCode scroll_lock_code = XKeysymToKeycode(display, XK_Scroll_Lock);
    
    for (size_t cnt = 0; cnt < size; ++cnt) {
      if (! modmap->modifiermap[cnt]) continue;

      if (num_lock_code == modmap->modifiermap[cnt])
	NumLockMask = mask_table[cnt / modmap->max_keypermod];
      if (scroll_lock_code == modmap->modifiermap[cnt])
	ScrollLockMask = mask_table[cnt / modmap->max_keypermod];
    }
  }

  MaskList[0] = 0;
  MaskList[1] = LockMask;
  MaskList[2] = NumLockMask;
  MaskList[3] = ScrollLockMask;
  MaskList[4] = LockMask | NumLockMask;
  MaskList[5] = NumLockMask  | ScrollLockMask;
  MaskList[6] = LockMask | ScrollLockMask;
  MaskList[7] = LockMask | NumLockMask | ScrollLockMask;
  MaskListLength = sizeof(MaskList) / sizeof(MaskList[0]);
  
  if (modmap) XFreeModifiermap(const_cast<XModifierKeymap*>(modmap));
#else
  NumLockMask = Mod2Mask;
  ScrollLockMask = Mod5Mask;
#endif // NOCLOBBER
}


BaseDisplay::~BaseDisplay(void) {
  while (screenInfoList->count()) {
    ScreenInfo *si = screenInfoList->first();

    screenInfoList->remove(si);
    delete si;
  }

  delete screenInfoList;

  // we don't create the BTimers, we don't delete them
  while (timerList->count())
    timerList->remove(0);

  delete timerList;

  XCloseDisplay(display);
}


void BaseDisplay::eventLoop(void) {
  run();

  int xfd = ConnectionNumber(display);

  while ((! _shutdown) && (! internal_error)) {
    if (XPending(display)) {
      XEvent e;
      XNextEvent(display, &e);

      if (last_bad_window != None && e.xany.window == last_bad_window) {
#ifdef    DEBUG
      fprintf(stderr, i18n->getMessage(BaseDisplaySet,
				       BaseDisplayBadWindowRemove,
			 "BaseDisplay::eventLoop(): removing bad window "
			 "from event queue\n"));
#endif // DEBUG
      } else {
	last_bad_window = None;
        process_event(&e);
      }
    } else {
      fd_set rfds;
      timeval now, tm, *timeout = (timeval *) 0;

      FD_ZERO(&rfds);
      FD_SET(xfd, &rfds);

      if (timerList->count()) {
        gettimeofday(&now, 0);

        tm.tv_sec = tm.tv_usec = 0l;

        BTimer *timer = timerList->first();

        tm.tv_sec = timer->getStartTime().tv_sec +
          timer->getTimeout().tv_sec - now.tv_sec;
        tm.tv_usec = timer->getStartTime().tv_usec +
          timer->getTimeout().tv_usec - now.tv_usec;

        while (tm.tv_usec >= 1000000) {
          tm.tv_sec++;
          tm.tv_usec -= 1000000;
        }

        while (tm.tv_usec < 0) {
          if (tm.tv_sec > 0) {
            tm.tv_sec--;
            tm.tv_usec += 1000000;
          } else {
            tm.tv_usec = 0;
            break;
          }
        }

        timeout = &tm;
      }

      select(xfd + 1, &rfds, 0, 0, timeout);

      // check for timer timeout
      gettimeofday(&now, 0);

      LinkedListIterator<BTimer> it(timerList);
      for(BTimer *timer = it.current(); timer; it++, timer = it.current()) {
        tm.tv_sec = timer->getStartTime().tv_sec +
          timer->getTimeout().tv_sec;
        tm.tv_usec = timer->getStartTime().tv_usec +
          timer->getTimeout().tv_usec;

        if ((now.tv_sec < tm.tv_sec) ||
            (now.tv_sec == tm.tv_sec && now.tv_usec < tm.tv_usec))
          break;

        timer->fireTimeout();

        // restart the current timer so that the start time is updated
        if (! timer->doOnce()) timer->start();
        else timer->stop();
      }
    }
  }
}


const Bool BaseDisplay::validateWindow(Window window) {
  XEvent event;
  if (XCheckTypedWindowEvent(display, window, DestroyNotify, &event)) {
    XPutBackEvent(display, &event);

    return False;
  }

  return True;
}


void BaseDisplay::grab(void) {
  if (! server_grabs++)
    XGrabServer(display);
}


void BaseDisplay::ungrab(void) {
  if (! --server_grabs)
    XUngrabServer(display);

  if (server_grabs < 0) server_grabs = 0;
}


void BaseDisplay::addTimer(BTimer *timer) {
  if (! timer) return;

  LinkedListIterator<BTimer> it(timerList);
  int index = 0;
  for (BTimer *tmp = it.current(); tmp; it++, index++, tmp = it.current())
    if ((tmp->getTimeout().tv_sec > timer->getTimeout().tv_sec) ||
        ((tmp->getTimeout().tv_sec == timer->getTimeout().tv_sec) &&
         (tmp->getTimeout().tv_usec >= timer->getTimeout().tv_usec)))
      break;

  timerList->insert(timer, index);
}


void BaseDisplay::removeTimer(BTimer *timer) {
  timerList->remove(timer);
}


/*
 * Grabs a button, but also grabs the button in every possible combination with
 * the keyboard lock keys, so that they do not cancel out the event.
 */
void BaseDisplay::grabButton(unsigned int button, unsigned int modifiers,
			     Window grab_window, Bool owner_events,
			     unsigned int event_mask, int pointer_mode,
			     int keybaord_mode, Window confine_to,
			     Cursor cursor) const
{
#ifndef   NOCLOBBER
  for (size_t cnt = 0; cnt < MaskListLength; ++cnt)
    XGrabButton(display, button, modifiers | MaskList[cnt], grab_window,
                owner_events, event_mask, pointer_mode, keybaord_mode,
                confine_to, cursor);
#else  // NOCLOBBER
    XGrabButton(display, button, modifiers, grab_window,
                owner_events, event_mask, pointer_mode, keybaord_mode,
                confine_to, cursor);
#endif // NOCLOBBER
}

/*
 * Releases the grab on a button, and ungrabs all possible combinations of the
 * keyboard lock keys.
 */
void BaseDisplay::ungrabButton(unsigned int button, unsigned int modifiers,
			       Window grab_window) const {
#ifndef   NOCLOBBER
  for (size_t cnt = 0; cnt < MaskListLength; ++cnt)
    XUngrabButton(display, button, modifiers | MaskList[cnt], grab_window);
#else  // NOCLOBBER
    XUngrabButton(display, button, modifiers, grab_window);
#endif // NOCLOBBER
}


ScreenInfo::ScreenInfo(BaseDisplay &d, int num) : basedisplay(d),
  screen_number(num)
{

  root_window = RootWindow(basedisplay.getXDisplay(), screen_number);
  depth = DefaultDepth(basedisplay.getXDisplay(), screen_number);

  m_size = Size(WidthOfScreen(ScreenOfDisplay(basedisplay.getXDisplay(),
                                              screen_number)),
                HeightOfScreen(ScreenOfDisplay(basedisplay.getXDisplay(),
                                               screen_number)));

  // search for a TrueColor Visual... if we can't find one... we will use the
  // default visual for the screen
  XVisualInfo vinfo_template, *vinfo_return;
  int vinfo_nitems;

  vinfo_template.screen = screen_number;
  vinfo_template.c_class = TrueColor;

  visual = (Visual *) 0;

  if ((vinfo_return = XGetVisualInfo(basedisplay.getXDisplay(),
                                     VisualScreenMask | VisualClassMask,
                                     &vinfo_template, &vinfo_nitems)) &&
      vinfo_nitems > 0) {
    for (int i = 0; i < vinfo_nitems; i++) {
      if (depth < (vinfo_return + i)->depth) {
        depth = (vinfo_return + i)->depth;
        visual = (vinfo_return + i)->visual;
      }
    }

    XFree(vinfo_return);
  }

  if (visual) {
    colormap = XCreateColormap(basedisplay.getXDisplay(), root_window,
			       visual, AllocNone);
  } else {
    visual = DefaultVisual(basedisplay.getXDisplay(), screen_number);
    colormap = DefaultColormap(basedisplay.getXDisplay(), screen_number);
  }
}
