// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// blackbox.cc for Blackbox - an X11 Window manager
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
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#ifdef    SHAPE
#include <X11/extensions/shape.h>
#endif // SHAPE

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

#ifdef    HAVE_SYS_PARAM_H
#  include <sys/param.h>
#endif // HAVE_SYS_PARAM_H

#ifdef    HAVE_SYS_SELECT_H
#  include <sys/select.h>
#endif // HAVE_SYS_SELECT_H

#ifdef    HAVE_SIGNAL_H
#  include <signal.h>
#endif // HAVE_SIGNAL_H

#ifdef    HAVE_SYS_SIGNAL_H
#  include <sys/signal.h>
#endif // HAVE_SYS_SIGNAL_H

#ifdef    HAVE_SYS_STAT_H
#  include <sys/types.h>
#  include <sys/stat.h>
#endif // HAVE_SYS_STAT_H

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

#ifdef    HAVE_LIBGEN_H
#  include <libgen.h>
#endif // HAVE_LIBGEN_H
}

#include <algorithm>
#include <string>
using std::string;

#include "i18n.hh"
#include "blackbox.hh"
#include "Basemenu.hh"
#include "Clientmenu.hh"
#include "GCCache.hh"
#include "Image.hh"
#include "Rootmenu.hh"
#include "Screen.hh"
#include "Slit.hh"
#include "Toolbar.hh"
#include "Util.hh"
#include "Window.hh"
#include "Workspace.hh"
#include "Workspacemenu.hh"


// X event scanner for enter/leave notifies - adapted from twm
struct scanargs {
  Window w;
  bool leave, inferior, enter;
};

static Bool queueScanner(Display *, XEvent *e, char *args) {
  scanargs *scan = (scanargs *) args;
  if ((e->type == LeaveNotify) &&
      (e->xcrossing.window == scan->w) &&
      (e->xcrossing.mode == NotifyNormal)) {
    scan->leave = True;
    scan->inferior = (e->xcrossing.detail == NotifyInferior);
  } else if ((e->type == EnterNotify) && (e->xcrossing.mode == NotifyUngrab)) {
    scan->enter = True;
  }

  return False;
}

Blackbox *blackbox;


Blackbox::Blackbox(char **m_argv, char *dpy_name, char *rc, char *menu)
  : BaseDisplay(m_argv[0], dpy_name) {
  if (! XSupportsLocale())
    fprintf(stderr, "X server does not support locale\n");

  if (XSetLocaleModifiers("") == NULL)
    fprintf(stderr, "cannot set locale modifiers\n");

  ::blackbox = this;
  argv = m_argv;
  if (! rc) rc = "~/.openbox/rc";
  rc_file = expandTilde(rc);
  config.setFile(rc_file);  
  if (! menu) menu = "~/.openbox/menu";
  menu_file = expandTilde(menu);

  no_focus = False;

  resource.auto_raise_delay.tv_sec = resource.auto_raise_delay.tv_usec = 0;

  active_screen = 0;
  focused_window = (BlackboxWindow *) 0;

  XrmInitialize();
  load_rc();

  init_icccm();

  cursor.session = XCreateFontCursor(getXDisplay(), XC_left_ptr);
  cursor.move = XCreateFontCursor(getXDisplay(), XC_fleur);
  cursor.ll_angle = XCreateFontCursor(getXDisplay(), XC_ll_angle);
  cursor.lr_angle = XCreateFontCursor(getXDisplay(), XC_lr_angle);

  for (unsigned int i = 0; i < getNumberOfScreens(); i++) {
    BScreen *screen = new BScreen(this, i);

    if (! screen->isScreenManaged()) {
      delete screen;
      continue;
    }

    screenList.push_back(screen);
  }

  if (screenList.empty()) {
    fprintf(stderr,
            i18n(blackboxSet, blackboxNoManagableScreens,
              "Blackbox::Blackbox: no managable screens found, aborting.\n"));
    ::exit(3);
  }

  // save current settings and default values
  save_rc();

  // set the screen with mouse to the first managed screen
  active_screen = screenList.front();
  setFocusedWindow(0);

  XSynchronize(getXDisplay(), False);
  XSync(getXDisplay(), False);

  reconfigure_wait = reread_menu_wait = False;

  timer = new BTimer(this, this);
  timer->setTimeout(0l);
}


Blackbox::~Blackbox(void) {
  std::for_each(screenList.begin(), screenList.end(), PointerAssassin());

  std::for_each(menuTimestamps.begin(), menuTimestamps.end(),
                PointerAssassin());

  delete timer;
}


void Blackbox::process_event(XEvent *e) {
  switch (e->type) {
  case ButtonPress: {
    // strip the lock key modifiers
    e->xbutton.state &= ~(NumLockMask | ScrollLockMask | LockMask);

    last_time = e->xbutton.time;

    BlackboxWindow *win = (BlackboxWindow *) 0;
    Basemenu *menu = (Basemenu *) 0;
    Slit *slit = (Slit *) 0;
    Toolbar *tbar = (Toolbar *) 0;
    BScreen *scrn = (BScreen *) 0;

    if ((win = searchWindow(e->xbutton.window))) {
      win->buttonPressEvent(&e->xbutton);

      /* XXX: is this sane on low colour desktops? */
      if (e->xbutton.button == 1)
        win->installColormap(True);
    } else if ((menu = searchMenu(e->xbutton.window))) {
      menu->buttonPressEvent(&e->xbutton);
    } else if ((slit = searchSlit(e->xbutton.window))) {
      slit->buttonPressEvent(&e->xbutton);
    } else if ((tbar = searchToolbar(e->xbutton.window))) {
      tbar->buttonPressEvent(&e->xbutton);
    } else if ((scrn = searchScreen(e->xbutton.window))) {
      scrn->buttonPressEvent(&e->xbutton);
      if (active_screen != scrn) {
        active_screen = scrn;
        // first, set no focus window on the old screen
        setFocusedWindow(0);
        // and move focus to this screen
        setFocusedWindow(0);
      }
    }
    break;
  }

  case ButtonRelease: {
    // strip the lock key modifiers
    e->xbutton.state &= ~(NumLockMask | ScrollLockMask | LockMask);

    last_time = e->xbutton.time;

    BlackboxWindow *win = (BlackboxWindow *) 0;
    Basemenu *menu = (Basemenu *) 0;
    Toolbar *tbar = (Toolbar *) 0;

    if ((win = searchWindow(e->xbutton.window)))
      win->buttonReleaseEvent(&e->xbutton);
    else if ((menu = searchMenu(e->xbutton.window)))
      menu->buttonReleaseEvent(&e->xbutton);
    else if ((tbar = searchToolbar(e->xbutton.window)))
      tbar->buttonReleaseEvent(&e->xbutton);

    break;
  }

  case ConfigureRequest: {
    // compress configure requests...
    XEvent realevent;
    unsigned int i = 0;
    while(XCheckTypedWindowEvent(getXDisplay(), e->xconfigurerequest.window,
                                 ConfigureRequest, &realevent)) {
      i++;
    }
    if ( i > 0 )
      e = &realevent;

    BlackboxWindow *win = (BlackboxWindow *) 0;
    Slit *slit = (Slit *) 0;

    if ((win = searchWindow(e->xconfigurerequest.window))) {
      win->configureRequestEvent(&e->xconfigurerequest);
    } else if ((slit = searchSlit(e->xconfigurerequest.window))) {
      slit->configureRequestEvent(&e->xconfigurerequest);
    } else {
      if (validateWindow(e->xconfigurerequest.window)) {
        XWindowChanges xwc;

        xwc.x = e->xconfigurerequest.x;
        xwc.y = e->xconfigurerequest.y;
        xwc.width = e->xconfigurerequest.width;
        xwc.height = e->xconfigurerequest.height;
        xwc.border_width = e->xconfigurerequest.border_width;
        xwc.sibling = e->xconfigurerequest.above;
        xwc.stack_mode = e->xconfigurerequest.detail;

        XConfigureWindow(getXDisplay(), e->xconfigurerequest.window,
                         e->xconfigurerequest.value_mask, &xwc);
      }
    }

    break;
  }

  case MapRequest: {
#ifdef    DEBUG
    fprintf(stderr, "Blackbox::process_event(): MapRequest for 0x%lx\n",
            e->xmaprequest.window);
#endif // DEBUG

    BlackboxWindow *win = searchWindow(e->xmaprequest.window);

    if (! win) {
      BScreen *screen = searchScreen(e->xmaprequest.parent);

      if (! screen) {
        /*
          we got a map request for a window who's parent isn't root. this
          can happen in only one circumstance:

            a client window unmapped a managed window, and then remapped it
            somewhere between unmapping the client window and reparenting it
            to root.

          regardless of how it happens, we need to find the screen that
          the window is on
        */
        XWindowAttributes wattrib;
        if (! XGetWindowAttributes(getXDisplay(), e->xmaprequest.window,
                                   &wattrib)) {
          // failed to get the window attributes, perhaps the window has
          // now been destroyed?
          break;
        }

        screen = searchScreen(wattrib.root);
        assert(screen != 0); // this should never happen
      }

      screen->manageWindow(e->xmaprequest.window);
    }

    break;
  }

  case UnmapNotify: {
    BlackboxWindow *win = (BlackboxWindow *) 0;
    Slit *slit = (Slit *) 0;

    if ((win = searchWindow(e->xunmap.window))) {
      win->unmapNotifyEvent(&e->xunmap);
    } else if ((slit = searchSlit(e->xunmap.window))) {
      slit->unmapNotifyEvent(&e->xunmap);
    }

    break;
  }

  case DestroyNotify: {
    BlackboxWindow *win = (BlackboxWindow *) 0;
    Slit *slit = (Slit *) 0;
    BWindowGroup *group = (BWindowGroup *) 0;

    if ((win = searchWindow(e->xdestroywindow.window))) {
      win->destroyNotifyEvent(&e->xdestroywindow);
    } else if ((slit = searchSlit(e->xdestroywindow.window))) {
      slit->removeClient(e->xdestroywindow.window, False);
    } else if ((group = searchGroup(e->xdestroywindow.window))) {
      delete group;
    }

    break;
  }

  case ReparentNotify: {
    /*
      this event is quite rare and is usually handled in unmapNotify
      however, if the window is unmapped when the reparent event occurs
      the window manager never sees it because an unmap event is not sent
      to an already unmapped window.
    */
    BlackboxWindow *win = searchWindow(e->xreparent.window);
    if (win) {
      win->reparentNotifyEvent(&e->xreparent);
    } else {
      Slit *slit = searchSlit(e->xreparent.window);
      if (slit && slit->getWindowID() != e->xreparent.parent)
        slit->removeClient(e->xreparent.window, True);
    }
    break;
  }

  case MotionNotify: {
    // motion notify compression...
    XEvent realevent;
    unsigned int i = 0;
    while (XCheckTypedWindowEvent(getXDisplay(), e->xmotion.window,
                                  MotionNotify, &realevent)) {
      i++;
    }

    // if we have compressed some motion events, use the last one
    if ( i > 0 )
      e = &realevent;

    // strip the lock key modifiers
    e->xbutton.state &= ~(NumLockMask | ScrollLockMask | LockMask);

    last_time = e->xmotion.time;

    BlackboxWindow *win = (BlackboxWindow *) 0;
    Basemenu *menu = (Basemenu *) 0;

    if ((win = searchWindow(e->xmotion.window)))
      win->motionNotifyEvent(&e->xmotion);
    else if ((menu = searchMenu(e->xmotion.window)))
      menu->motionNotifyEvent(&e->xmotion);

    break;
  }

  case PropertyNotify: {
    last_time = e->xproperty.time;

    if (e->xproperty.state != PropertyDelete) {
      BlackboxWindow *win = searchWindow(e->xproperty.window);

      if (win)
        win->propertyNotifyEvent(e->xproperty.atom);
    }

    break;
  }

  case EnterNotify: {
    last_time = e->xcrossing.time;

    BScreen *screen = (BScreen *) 0;
    BlackboxWindow *win = (BlackboxWindow *) 0;
    Basemenu *menu = (Basemenu *) 0;
    Toolbar *tbar = (Toolbar *) 0;
    Slit *slit = (Slit *) 0;

    if (e->xcrossing.mode == NotifyGrab) break;

    XEvent dummy;
    scanargs sa;
    sa.w = e->xcrossing.window;
    sa.enter = sa.leave = False;
    XCheckIfEvent(getXDisplay(), &dummy, queueScanner, (char *) &sa);

    if ((e->xcrossing.window == e->xcrossing.root) &&
        (screen = searchScreen(e->xcrossing.window))) {
      screen->getImageControl()->installRootColormap();
    } else if ((win = searchWindow(e->xcrossing.window))) {
      if (win->getScreen()->isSloppyFocus() &&
          (! win->isFocused()) && (! no_focus)) {
        if (((! sa.leave) || sa.inferior) && win->isVisible()) {
          if (win->setInputFocus())
            win->installColormap(True); // XXX: shouldnt we honour no install?
        }
      }
    } else if ((menu = searchMenu(e->xcrossing.window))) {
      menu->enterNotifyEvent(&e->xcrossing);
    } else if ((tbar = searchToolbar(e->xcrossing.window))) {
      tbar->enterNotifyEvent(&e->xcrossing);
    } else if ((slit = searchSlit(e->xcrossing.window))) {
      slit->enterNotifyEvent(&e->xcrossing);
    }
    break;
  }

  case LeaveNotify: {
    last_time = e->xcrossing.time;

    BlackboxWindow *win = (BlackboxWindow *) 0;
    Basemenu *menu = (Basemenu *) 0;
    Toolbar *tbar = (Toolbar *) 0;
    Slit *slit = (Slit *) 0;

    if ((menu = searchMenu(e->xcrossing.window)))
      menu->leaveNotifyEvent(&e->xcrossing);
    else if ((win = searchWindow(e->xcrossing.window)))
      win->installColormap(False);
    else if ((tbar = searchToolbar(e->xcrossing.window)))
      tbar->leaveNotifyEvent(&e->xcrossing);
    else if ((slit = searchSlit(e->xcrossing.window)))
      slit->leaveNotifyEvent(&e->xcrossing);
    break;
  }

  case Expose: {
    // compress expose events
    XEvent realevent;
    unsigned int i = 0;
    int ex1, ey1, ex2, ey2;
    ex1 = e->xexpose.x;
    ey1 = e->xexpose.y;
    ex2 = ex1 + e->xexpose.width - 1;
    ey2 = ey1 + e->xexpose.height - 1;
    while (XCheckTypedWindowEvent(getXDisplay(), e->xexpose.window,
                                  Expose, &realevent)) {
      i++;

      // merge expose area
      ex1 = std::min(realevent.xexpose.x, ex1);
      ey1 = std::min(realevent.xexpose.y, ey1);
      ex2 = std::max(realevent.xexpose.x + realevent.xexpose.width - 1, ex2);
      ey2 = std::max(realevent.xexpose.y + realevent.xexpose.height - 1, ey2);
    }
    if ( i > 0 )
      e = &realevent;

    // use the merged area
    e->xexpose.x = ex1;
    e->xexpose.y = ey1;
    e->xexpose.width = ex2 - ex1 + 1;
    e->xexpose.height = ey2 - ey1 + 1;

    BlackboxWindow *win = (BlackboxWindow *) 0;
    Basemenu *menu = (Basemenu *) 0;
    Toolbar *tbar = (Toolbar *) 0;

    if ((win = searchWindow(e->xexpose.window)))
      win->exposeEvent(&e->xexpose);
    else if ((menu = searchMenu(e->xexpose.window)))
      menu->exposeEvent(&e->xexpose);
    else if ((tbar = searchToolbar(e->xexpose.window)))
      tbar->exposeEvent(&e->xexpose);

    break;
  }

  case KeyPress: {
    Toolbar *tbar = searchToolbar(e->xkey.window);

    if (tbar && tbar->isEditing())
      tbar->keyPressEvent(&e->xkey);

    break;
  }

  case ColormapNotify: {
    BScreen *screen = searchScreen(e->xcolormap.window);

    if (screen)
      screen->setRootColormapInstalled((e->xcolormap.state ==
                                        ColormapInstalled) ? True : False);

    break;
  }

  case FocusIn: {
    if (e->xfocus.detail != NotifyNonlinear) {
      /*
        don't process FocusIns when:
        1. the new focus window isn't an ancestor or inferior of the old
        focus window (NotifyNonlinear)
      */
      break;
    }

    BlackboxWindow *win = searchWindow(e->xfocus.window);
    if (win) {
      if (! win->isFocused())
        win->setFocusFlag(True);

      /*
        set the event window to None.  when the FocusOut event handler calls
        this function recursively, it uses this as an indication that focus
        has moved to a known window.
      */
      e->xfocus.window = None;
    }

    break;
  }

  case FocusOut: {
    if (e->xfocus.detail != NotifyNonlinear) {
      /*
        don't process FocusOuts when:
        2. the new focus window isn't an ancestor or inferior of the old
        focus window (NotifyNonlinear)
      */
      break;
    }

    BlackboxWindow *win = searchWindow(e->xfocus.window);
    if (win && win->isFocused()) {
      /*
        before we mark "win" as unfocused, we need to verify that focus is
        going to a known location, is in a known location, or set focus
        to a known location.
      */

      XEvent event;
      // don't check the current focus if FocusOut was generated during a grab
      bool check_focus = (e->xfocus.mode == NotifyNormal);

      /*
        First, check if there is a pending FocusIn event waiting.  if there
        is, process it and determine if focus has moved to another window
        (the FocusIn event handler sets the window in the event
        structure to None to indicate this).
      */
      if (XCheckTypedEvent(getXDisplay(), FocusIn, &event)) {

        process_event(&event);
        if (event.xfocus.window == None) {
          // focus has moved
          check_focus = False;
        }
      }

      if (check_focus) {
        /*
          Second, we query the X server for the current input focus.
          to make sure that we keep a consistent state.
        */
        BlackboxWindow *focus;
        Window w;
        int revert;
        XGetInputFocus(getXDisplay(), &w, &revert);
        focus = searchWindow(w);
        if (focus) {
          /*
            focus got from "win" to "focus" under some very strange
            circumstances, and we need to make sure that the focus indication
            is correct.
          */
          setFocusedWindow(focus);
        } else {
          // we have no idea where focus went... so we set it to somewhere
          setFocusedWindow(0);
        }
      }
    }

    break;
  }

  case ClientMessage: {
    if (e->xclient.format == 32) {
      if (e->xclient.message_type == getWMChangeStateAtom()) {
        BlackboxWindow *win = searchWindow(e->xclient.window);
        if (! win || ! win->validateClient()) return;

        if (e->xclient.data.l[0] == IconicState)
          win->iconify();
        if (e->xclient.data.l[0] == NormalState)
          win->deiconify();
      } else if(e->xclient.message_type == getBlackboxChangeWorkspaceAtom()) {
        BScreen *screen = searchScreen(e->xclient.window);

        if (screen && e->xclient.data.l[0] >= 0 &&
            e->xclient.data.l[0] <
            static_cast<signed>(screen->getWorkspaceCount()))
          screen->changeWorkspaceID(e->xclient.data.l[0]);
      } else if (e->xclient.message_type == getBlackboxChangeWindowFocusAtom()) {
        BlackboxWindow *win = searchWindow(e->xclient.window);

        if (win && win->isVisible() && win->setInputFocus())
          win->installColormap(True);
      } else if (e->xclient.message_type == getBlackboxCycleWindowFocusAtom()) {
        BScreen *screen = searchScreen(e->xclient.window);

        if (screen) {
          if (! e->xclient.data.l[0])
            screen->prevFocus();
          else
            screen->nextFocus();
        }
      } else if (e->xclient.message_type == getBlackboxChangeAttributesAtom()) {
        BlackboxWindow *win = searchWindow(e->xclient.window);

        if (win && win->validateClient()) {
          BlackboxHints net;
          net.flags = e->xclient.data.l[0];
          net.attrib = e->xclient.data.l[1];
          net.workspace = e->xclient.data.l[2];
          net.stack = e->xclient.data.l[3];
          net.decoration = e->xclient.data.l[4];

          win->changeBlackboxHints(&net);
        }
      }
    }

    break;
  }

  case NoExpose:
  case ConfigureNotify:
  case MapNotify:
    break; // not handled, just ignore

  default: {
#ifdef    SHAPE
    if (e->type == getShapeEventBase()) {
      XShapeEvent *shape_event = (XShapeEvent *) e;
      BlackboxWindow *win = searchWindow(e->xany.window);

      if (win)
        win->shapeEvent(shape_event);
    }
#endif // SHAPE
  }
  } // switch
}


bool Blackbox::handleSignal(int sig) {
  switch (sig) {
  case SIGHUP:
  case SIGUSR1:
    reconfigure();
    break;

  case SIGUSR2:
    rereadMenu();
    break;

  case SIGPIPE:
  case SIGSEGV:
  case SIGFPE:
  case SIGINT:
  case SIGTERM:
    shutdown();

  default:
    return False;
  }

  return True;
}


void Blackbox::init_icccm(void) {
  xa_wm_colormap_windows =
    XInternAtom(getXDisplay(), "WM_COLORMAP_WINDOWS", False);
  xa_wm_protocols = XInternAtom(getXDisplay(), "WM_PROTOCOLS", False);
  xa_wm_state = XInternAtom(getXDisplay(), "WM_STATE", False);
  xa_wm_change_state = XInternAtom(getXDisplay(), "WM_CHANGE_STATE", False);
  xa_wm_delete_window = XInternAtom(getXDisplay(), "WM_DELETE_WINDOW", False);
  xa_wm_take_focus = XInternAtom(getXDisplay(), "WM_TAKE_FOCUS", False);
  motif_wm_hints = XInternAtom(getXDisplay(), "_MOTIF_WM_HINTS", False);

  blackbox_hints = XInternAtom(getXDisplay(), "_BLACKBOX_HINTS", False);
  blackbox_attributes =
    XInternAtom(getXDisplay(), "_BLACKBOX_ATTRIBUTES", False);
  blackbox_change_attributes =
    XInternAtom(getXDisplay(), "_BLACKBOX_CHANGE_ATTRIBUTES", False);
  blackbox_structure_messages =
    XInternAtom(getXDisplay(), "_BLACKBOX_STRUCTURE_MESSAGES", False);
  blackbox_notify_startup =
    XInternAtom(getXDisplay(), "_BLACKBOX_NOTIFY_STARTUP", False);
  blackbox_notify_window_add =
    XInternAtom(getXDisplay(), "_BLACKBOX_NOTIFY_WINDOW_ADD", False);
  blackbox_notify_window_del =
    XInternAtom(getXDisplay(), "_BLACKBOX_NOTIFY_WINDOW_DEL", False);
  blackbox_notify_current_workspace =
    XInternAtom(getXDisplay(), "_BLACKBOX_NOTIFY_CURRENT_WORKSPACE", False);
  blackbox_notify_workspace_count =
    XInternAtom(getXDisplay(), "_BLACKBOX_NOTIFY_WORKSPACE_COUNT", False);
  blackbox_notify_window_focus =
    XInternAtom(getXDisplay(), "_BLACKBOX_NOTIFY_WINDOW_FOCUS", False);
  blackbox_notify_window_raise =
    XInternAtom(getXDisplay(), "_BLACKBOX_NOTIFY_WINDOW_RAISE", False);
  blackbox_notify_window_lower =
    XInternAtom(getXDisplay(), "_BLACKBOX_NOTIFY_WINDOW_LOWER", False);
  blackbox_change_workspace =
    XInternAtom(getXDisplay(), "_BLACKBOX_CHANGE_WORKSPACE", False);
  blackbox_change_window_focus =
    XInternAtom(getXDisplay(), "_BLACKBOX_CHANGE_WINDOW_FOCUS", False);
  blackbox_cycle_window_focus =
    XInternAtom(getXDisplay(), "_BLACKBOX_CYCLE_WINDOW_FOCUS", False);

#ifdef    NEWWMSPEC
  net_supported = XInternAtom(getXDisplay(), "_NET_SUPPORTED", False);
  net_client_list = XInternAtom(getXDisplay(), "_NET_CLIENT_LIST", False);
  net_client_list_stacking =
    XInternAtom(getXDisplay(), "_NET_CLIENT_LIST_STACKING", False);
  net_number_of_desktops =
    XInternAtom(getXDisplay(), "_NET_NUMBER_OF_DESKTOPS", False);
  net_desktop_geometry =
    XInternAtom(getXDisplay(), "_NET_DESKTOP_GEOMETRY", False);
  net_desktop_viewport =
    XInternAtom(getXDisplay(), "_NET_DESKTOP_VIEWPORT", False);
  net_current_desktop =
    XInternAtom(getXDisplay(), "_NET_CURRENT_DESKTOP", False);
  net_desktop_names = XInternAtom(getXDisplay(), "_NET_DESKTOP_NAMES", False);
  net_active_window = XInternAtom(getXDisplay(), "_NET_ACTIVE_WINDOW", False);
  net_workarea = XInternAtom(getXDisplay(), "_NET_WORKAREA", False);
  net_supporting_wm_check =
    XInternAtom(getXDisplay(), "_NET_SUPPORTING_WM_CHECK", False);
  net_virtual_roots = XInternAtom(getXDisplay(), "_NET_VIRTUAL_ROOTS", False);
  net_close_window = XInternAtom(getXDisplay(), "_NET_CLOSE_WINDOW", False);
  net_wm_moveresize = XInternAtom(getXDisplay(), "_NET_WM_MOVERESIZE", False);
  net_properties = XInternAtom(getXDisplay(), "_NET_PROPERTIES", False);
  net_wm_name = XInternAtom(getXDisplay(), "_NET_WM_NAME", False);
  net_wm_desktop = XInternAtom(getXDisplay(), "_NET_WM_DESKTOP", False);
  net_wm_window_type =
    XInternAtom(getXDisplay(), "_NET_WM_WINDOW_TYPE", False);
  net_wm_state = XInternAtom(getXDisplay(), "_NET_WM_STATE", False);
  net_wm_strut = XInternAtom(getXDisplay(), "_NET_WM_STRUT", False);
  net_wm_icon_geometry =
    XInternAtom(getXDisplay(), "_NET_WM_ICON_GEOMETRY", False);
  net_wm_icon = XInternAtom(getXDisplay(), "_NET_WM_ICON", False);
  net_wm_pid = XInternAtom(getXDisplay(), "_NET_WM_PID", False);
  net_wm_handled_icons =
    XInternAtom(getXDisplay(), "_NET_WM_HANDLED_ICONS", False);
  net_wm_ping = XInternAtom(getXDisplay(), "_NET_WM_PING", False);
#endif // NEWWMSPEC

#ifdef    HAVE_GETPID
  blackbox_pid = XInternAtom(getXDisplay(), "_BLACKBOX_PID", False);
#endif // HAVE_GETPID
}


bool Blackbox::validateWindow(Window window) {
  XEvent event;
  if (XCheckTypedWindowEvent(getXDisplay(), window, DestroyNotify, &event)) {
    XPutBackEvent(getXDisplay(), &event);

    return False;
  }

  return True;
}


BScreen *Blackbox::searchScreen(Window window) {
  ScreenList::iterator it = screenList.begin();

  for (; it != screenList.end(); ++it) {
    BScreen *s = *it;
    if (s->getRootWindow() == window)
      return s;
  }

  return (BScreen *) 0;
}


BlackboxWindow *Blackbox::searchWindow(Window window) {
  WindowLookup::iterator it = windowSearchList.find(window);
  if (it != windowSearchList.end())
    return it->second;

  return (BlackboxWindow*) 0;
}


BWindowGroup *Blackbox::searchGroup(Window window) {
  GroupLookup::iterator it = groupSearchList.find(window);
  if (it != groupSearchList.end())
    return it->second;

  return (BWindowGroup *) 0;
}


Basemenu *Blackbox::searchMenu(Window window) {
  MenuLookup::iterator it = menuSearchList.find(window);
  if (it != menuSearchList.end())
    return it->second;

  return (Basemenu*) 0;
}


Toolbar *Blackbox::searchToolbar(Window window) {
  ToolbarLookup::iterator it = toolbarSearchList.find(window);
  if (it != toolbarSearchList.end())
    return it->second;

  return (Toolbar*) 0;
}


Slit *Blackbox::searchSlit(Window window) {
  SlitLookup::iterator it = slitSearchList.find(window);
  if (it != slitSearchList.end())
    return it->second;

  return (Slit*) 0;
}


void Blackbox::saveWindowSearch(Window window, BlackboxWindow *data) {
  windowSearchList.insert(WindowLookupPair(window, data));
}


void Blackbox::saveGroupSearch(Window window, BWindowGroup *data) {
  groupSearchList.insert(GroupLookupPair(window, data));
}


void Blackbox::saveMenuSearch(Window window, Basemenu *data) {
  menuSearchList.insert(MenuLookupPair(window, data));
}


void Blackbox::saveToolbarSearch(Window window, Toolbar *data) {
  toolbarSearchList.insert(ToolbarLookupPair(window, data));
}


void Blackbox::saveSlitSearch(Window window, Slit *data) {
  slitSearchList.insert(SlitLookupPair(window, data));
}


void Blackbox::removeWindowSearch(Window window) {
  windowSearchList.erase(window);
}


void Blackbox::removeGroupSearch(Window window) {
  groupSearchList.erase(window);
}


void Blackbox::removeMenuSearch(Window window) {
  menuSearchList.erase(window);
}


void Blackbox::removeToolbarSearch(Window window) {
  toolbarSearchList.erase(window);
}


void Blackbox::removeSlitSearch(Window window) {
  slitSearchList.erase(window);
}


void Blackbox::restart(const char *prog) {
  shutdown();

  if (prog) {
    execlp(prog, prog, NULL);
    perror(prog);
  }

  // fall back in case the above execlp doesn't work
  execvp(argv[0], argv);
  string name = basename(argv[0]);
  execvp(name.c_str(), argv);
}


void Blackbox::shutdown(void) {
  BaseDisplay::shutdown();

  XSetInputFocus(getXDisplay(), PointerRoot, None, CurrentTime);

  std::for_each(screenList.begin(), screenList.end(),
                std::mem_fun(&BScreen::shutdown));

  XSync(getXDisplay(), False);
}


/*
 * Save all values as they are so that the defaults will be written to the rc
 * file
 */
void Blackbox::save_rc(void) {
  config.setAutoSave(false);

  config.setValue("session.colorsPerChannel", resource.colors_per_channel);
  config.setValue("session.doubleClickInterval",
                  resource.double_click_interval);
  config.setValue("session.autoRaiseDelay",
                  ((resource.auto_raise_delay.tv_sec * 1000) +
                   (resource.auto_raise_delay.tv_usec / 1000)));
  config.setValue("session.cacheLife", resource.cache_life / 60000);
  config.setValue("session.cacheMax", resource.cache_max);
  config.setValue("session.styleFile", resource.style_file);
  
  std::for_each(screenList.begin(), screenList.end(),
                std::mem_fun(&BScreen::save_rc));
 
  config.setAutoSave(true);
  config.save();
}


void Blackbox::load_rc(void) {
  if (! config.load())
        config.create();
  
  string s;

  if (! config.getValue("session.colorsPerChannel",
                        resource.colors_per_channel))
    resource.colors_per_channel = 4;
  if (resource.colors_per_channel < 2) resource.colors_per_channel = 2;
  else if (resource.colors_per_channel > 6) resource.colors_per_channel = 6;

  if (config.getValue("session.styleFile", s))
    resource.style_file = expandTilde(s);
  else
    resource.style_file = DEFAULTSTYLE;

  if (! config.getValue("session.doubleClickInterval",
                       resource.double_click_interval));
    resource.double_click_interval = 250;

  if (! config.getValue("session.autoRaiseDelay",
                       resource.auto_raise_delay.tv_usec))
    resource.auto_raise_delay.tv_usec = 400;
  resource.auto_raise_delay.tv_sec = resource.auto_raise_delay.tv_usec / 1000;
  resource.auto_raise_delay.tv_usec -=
    (resource.auto_raise_delay.tv_sec * 1000);
  resource.auto_raise_delay.tv_usec *= 1000;

  if (! config.getValue("session.cacheLife", resource.cache_life))
    resource.cache_life = 5;
  resource.cache_life *= 60000;

  if (! config.getValue("session.cacheMax", resource.cache_max))
    resource.cache_max = 200;
}


void Blackbox::reconfigure(void) {
  reconfigure_wait = True;

  if (! timer->isTiming()) timer->start();
}


void Blackbox::real_reconfigure(void) {
  load_rc();
  
  std::for_each(menuTimestamps.begin(), menuTimestamps.end(),
                PointerAssassin());
  menuTimestamps.clear();

  gcCache()->purge();

  std::for_each(screenList.begin(), screenList.end(),
                std::mem_fun(&BScreen::reconfigure));
}


void Blackbox::checkMenu(void) {
  bool reread = False;
  MenuTimestampList::iterator it = menuTimestamps.begin();
  for(; it != menuTimestamps.end(); ++it) {
    MenuTimestamp *tmp = *it;
    struct stat buf;

    if (! stat(tmp->filename.c_str(), &buf)) {
      if (tmp->timestamp != buf.st_ctime)
        reread = True;
    } else {
      reread = True;
    }
  }

  if (reread) rereadMenu();
}


void Blackbox::rereadMenu(void) {
  reread_menu_wait = True;

  if (! timer->isTiming()) timer->start();
}


void Blackbox::real_rereadMenu(void) {
  std::for_each(menuTimestamps.begin(), menuTimestamps.end(),
                PointerAssassin());
  menuTimestamps.clear();

  std::for_each(screenList.begin(), screenList.end(),
                std::mem_fun(&BScreen::rereadMenu));
}


void Blackbox::saveStyleFilename(const string& filename) {
  assert(! filename.empty());
  resource.style_file = filename;
  config.setValue("session.styleFile", resource.style_file);
}


void Blackbox::addMenuTimestamp(const string& filename) {
  assert(! filename.empty());
  bool found = False;

  MenuTimestampList::iterator it = menuTimestamps.begin();
  for (; it != menuTimestamps.end() && ! found; ++it) {
    if ((*it)->filename == filename) found = True;
  }
  if (! found) {
    struct stat buf;

    if (! stat(filename.c_str(), &buf)) {
      MenuTimestamp *ts = new MenuTimestamp;

      ts->filename = filename;
      ts->timestamp = buf.st_ctime;

      menuTimestamps.push_back(ts);
    }
  }
}


void Blackbox::timeout(void) {
  if (reconfigure_wait)
    real_reconfigure();

  if (reread_menu_wait)
    real_rereadMenu();

  reconfigure_wait = reread_menu_wait = False;
}


void Blackbox::setFocusedWindow(BlackboxWindow *win) {
  if (focused_window && focused_window == win) // nothing to do
    return;

  BScreen *old_screen = 0;

  if (focused_window) {
    focused_window->setFocusFlag(False);
    old_screen = focused_window->getScreen();
  }

  if (win && ! win->isIconic()) {
    // the active screen is the one with the last focused window...
    // this will keep focus on this screen no matter where the mouse goes,
    // so multihead keybindings will continue to work on that screen until the
    // user focuses a window on a different screen.
    active_screen = win->getScreen();
    focused_window = win;
  } else {
    focused_window = 0;
    if (! old_screen) {
      if (active_screen) {
        // set input focus to the toolbar of the screen with mouse
        XSetInputFocus(getXDisplay(),
                       active_screen->getToolbar()->getWindowID(),
                       RevertToPointerRoot, CurrentTime);
      } else {
        // set input focus to the toolbar of the first managed screen
        XSetInputFocus(getXDisplay(),
                       screenList.front()->getToolbar()->getWindowID(),
                       RevertToPointerRoot, CurrentTime);
      }
    } else {
      // set input focus to the toolbar of the last screen
      XSetInputFocus(getXDisplay(), old_screen->getToolbar()->getWindowID(),
                     RevertToPointerRoot, CurrentTime);
    }
  }

  if (active_screen && active_screen->isScreenManaged()) {
    active_screen->getToolbar()->redrawWindowLabel(True);
    active_screen->updateNetizenWindowFocus();
  }

  if (old_screen && old_screen != active_screen) {
    old_screen->getToolbar()->redrawWindowLabel(True);
    old_screen->updateNetizenWindowFocus();
  }
}
