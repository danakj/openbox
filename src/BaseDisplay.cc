// -*- mode: C++; indent-tabs-mode: nil; -*-
// BaseDisplay.cc for Blackbox - an X11 Window manager
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

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
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

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#ifdef HAVE_STRING_H
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
}

#include <sstream>
using std::string;

#include "i18n.hh"
#include "BaseDisplay.hh"
#include "GCCache.hh"
#include "Timer.hh"
#include "Util.hh"


// X error handler to handle any and all X errors while the application is
// running
static bool internal_error = False;
static Window last_bad_window = None;

BaseDisplay *base_display;

#ifdef    DEBUG
static int handleXErrors(Display *d, XErrorEvent *e) {
  char errtxt[128];

  XGetErrorText(d, e->error_code, errtxt, 128);
  fprintf(stderr,
          i18n(BaseDisplaySet, BaseDisplayXError,
               "%s:  X error: %s(%d) opcodes %d/%d\n  resource 0x%lx\n"),
          base_display->getApplicationName(), errtxt, e->error_code,
          e->request_code, e->minor_code, e->resourceid);
#else
static int handleXErrors(Display *, XErrorEvent *e) {
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

    fprintf(stderr, i18n(BaseDisplaySet, BaseDisplaySignalCaught,
                         "%s:  signal %d caught\n"),
            base_display->getApplicationName(), sig);

    if (! base_display->isStartup() && ! re_enter) {
      internal_error = True;

      re_enter = 1;
      fprintf(stderr, i18n(BaseDisplaySet, BaseDisplayShuttingDown,
                           "shutting down\n"));
      base_display->shutdown();
    }

    if (sig != SIGTERM && sig != SIGINT) {
      fprintf(stderr, i18n(BaseDisplaySet, BaseDisplayAborting,
                           "aborting... dumping core\n"));
      abort();
    }

    exit(0);

    break;
  }
}


BaseDisplay::BaseDisplay(const char *app_name, const char *dpy_name) {
  application_name = app_name;

  run_state = STARTUP;
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
    fprintf(stderr,
            i18n(BaseDisplaySet, BaseDisplayXConnectFail,
               "BaseDisplay::BaseDisplay: connection to X server failed.\n"));
    ::exit(2);
  } else if (fcntl(ConnectionNumber(display), F_SETFD, 1) == -1) {
    fprintf(stderr,
            i18n(BaseDisplaySet, BaseDisplayCloseOnExecFail,
                 "BaseDisplay::BaseDisplay: couldn't mark display connection "
                 "as close-on-exec\n"));
    ::exit(2);
  }

  display_name = XDisplayName(dpy_name);

#ifdef    SHAPE
  shape.extensions = XShapeQueryExtension(display, &shape.event_basep,
                                          &shape.error_basep);
#else // !SHAPE
  shape.extensions = False;
#endif // SHAPE

  XSetErrorHandler((XErrorHandler) handleXErrors);

  screenInfoList.reserve(ScreenCount(display));
  for (int i = 0; i < ScreenCount(display); ++i)
    screenInfoList.push_back(ScreenInfo(this, i));

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
    const KeyCode num_lock = XKeysymToKeycode(display, XK_Num_Lock);
    const KeyCode scroll_lock = XKeysymToKeycode(display, XK_Scroll_Lock);

    for (size_t cnt = 0; cnt < size; ++cnt) {
      if (! modmap->modifiermap[cnt]) continue;

      if (num_lock == modmap->modifiermap[cnt])
        NumLockMask = mask_table[cnt / modmap->max_keypermod];
      if (scroll_lock == modmap->modifiermap[cnt])
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

  gccache = 0;
}


BaseDisplay::~BaseDisplay(void) {
  delete gccache;

  XCloseDisplay(display);
}


void BaseDisplay::eventLoop(void) {
  run();

  const int xfd = ConnectionNumber(display);

  while (run_state == RUNNING && ! internal_error) {
    if (XPending(display)) {
      XEvent e;
      XNextEvent(display, &e);

      if (last_bad_window != None && e.xany.window == last_bad_window)
        continue;

      last_bad_window = None;
      process_event(&e);
    } else {
      fd_set rfds;
      timeval now, tm, *timeout = (timeval *) 0;

      FD_ZERO(&rfds);
      FD_SET(xfd, &rfds);

      if (! timerList.empty()) {
        const BTimer* const timer = timerList.top();

        gettimeofday(&now, 0);
        tm = timer->timeRemaining(now);

        timeout = &tm;
      }

      select(xfd + 1, &rfds, 0, 0, timeout);

      // check for timer timeout
      gettimeofday(&now, 0);

      // there is a small chance for deadlock here:
      // *IF* the timer list keeps getting refreshed *AND* the time between
      // timer->start() and timer->shouldFire() is within the timer's period
      // then the timer will keep firing.  This should be VERY near impossible.
      while (! timerList.empty()) {
        BTimer *timer = timerList.top();
        if (! timer->shouldFire(now))
          break;

        timerList.pop();

        timer->fireTimeout();
        timer->halt();
        if (timer->isRecurring())
          timer->start();
      }
    }
  }
}


void BaseDisplay::addTimer(BTimer *timer) {
  if (! timer) return;

  timerList.push(timer);
}


void BaseDisplay::removeTimer(BTimer *timer) {
  timerList.release(timer);
}


/*
 * Grabs a button, but also grabs the button in every possible combination
 * with the keyboard lock keys, so that they do not cancel out the event.
 */
void BaseDisplay::grabButton(unsigned int button, unsigned int modifiers,
                             Window grab_window, bool owner_events,
                             unsigned int event_mask, int pointer_mode,
                             int keyboard_mode, Window confine_to,
                             Cursor cursor) const {
  for (size_t cnt = 0; cnt < MaskListLength; ++cnt) {
    XGrabButton(display, button, modifiers | MaskList[cnt], grab_window,
                owner_events, event_mask, pointer_mode, keyboard_mode,
                confine_to, cursor);
  }
}

/*
 * Releases the grab on a button, and ungrabs all possible combinations of the
 * keyboard lock keys.
 */
void BaseDisplay::ungrabButton(unsigned int button, unsigned int modifiers,
                               Window grab_window) const {
  for (size_t cnt = 0; cnt < MaskListLength; ++cnt) {
    XUngrabButton(display, button, modifiers | MaskList[cnt], grab_window);
  }
}


const ScreenInfo* BaseDisplay::getScreenInfo(unsigned int s) const {
  if (s < screenInfoList.size())
    return &screenInfoList[s];
  return (const ScreenInfo*) 0;
}


BGCCache *BaseDisplay::gcCache(void) const
{
    if (! gccache) gccache = new BGCCache(this);
    return gccache;
}


ScreenInfo::ScreenInfo(BaseDisplay *d, unsigned int num) {
  basedisplay = d;
  screen_number = num;

  root_window = RootWindow(basedisplay->getXDisplay(), screen_number);
  depth = DefaultDepth(basedisplay->getXDisplay(), screen_number);

  rect.setSize(WidthOfScreen(ScreenOfDisplay(basedisplay->getXDisplay(),
                                             screen_number)),
               HeightOfScreen(ScreenOfDisplay(basedisplay->getXDisplay(),
                                              screen_number)));

  // search for a TrueColor Visual... if we can't find one... we will use the
  // default visual for the screen
  XVisualInfo vinfo_template, *vinfo_return;
  int vinfo_nitems;

  vinfo_template.screen = screen_number;
  vinfo_template.c_class = TrueColor;

  visual = (Visual *) 0;

  vinfo_return = XGetVisualInfo(basedisplay->getXDisplay(),
                                VisualScreenMask | VisualClassMask,
                                &vinfo_template, &vinfo_nitems);
  if (vinfo_return && vinfo_nitems > 0) {
    for (int i = 0; i < vinfo_nitems; i++) {
      if (depth < (vinfo_return + i)->depth) {
        depth = (vinfo_return + i)->depth;
        visual = (vinfo_return + i)->visual;
      }
    }

    XFree(vinfo_return);
  }

  if (visual) {
    colormap = XCreateColormap(basedisplay->getXDisplay(), root_window,
                               visual, AllocNone);
  } else {
    visual = DefaultVisual(basedisplay->getXDisplay(), screen_number);
    colormap = DefaultColormap(basedisplay->getXDisplay(), screen_number);
  }

  // get the default display string and strip the screen number
  string default_string = DisplayString(basedisplay->getXDisplay());
  const string::size_type pos = default_string.rfind(".");
  if (pos != string::npos)
    default_string.resize(pos);

  std::ostringstream formatter;
  formatter << "DISPLAY=" << default_string << '.' << screen_number;
  display_string = formatter.str();
}
