// openbox.cc for Openbox
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
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#ifdef    SHAPE
#include <X11/extensions/shape.h>
#endif // SHAPE

#include "i18n.h"
#include "openbox.h"
#include "Basemenu.h"
#include "Clientmenu.h"
#include "Rootmenu.h"
#include "Screen.h"

#ifdef    SLIT
#include "Slit.h"
#endif // SLIT

#include "Toolbar.h"
#include "Window.h"
#include "Workspace.h"
#include "Workspacemenu.h"
#include "Util.h"

#include <string>
#include <algorithm>

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

#ifdef    HAVE_SYS_PARAM_H
#  include <sys/param.h>
#endif // HAVE_SYS_PARAM_H

#ifndef   MAXPATHLEN
#define   MAXPATHLEN 255
#endif // MAXPATHLEN

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

#ifndef   HAVE_BASENAME
static inline char *basename (char *s) {
  char *save = s;

  while (*s) if (*s++ == '/') save = s;

  return save;
}
#endif // HAVE_BASENAME


// X event scanner for enter/leave notifies - adapted from twm
typedef struct scanargs {
  Window w;
  Bool leave, inferior, enter;
} scanargs;

static Bool queueScanner(Display *, XEvent *e, char *args) {
  if ((e->type == LeaveNotify) &&
      (e->xcrossing.window == ((scanargs *) args)->w) &&
      (e->xcrossing.mode == NotifyNormal)) {
    ((scanargs *) args)->leave = True;
    ((scanargs *) args)->inferior = (e->xcrossing.detail == NotifyInferior);
  } else if ((e->type == EnterNotify) &&
             (e->xcrossing.mode == NotifyUngrab)) {
    ((scanargs *) args)->enter = True;
  }

  return False;
}

Openbox *openbox;


Openbox::Openbox(int m_argc, char **m_argv, char *dpy_name, char *rc)
  : BaseDisplay(m_argv[0], dpy_name) {
  grab();

  if (! XSupportsLocale())
    fprintf(stderr, "X server does not support locale\n");

  if (XSetLocaleModifiers("") == NULL)
    fprintf(stderr, "cannot set locale modifiers\n");

  ::openbox = this;
  argc = m_argc;
  argv = m_argv;
  if (rc == NULL) {
    char *homedir = getenv("HOME");

    rc_file = new char[strlen(homedir) + strlen("/.openbox/rc") + 1];
    sprintf(rc_file, "%s/.openbox", homedir);

    // try to make sure the ~/.openbox directory exists
    mkdir(rc_file, S_IREAD | S_IWRITE | S_IEXEC | S_IRGRP | S_IWGRP | S_IXGRP |
          S_IROTH | S_IWOTH | S_IXOTH);
    
    sprintf(rc_file, "%s/.openbox/rc", homedir);
  } else {
    rc_file = bstrdup(rc);
  }
  config.setFile(rc_file);

  no_focus = False;

  resource.menu_file = resource.style_file = (char *) 0;
  resource.titlebar_layout = (char *) NULL;
  resource.auto_raise_delay.tv_sec = resource.auto_raise_delay.tv_usec = 0;

  focused_window = masked_window = (OpenboxWindow *) 0;
  masked = None;

  windowSearchList = new LinkedList<WindowSearch>;
  menuSearchList = new LinkedList<MenuSearch>;

#ifdef    SLIT
  slitSearchList = new LinkedList<SlitSearch>;
#endif // SLIT

  toolbarSearchList = new LinkedList<ToolbarSearch>;
  groupSearchList = new LinkedList<WindowSearch>;

  menuTimestamps = new LinkedList<MenuTimestamp>;

  load_rc();

#ifdef    HAVE_GETPID
  openbox_pid = XInternAtom(getXDisplay(), "_BLACKBOX_PID", False);
#endif // HAVE_GETPID

  screenList = new LinkedList<BScreen>;
  for (int i = 0; i < getNumberOfScreens(); i++) {
    BScreen *screen = new BScreen(*this, i, config);

    if (! screen->isScreenManaged()) {
      delete screen;
      continue;
    }

    screenList->insert(screen);
  }

  if (! screenList->count()) {
    fprintf(stderr,
	    i18n->getMessage(openboxSet, openboxNoManagableScreens,
	       "Openbox::Openbox: no managable screens found, aborting.\n"));
    ::exit(3);
  }

  XSynchronize(getXDisplay(), False);
  XSync(getXDisplay(), False);

  reconfigure_wait = reread_menu_wait = False;

  timer = new BTimer(*this, *this);
  timer->setTimeout(0);
  timer->fireOnce(True);

  ungrab();
}


Openbox::~Openbox(void) {
  while (screenList->count())
    delete screenList->remove(0);

  while (menuTimestamps->count()) {
    MenuTimestamp *ts = menuTimestamps->remove(0);

    if (ts->filename)
      delete [] ts->filename;

    delete ts;
  }

  if (resource.menu_file)
    delete [] resource.menu_file;

  if (resource.style_file)
    delete [] resource.style_file;

  delete timer;

  delete screenList;
  delete menuTimestamps;

  delete windowSearchList;
  delete menuSearchList;
  delete toolbarSearchList;
  delete groupSearchList;

  delete [] rc_file;

#ifdef    SLIT
  delete slitSearchList;
#endif // SLIT
}


void Openbox::process_event(XEvent *e) {
  if ((masked == e->xany.window) && masked_window &&
      (e->type == MotionNotify)) {
    last_time = e->xmotion.time;
    masked_window->motionNotifyEvent(&e->xmotion);

    return;
  }

  switch (e->type) {
  case ButtonPress: {
    // strip the lock key modifiers
    e->xbutton.state &= ~(NumLockMask | ScrollLockMask | LockMask);

    last_time = e->xbutton.time;

    OpenboxWindow *win = (OpenboxWindow *) 0;
    Basemenu *menu = (Basemenu *) 0;

#ifdef    SLIT
    Slit *slit = (Slit *) 0;
#endif // SLIT

    Toolbar *tbar = (Toolbar *) 0;

    if ((win = searchWindow(e->xbutton.window))) {
      win->buttonPressEvent(&e->xbutton);

      if (e->xbutton.button == 1)
	win->installColormap(True);
    } else if ((menu = searchMenu(e->xbutton.window))) {
      menu->buttonPressEvent(&e->xbutton);

#ifdef    SLIT
    } else if ((slit = searchSlit(e->xbutton.window))) {
      slit->buttonPressEvent(&e->xbutton);
#endif // SLIT

    } else if ((tbar = searchToolbar(e->xbutton.window))) {
      tbar->buttonPressEvent(&e->xbutton);
    } else {
      LinkedListIterator<BScreen> it(screenList);
      BScreen *screen = it.current();
      for (; screen; it++, screen = it.current()) {
	if (e->xbutton.window == screen->getRootWindow()) {
	  if (e->xbutton.button == 1) {
            if (! screen->isRootColormapInstalled())
	      screen->getImageControl()->installRootColormap();

	    if (screen->getWorkspacemenu()->isVisible())
	      screen->getWorkspacemenu()->hide();

            if (screen->getRootmenu()->isVisible())
              screen->getRootmenu()->hide();
          } else if (e->xbutton.button == 2) {
	    int mx = e->xbutton.x_root -
	      (screen->getWorkspacemenu()->getWidth() / 2);
	    int my = e->xbutton.y_root -
	      (screen->getWorkspacemenu()->getTitleHeight() / 2);

	    if (mx < 0) mx = 0;
	    if (my < 0) my = 0;

	    if (mx + screen->getWorkspacemenu()->getWidth() >
		screen->size().w())
	      mx = screen->size().w() -
		screen->getWorkspacemenu()->getWidth() -
		screen->getBorderWidth();

	    if (my + screen->getWorkspacemenu()->getHeight() >
		screen->size().h())
	      my = screen->size().h() -
		screen->getWorkspacemenu()->getHeight() -
		screen->getBorderWidth();

	    screen->getWorkspacemenu()->move(mx, my);

	    if (! screen->getWorkspacemenu()->isVisible()) {
	      screen->getWorkspacemenu()->removeParent();
	      screen->getWorkspacemenu()->show();
	    }
	  } else if (e->xbutton.button == 3) {
	    int mx = e->xbutton.x_root -
	      (screen->getRootmenu()->getWidth() / 2);
	    int my = e->xbutton.y_root -
	      (screen->getRootmenu()->getTitleHeight() / 2);

	    if (mx < 0) mx = 0;
	    if (my < 0) my = 0;

	    if (mx + screen->getRootmenu()->getWidth() > screen->size().w())
	      mx = screen->size().w() -
		screen->getRootmenu()->getWidth() -
		screen->getBorderWidth();

	    if (my + screen->getRootmenu()->getHeight() > screen->size().h())
		my = screen->size().h() -
		  screen->getRootmenu()->getHeight() -
		  screen->getBorderWidth();

	    screen->getRootmenu()->move(mx, my);

	    if (! screen->getRootmenu()->isVisible()) {
	      checkMenu();
	      screen->getRootmenu()->show();
	    }
          } else if (e->xbutton.button == 4) {
            if ((screen->getCurrentWorkspaceID() + 1) >
                screen->getWorkspaceCount() - 1)
              screen->changeWorkspaceID(0);
            else
              screen->changeWorkspaceID(screen->getCurrentWorkspaceID() + 1);
          } else if (e->xbutton.button == 5) {
            if ((screen->getCurrentWorkspaceID() - 1) < 0)
              screen->changeWorkspaceID(screen->getWorkspaceCount() - 1);
            else
              screen->changeWorkspaceID(screen->getCurrentWorkspaceID() - 1);
          }
        }
      }
    }

    break;
  }

  case ButtonRelease: {
    // strip the lock key modifiers
    e->xbutton.state &= ~(NumLockMask | ScrollLockMask | LockMask);

    last_time = e->xbutton.time;

    OpenboxWindow *win = (OpenboxWindow *) 0;
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
    OpenboxWindow *win = (OpenboxWindow *) 0;

#ifdef    SLIT
    Slit *slit = (Slit *) 0;
#endif // SLIT

    if ((win = searchWindow(e->xconfigurerequest.window))) {
      win->configureRequestEvent(&e->xconfigurerequest);

#ifdef    SLIT
    } else if ((slit = searchSlit(e->xconfigurerequest.window))) {
      slit->configureRequestEvent(&e->xconfigurerequest);
#endif // SLIT

    } else {
      grab();

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

      ungrab();
    }

    break;
  }

  case MapRequest: {
#ifdef    DEBUG
    fprintf(stderr,
	    i18n->getMessage(openboxSet, openboxMapRequest,
		 "Openbox::process_event(): MapRequest for 0x%lx\n"),
	    e->xmaprequest.window);
#endif // DEBUG

    OpenboxWindow *win = searchWindow(e->xmaprequest.window);

    if (! win)
      win = new OpenboxWindow(*this, e->xmaprequest.window);

    if ((win = searchWindow(e->xmaprequest.window)))
      win->mapRequestEvent(&e->xmaprequest);

    break;
  }

  case MapNotify: {
    OpenboxWindow *win = searchWindow(e->xmap.window);

    if (win)
      win->mapNotifyEvent(&e->xmap);

      break;
  }

  case UnmapNotify: {
    OpenboxWindow *win = (OpenboxWindow *) 0;

#ifdef    SLIT
    Slit *slit = (Slit *) 0;
#endif // SLIT

    if ((win = searchWindow(e->xunmap.window))) {
      win->unmapNotifyEvent(&e->xunmap);
      if (focused_window == win)
	focused_window = (OpenboxWindow *) 0;
#ifdef    SLIT
    } else if ((slit = searchSlit(e->xunmap.window))) {
      slit->removeClient(e->xunmap.window);
#endif // SLIT

    }

    break;
  }

  case DestroyNotify: {
    OpenboxWindow *win = (OpenboxWindow *) 0;

#ifdef    SLIT
    Slit *slit = (Slit *) 0;
#endif // SLIT

    if ((win = searchWindow(e->xdestroywindow.window))) {
      win->destroyNotifyEvent(&e->xdestroywindow);
      if (focused_window == win)
	focused_window = (OpenboxWindow *) 0;
#ifdef    SLIT
    } else if ((slit = searchSlit(e->xdestroywindow.window))) {
      slit->removeClient(e->xdestroywindow.window, False);
#endif // SLIT
    }

    break;
  }

  case MotionNotify: {
    // strip the lock key modifiers
    e->xbutton.state &= ~(NumLockMask | ScrollLockMask | LockMask);
    
    last_time = e->xmotion.time;

    OpenboxWindow *win = (OpenboxWindow *) 0;
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
      OpenboxWindow *win = searchWindow(e->xproperty.window);

      if (win)
	win->propertyNotifyEvent(e->xproperty.atom);
    }

    break;
  }

  case EnterNotify: {
    last_time = e->xcrossing.time;

    BScreen *screen = (BScreen *) 0;
    OpenboxWindow *win = (OpenboxWindow *) 0;
    Basemenu *menu = (Basemenu *) 0;
    Toolbar *tbar = (Toolbar *) 0;

#ifdef    SLIT
    Slit *slit = (Slit *) 0;
#endif // SLIT

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
	grab();

        if (((! sa.leave) || sa.inferior) && win->isVisible() &&
            win->setInputFocus())
	  win->installColormap(True);

        ungrab();
      }
    } else if ((menu = searchMenu(e->xcrossing.window))) {
      menu->enterNotifyEvent(&e->xcrossing);
    } else if ((tbar = searchToolbar(e->xcrossing.window))) {
      tbar->enterNotifyEvent(&e->xcrossing);
#ifdef    SLIT
    } else if ((slit = searchSlit(e->xcrossing.window))) {
      slit->enterNotifyEvent(&e->xcrossing);
#endif // SLIT
    }
    break;
  }

  case LeaveNotify: {
    last_time = e->xcrossing.time;

    OpenboxWindow *win = (OpenboxWindow *) 0;
    Basemenu *menu = (Basemenu *) 0;
    Toolbar *tbar = (Toolbar *) 0;

#ifdef    SLIT
    Slit *slit = (Slit *) 0;
#endif // SLIT

    if ((menu = searchMenu(e->xcrossing.window)))
      menu->leaveNotifyEvent(&e->xcrossing);
    else if ((win = searchWindow(e->xcrossing.window)))
      win->installColormap(False);
    else if ((tbar = searchToolbar(e->xcrossing.window)))
      tbar->leaveNotifyEvent(&e->xcrossing);
#ifdef    SLIT
    else if ((slit = searchSlit(e->xcrossing.window)))
      slit->leaveNotifyEvent(&e->xcrossing);
#endif // SLIT

    break;
  }

  case Expose: {
    OpenboxWindow *win = (OpenboxWindow *) 0;
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
    if (e->xfocus.mode == NotifyUngrab || e->xfocus.detail == NotifyPointer)
      break;

    OpenboxWindow *win = searchWindow(e->xfocus.window);
    if (win && ! win->isFocused())
      setFocusedWindow(win);

    break;
  }

  case FocusOut:
    break;

  case ClientMessage: {
    if (e->xclient.format == 32) {
      if (e->xclient.message_type == getWMChangeStateAtom()) {
        OpenboxWindow *win = searchWindow(e->xclient.window);
        if (! win || ! win->validateClient()) return;

        if (e->xclient.data.l[0] == IconicState)
	  win->iconify();
        if (e->xclient.data.l[0] == NormalState)
          win->deiconify();
      } else if (e->xclient.message_type == getOpenboxChangeWorkspaceAtom()) {
	BScreen *screen = searchScreen(e->xclient.window);

	if (screen && e->xclient.data.l[0] >= 0 &&
	    e->xclient.data.l[0] < screen->getWorkspaceCount())
	  screen->changeWorkspaceID(e->xclient.data.l[0]);
      } else if (e->xclient.message_type == getOpenboxChangeWindowFocusAtom()) {
	OpenboxWindow *win = searchWindow(e->xclient.window);

	if (win && win->isVisible() && win->setInputFocus())
          win->installColormap(True);
      } else if (e->xclient.message_type == getOpenboxCycleWindowFocusAtom()) {
        BScreen *screen = searchScreen(e->xclient.window);

        if (screen) {
          if (! e->xclient.data.l[0])
            screen->prevFocus();
          else
            screen->nextFocus();
	}
      } else if (e->xclient.message_type == getOpenboxChangeAttributesAtom()) {
	OpenboxWindow *win = searchWindow(e->xclient.window);

	if (win && win->validateClient()) {
	  OpenboxHints net;
	  net.flags = e->xclient.data.l[0];
	  net.attrib = e->xclient.data.l[1];
	  net.workspace = e->xclient.data.l[2];
	  net.stack = e->xclient.data.l[3];
	  net.decoration = e->xclient.data.l[4];

	  win->changeOpenboxHints(&net);
	}
      }
    }

    break;
  }


  default: {
#ifdef    SHAPE
    if (e->type == getShapeEventBase()) {
      XShapeEvent *shape_event = (XShapeEvent *) e;
      OpenboxWindow *win = (OpenboxWindow *) 0;

      if ((win = searchWindow(e->xany.window)) ||
	  (shape_event->kind != ShapeBounding))
	win->shapeEvent(shape_event);
    }
#endif // SHAPE

  }
  } // switch
}


Bool Openbox::handleSignal(int sig) {
  switch (sig) {
  case SIGHUP:
    reconfigure();
    break;

  case SIGUSR1:
    reload_rc();
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


BScreen *Openbox::searchScreen(Window window) {
  LinkedListIterator<BScreen> it(screenList);

  for (BScreen *curr = it.current(); curr; it++, curr = it.current()) {
    if (curr->getRootWindow() == window) {
      return curr;
    }
  }

  return (BScreen *) 0;
}


OpenboxWindow *Openbox::searchWindow(Window window) {
  LinkedListIterator<WindowSearch> it(windowSearchList);

  for (WindowSearch *tmp = it.current(); tmp; it++, tmp = it.current()) {
      if (tmp->getWindow() == window) {
	return tmp->getData();
      }
  }

  return (OpenboxWindow *) 0;
}


OpenboxWindow *Openbox::searchGroup(Window window, OpenboxWindow *win) {
  OpenboxWindow *w = (OpenboxWindow *) 0;
  LinkedListIterator<WindowSearch> it(groupSearchList);

  for (WindowSearch *tmp = it.current(); tmp; it++, tmp = it.current()) {
    if (tmp->getWindow() == window) {
      w = tmp->getData();
      if (w->getClientWindow() != win->getClientWindow())
        return win;
    }
  }

  return (OpenboxWindow *) 0;
}


Basemenu *Openbox::searchMenu(Window window) {
  LinkedListIterator<MenuSearch> it(menuSearchList);

  for (MenuSearch *tmp = it.current(); tmp; it++, tmp = it.current()) {
    if (tmp->getWindow() == window)
      return tmp->getData();
  }

  return (Basemenu *) 0;
}


Toolbar *Openbox::searchToolbar(Window window) {
  LinkedListIterator<ToolbarSearch> it(toolbarSearchList);

  for (ToolbarSearch *tmp = it.current(); tmp; it++, tmp = it.current()) {
    if (tmp->getWindow() == window)
      return tmp->getData();
  }

  return (Toolbar *) 0;
}


#ifdef    SLIT
Slit *Openbox::searchSlit(Window window) {
  LinkedListIterator<SlitSearch> it(slitSearchList);

  for (SlitSearch *tmp = it.current(); tmp; it++, tmp = it.current()) {
    if (tmp->getWindow() == window)
      return tmp->getData();
  }

  return (Slit *) 0;
}
#endif // SLIT


void Openbox::saveWindowSearch(Window window, OpenboxWindow *data) {
  windowSearchList->insert(new WindowSearch(window, data));
}


void Openbox::saveGroupSearch(Window window, OpenboxWindow *data) {
  groupSearchList->insert(new WindowSearch(window, data));
}


void Openbox::saveMenuSearch(Window window, Basemenu *data) {
  menuSearchList->insert(new MenuSearch(window, data));
}


void Openbox::saveToolbarSearch(Window window, Toolbar *data) {
  toolbarSearchList->insert(new ToolbarSearch(window, data));
}


#ifdef    SLIT
void Openbox::saveSlitSearch(Window window, Slit *data) {
  slitSearchList->insert(new SlitSearch(window, data));
}
#endif // SLIT


void Openbox::removeWindowSearch(Window window) {
  LinkedListIterator<WindowSearch> it(windowSearchList);
  for (WindowSearch *tmp = it.current(); tmp; it++, tmp = it.current()) {
    if (tmp->getWindow() == window) {
      windowSearchList->remove(tmp);
      delete tmp;
      break;
    }
  }
}


void Openbox::removeGroupSearch(Window window) {
  LinkedListIterator<WindowSearch> it(groupSearchList);
  for (WindowSearch *tmp = it.current(); tmp; it++, tmp = it.current()) {
    if (tmp->getWindow() == window) {
      groupSearchList->remove(tmp);
      delete tmp;
      break;
    }
  }
}


void Openbox::removeMenuSearch(Window window) {
  LinkedListIterator<MenuSearch> it(menuSearchList);
  for (MenuSearch *tmp = it.current(); tmp; it++, tmp = it.current()) {
    if (tmp->getWindow() == window) {
      menuSearchList->remove(tmp);
      delete tmp;
      break;
    }
  }
}


void Openbox::removeToolbarSearch(Window window) {
  LinkedListIterator<ToolbarSearch> it(toolbarSearchList);
  for (ToolbarSearch *tmp = it.current(); tmp; it++, tmp = it.current()) {
    if (tmp->getWindow() == window) {
      toolbarSearchList->remove(tmp);
      delete tmp;
      break;
    }
  }
}


#ifdef    SLIT
void Openbox::removeSlitSearch(Window window) {
  LinkedListIterator<SlitSearch> it(slitSearchList);
  for (SlitSearch *tmp = it.current(); tmp; it++, tmp = it.current()) {
    if (tmp->getWindow() == window) {
      slitSearchList->remove(tmp);
      delete tmp;
      break;
    }
  }
}
#endif // SLIT


void Openbox::restart(const char *prog) {
  shutdown();

  if (prog) {
    execlp(prog, prog, NULL);
    perror(prog);
  }

  // fall back in case the above execlp doesn't work
  execvp(argv[0], argv);
  execvp(basename(argv[0]), argv);
}


void Openbox::shutdown(void) {
  BaseDisplay::shutdown();

  XSetInputFocus(getXDisplay(), PointerRoot, None, CurrentTime);

  LinkedListIterator<BScreen> it(screenList);
  for (BScreen *s = it.current(); s; it++, s = it.current())
    s->shutdown();

  XSync(getXDisplay(), False);

  save_rc();
}


void Openbox::save_rc(void) {
  config.setAutoSave(false);
  
  config.setValue("session.menuFile", getMenuFilename());
  config.setValue("session.colorsPerChannel",
                  resource.colors_per_channel);
  config.setValue("session.doubleClickInterval",
                  (long)resource.double_click_interval);
  config.setValue("session.autoRaiseDelay",
          ((resource.auto_raise_delay.tv_sec * 1000) +
           (resource.auto_raise_delay.tv_usec / 1000)));
  config.setValue("session.cacheLife", (long)resource.cache_life / 60000);
  config.setValue("session.cacheMax", (long)resource.cache_max);

  LinkedListIterator<BScreen> it(screenList);
  for (BScreen *screen = it.current(); screen; it++, screen = it.current()) {
//  ScreenList::iterator it = screenList.begin();
//  for (; it != screenList.end(); ++it) {
//    BScreen *screen = *it;
    char rc_string[1024];
    const int screen_number = screen->getScreenNumber();

    config.setValue("session.opaqueMove",
                    (screen->doOpaqueMove()) ? "True" : "False");
    config.setValue("session.imageDither",
                    (screen->getImageControl()->doDither()) ? "True" : "False");

    sprintf(rc_string, "session.screen%d.fullMaximization", screen_number);
    config.setValue(rc_string, screen->doFullMax() ? "True" : "False");

    sprintf(rc_string, "session.screen%d.focusNewWindows", screen_number);
    config.setValue(rc_string, screen->doFocusNew() ? "True" : "False");

    sprintf(rc_string, "session.screen%d.focusLastWindow", screen_number);
    config.setValue(rc_string, screen->doFocusLast() ? "True" : "False");

    sprintf(rc_string, "session.screen%d.rowPlacementDirection", screen_number);
    config.setValue(rc_string,
                    screen->getRowPlacementDirection() == BScreen::LeftRight ?
                    "LeftToRight" : "RightToLeft");

    sprintf(rc_string, "session.screen%d.colPlacementDirection", screen_number);
    config.setValue(rc_string,
                    screen->getColPlacementDirection() == BScreen::TopBottom ?
                    "TopToBottom" : "BottomToTop");

    const char *placement;
    switch (screen->getPlacementPolicy()) {
    case BScreen::CascadePlacement: placement = "CascadePlacement"; break;
    case BScreen::BestFitPlacement: placement = "BestFitPlacement"; break;
    case BScreen::ColSmartPlacement: placement = "ColSmartPlacement"; break;
    default:
    case BScreen::RowSmartPlacement: placement = "RowSmartPlacement"; break;
    }
    sprintf(rc_string, "session.screen%d.windowPlacement", screen_number);
    config.setValue(rc_string, placement);

    sprintf(rc_string, "session.screen%d.focusModel", screen_number);
    config.setValue(rc_string,
                    (screen->isSloppyFocus() ?
                     (screen->doAutoRaise() ? "AutoRaiseSloppyFocus" :
                      "SloppyFocus") : "ClickToFocus"));

    sprintf(rc_string, "session.screen%d.workspaces", screen_number);
    config.setValue(rc_string, screen->getWorkspaceCount());

#ifdef    HAVE_STRFTIME
    sprintf(rc_string, "session.screen%d.strftimeFormat", screen_number);
    config.setValue(rc_string, screen->getStrftimeFormat());
#else // !HAVE_STRFTIME
    sprintf(rc_string, "session.screen%d.dateFormat", screen_number);
    config.setValue(rc_string, screen->getDateFormat() == B_EuropeanDate ?
                    "European" : "American");

    sprintf(rc_string, "session.screen%d.clockFormat", screen_number);
    config.setValue(rc_string, screen->isClock24Hour() ? 24 : 12);
#endif // HAVE_STRFTIME

    sprintf(rc_string, "session.screen%d.edgeSnapThreshold", screen_number);
    config.setValue(rc_string, screen->getEdgeSnapThreshold());

    // write out the user's workspace names
    int i, len = 0;
    for (i = 0; i < screen->getWorkspaceCount(); i++)
      len += strlen((screen->getWorkspace(i)->getName()) ?
        screen->getWorkspace(i)->getName() : "Null") + 1;

    char *resource_string = new char[len + 1024],
      *save_string = new char[len], *save_string_pos = save_string,
      *name_string_pos;
    if (save_string) {
      for (i = 0; i < screen->getWorkspaceCount(); i++) {
        len = strlen((screen->getWorkspace(i)->getName()) ?
         screen->getWorkspace(i)->getName() : "Null") + 1;
        name_string_pos =
          (char *) ((screen->getWorkspace(i)->getName()) ?
                    screen->getWorkspace(i)->getName() : "Null");
        
        while (--len) *(save_string_pos++) = *(name_string_pos++);
        *(save_string_pos++) = ',';
      }
    }

    *(--save_string_pos) = '\0';

    sprintf(resource_string, "session.screen%d.workspaceNames", screen_number);
    config.setValue(resource_string, save_string);

    delete [] resource_string;
    delete [] save_string;
/*
    std::string save_string = screen->getWorkspace(0)->getName();
    for (unsigned int i = 1; i < screen->getWorkspaceCount(); ++i) {
      save_string += ',';
      save_string += screen->getWorkspace(i)->getName();
    }

    char *resource_string = new char[save_string.length() + 48];
    sprintf(resource_string, "session.screen%d.workspaceNames", screen_number);
    config.setValue(rc_string, save_string);

    delete [] resource_string;*/
  }

  config.setAutoSave(true);
  config.save();
}

void Openbox::load_rc(void) {
  if (!config.load())
    return;

  std::string s;
  long l;
  
  if (resource.menu_file)
    delete [] resource.menu_file;
  if (config.getValue("session.menuFile", "Session.MenuFile", s))
    resource.menu_file = bstrdup(s.c_str());
  else
    resource.menu_file = bstrdup(DEFAULTMENU);

  if (config.getValue("session.colorsPerChannel", "Session.ColorsPerChannel",
                      l))
    resource.colors_per_channel = (l < 2 ? 2 : (l > 6 ? 6 : l)); // >= 2, <= 6
  else
    resource.colors_per_channel = 4;

  if (resource.style_file)
    delete [] resource.style_file;
  if (config.getValue("session.styleFile", "Session.StyleFile", s))
    resource.style_file = bstrdup(s.c_str());
  else
    resource.style_file = bstrdup(DEFAULTSTYLE);

  if (resource.titlebar_layout)
    delete [] resource.titlebar_layout;
  if (config.getValue("session.titlebarLayout", "Session.TitlebarLayout", s))
    resource.titlebar_layout = bstrdup(s.c_str());
  else
    resource.titlebar_layout = bstrdup("ILMC");

  if (config.getValue("session.doubleClickInterval",
                      "Session.DoubleClickInterval", l))
    resource.double_click_interval = l;
  else
    resource.double_click_interval = 250;

  if (!config.getValue("session.autoRaiseDelay", "Session.AutoRaiseDelay", l))
    resource.auto_raise_delay.tv_usec = l;
  else
    resource.auto_raise_delay.tv_usec = 400;
  resource.auto_raise_delay.tv_sec = resource.auto_raise_delay.tv_usec / 1000;
  resource.auto_raise_delay.tv_usec -=
    (resource.auto_raise_delay.tv_sec * 1000);
  resource.auto_raise_delay.tv_usec *= 1000;

  if (config.getValue("session.cacheLife", "Session.CacheLife", l))
    resource.cache_life = l;
  else
    resource.cache_life = 51;
  resource.cache_life *= 60000;

  if (config.getValue("session.cacheMax", "Session.CacheMax", l))
    resource.cache_max = l;
  else
    resource.cache_max = 200;
}


void Openbox::load_rc(BScreen *screen) {
  ASSERT (screen != NULL);
  const int screen_number = screen->getScreenNumber();
  ASSERT (screen_number >= 0);

  if (!config.load())
    return;

  std::string s;
  long l;
  bool b;
  char name_lookup[1024], class_lookup[1024];
   
  sprintf(name_lookup,  "session.screen%d.fullMaximization", screen_number);
  sprintf(class_lookup, "Session.Screen%d.FullMaximization", screen_number);
  if (config.getValue(name_lookup, class_lookup, b))
    screen->saveFullMax((Bool)b);
  else
    screen->saveFullMax(False);

  sprintf(name_lookup,  "session.screen%d.focusNewWindows", screen_number);
  sprintf(class_lookup, "Session.Screen%d.FocusNewWindows", screen_number);
  if (config.getValue(name_lookup, class_lookup, b))
    screen->saveFocusNew((Bool)b);
  else
    screen->saveFocusNew(False);
  
  sprintf(name_lookup,  "session.screen%d.focusLastWindow", screen_number);
  sprintf(class_lookup, "Session.Screen%d.focusLastWindow", screen_number);
  if (config.getValue(name_lookup, class_lookup, b))
    screen->saveFocusLast((Bool)b);
  else
    screen->saveFocusLast(False);
  
  sprintf(name_lookup,  "session.screen%d.rowPlacementDirection",
          screen_number);
  sprintf(class_lookup, "Session.Screen%d.RowPlacementDirection",
	  screen_number);
  if (config.getValue(name_lookup, class_lookup, s)) {
    if (0 == strncasecmp(s.c_str(), "righttoleft", s.length()))
      screen->saveRowPlacementDirection(BScreen::RightLeft);
    else
      screen->saveRowPlacementDirection(BScreen::LeftRight);
  } else
    screen->saveRowPlacementDirection(BScreen::LeftRight);
  
  sprintf(name_lookup,  "session.screen%d.colPlacementDirection",
	  screen_number);
  sprintf(class_lookup, "Session.Screen%d.ColPlacementDirection",
	  screen_number);
  if (config.getValue(name_lookup, class_lookup, s)) {
    if (0 == strncasecmp(s.c_str(), "bottomtotop", s.length()))
      screen->saveColPlacementDirection(BScreen::BottomTop);
    else
      screen->saveColPlacementDirection(BScreen::TopBottom);
  } else
    screen->saveColPlacementDirection(BScreen::TopBottom);

  sprintf(name_lookup,  "session.screen%d.workspaces", screen_number);
  sprintf(class_lookup, "Session.Screen%d.Workspaces", screen_number);
  if (config.getValue(name_lookup, class_lookup, l))
    screen->saveWorkspaces(l);
  else
    screen->saveWorkspaces(1);
  
  screen->removeWorkspaceNames();
  sprintf(name_lookup,  "session.screen%d.workspaceNames", screen_number);
  sprintf(class_lookup, "Session.Screen%d.WorkspaceNames", screen_number);
  if (config.getValue(name_lookup, class_lookup, s)) {
  //  for (int i = 0; i < screen->getNumberOfWorkspaces(); i++) {
    std::string::const_iterator it = s.begin(), end = s.end();
    while(1) {
      std::string::const_iterator tmp = it;// current string.begin()
      it = std::find(tmp, end, ',');       // look for comma between tmp and end
      std::string name(tmp, it);           // name = s[tmp:it]
      screen->addWorkspaceName(name.c_str());
      if (it == end)
        break;
      ++it;
    }
  }

  sprintf(name_lookup,  "session.screen%d.focusModel", screen_number);
  sprintf(class_lookup, "Session.Screen%d.FocusModel", screen_number);
  if (config.getValue(name_lookup, class_lookup, s)) {
    if (0 == strncasecmp(s.c_str(), "clicktofocus", s.length())) {
      screen->saveAutoRaise(False);
      screen->saveSloppyFocus(False);
    } else if (0 == strncasecmp(s.c_str(), "autoraisesloppyfocus",
                                s.length())) {
      screen->saveSloppyFocus(True);
      screen->saveAutoRaise(True);
    } else {
      screen->saveSloppyFocus(True);
      screen->saveAutoRaise(False);
    }
  } else {
    screen->saveSloppyFocus(True);
    screen->saveAutoRaise(False);
  }

  sprintf(name_lookup,  "session.screen%d.windowZones", screen_number);
  sprintf(class_lookup, "Session.Screen%d.WindowZones", screen_number);
  if (config.getValue(name_lookup, class_lookup, l))
    screen->saveWindowZones((l == 1 || l == 2 || l == 4) ? l : 1);
  else
    screen->saveWindowZones(1);
  
  sprintf(name_lookup,  "session.screen%d.windowPlacement", screen_number);
  sprintf(class_lookup, "Session.Screen%d.WindowPlacement", screen_number);
  if (config.getValue(name_lookup, class_lookup, s)) {
    if (0 == strncasecmp(s.c_str(), "RowSmartPlacement", s.length()))
      screen->savePlacementPolicy(BScreen::RowSmartPlacement);
    else if (0 == strncasecmp(s.c_str(), "ColSmartPlacement", s.length()))
      screen->savePlacementPolicy(BScreen::ColSmartPlacement);
    else if (0 == strncasecmp(s.c_str(), "BestFitPlacement", s.length()))
      screen->savePlacementPolicy(BScreen::BestFitPlacement);
    else
      screen->savePlacementPolicy(BScreen::CascadePlacement);
  } else
    screen->savePlacementPolicy(BScreen::RowSmartPlacement);

#ifdef    SLIT
#endif // SLIT

#ifdef    HAVE_STRFTIME
  sprintf(name_lookup,  "session.screen%d.strftimeFormat", screen_number);
  sprintf(class_lookup, "Session.Screen%d.StrftimeFormat", screen_number);
  if (config.getValue(name_lookup, class_lookup, s))
    screen->saveStrftimeFormat(s.c_str());
  else
    screen->saveStrftimeFormat("%I:%M %p");

#else //  HAVE_STRFTIME
  sprintf(name_lookup,  "session.screen%d.dateFormat", screen_number);
  sprintf(class_lookup, "Session.Screen%d.DateFormat", screen_number);
  if (config.getValue(name_lookup, class_lookup, s)) {
    if (strncasecmp(s.c_str(), "european", s.length()))
      screen->saveDateFormat(B_AmericanDate);
    else
      screen->saveDateFormat(B_EuropeanDate);
  } else
    screen->saveDateFormat(B_AmericanDate);

  sprintf(name_lookup,  "session.screen%d.clockFormat", screen_number);
  sprintf(class_lookup, "Session.Screen%d.ClockFormat", screen_number);
  if (config.getValue(name_lookup, class_lookup, l)) {
    if (clock == 24)
      screen->saveClock24Hour(True);
    else
      screen->saveClock24Hour(False);
  } else
    screen->saveClock24Hour(False);
#endif // HAVE_STRFTIME

  sprintf(name_lookup,  "session.screen%d.edgeSnapThreshold", screen_number);
  sprintf(class_lookup, "Session.Screen%d.EdgeSnapThreshold", screen_number);
  if (config.getValue(name_lookup, class_lookup, l))
    screen->saveEdgeSnapThreshold(l);
  else
    screen->saveEdgeSnapThreshold(4);

  sprintf(name_lookup,  "session.screen%d.imageDither", screen_number);
  sprintf(class_lookup, "Session.Screen%d.ImageDither", screen_number);
  if (config.getValue("session.imageDither", "Session.ImageDither", b))
    screen->saveImageDither((Bool)b);
  else
    screen->saveImageDither(True);

  sprintf(name_lookup, "session.screen%d.rootCommand", screen_number);
  sprintf(class_lookup, "Session.Screen%d.RootCommand", screen_number);
  if (config.getValue(name_lookup, class_lookup, s))
    screen->saveRootCommand(s.c_str());
  else
    screen->saveRootCommand(NULL);

  if (config.getValue("session.opaqueMove", "Session.OpaqueMove", b))
    screen->saveOpaqueMove((Bool)b);
  else
    screen->saveOpaqueMove(False);
}


void Openbox::reload_rc(void) {
  load_rc();
  reconfigure();
}


void Openbox::reconfigure(void) {
  reconfigure_wait = True;

  if (! timer->isTiming()) timer->start();
}


void Openbox::real_reconfigure(void) {
  grab();

  config.load();
  config.setValue("session.styleFile", resource.style_file);    // autosave's
  
  for (int i = 0, n = menuTimestamps->count(); i < n; i++) {
    MenuTimestamp *ts = menuTimestamps->remove(0);

    if (ts) {
      if (ts->filename)
	delete [] ts->filename;

      delete ts;
    }
  }

  LinkedListIterator<BScreen> it(screenList);
  for (BScreen *screen = it.current(); screen; it++, screen = it.current()) {
    screen->reconfigure();
  }

  ungrab();
}


void Openbox::checkMenu(void) {
  Bool reread = False;
  LinkedListIterator<MenuTimestamp> it(menuTimestamps);
  for (MenuTimestamp *tmp = it.current(); tmp && (! reread);
       it++, tmp = it.current()) {
    struct stat buf;

    if (! stat(tmp->filename, &buf)) {
      if (tmp->timestamp != buf.st_ctime)
        reread = True;
    } else {
      reread = True;
    }
  }

  if (reread) rereadMenu();
}


void Openbox::rereadMenu(void) {
  reread_menu_wait = True;

  if (! timer->isTiming()) timer->start();
}


void Openbox::real_rereadMenu(void) {
  for (int i = 0, n = menuTimestamps->count(); i < n; i++) {
    MenuTimestamp *ts = menuTimestamps->remove(0);

    if (ts) {
      if (ts->filename)
	delete [] ts->filename;

      delete ts;
    }
  }

  LinkedListIterator<BScreen> it(screenList);
  for (BScreen *screen = it.current(); screen; it++, screen = it.current())
    screen->rereadMenu();
}


void Openbox::saveStyleFilename(const char *filename) {
  if (resource.style_file)
    delete [] resource.style_file;

  resource.style_file = bstrdup(filename);
}


void Openbox::saveMenuFilename(const char *filename) {
  Bool found = False;

  LinkedListIterator<MenuTimestamp> it(menuTimestamps);
  for (MenuTimestamp *tmp = it.current(); tmp && (! found);
       it++, tmp = it.current()) {
    if (! strcmp(tmp->filename, filename)) found = True;
  }
  if (! found) {
    struct stat buf;

    if (! stat(filename, &buf)) {
      MenuTimestamp *ts = new MenuTimestamp;

      ts->filename = bstrdup(filename);
      ts->timestamp = buf.st_ctime;

      menuTimestamps->insert(ts);
    }
  }
}


void Openbox::timeout(void) {
  if (reconfigure_wait)
    real_reconfigure();

  if (reread_menu_wait)
    real_rereadMenu();

  reconfigure_wait = reread_menu_wait = False;
}


void Openbox::setFocusedWindow(OpenboxWindow *win) {
  BScreen *old_screen = (BScreen *) 0, *screen = (BScreen *) 0;
  OpenboxWindow *old_win = (OpenboxWindow *) 0;
  Toolbar *old_tbar = (Toolbar *) 0, *tbar = (Toolbar *) 0;
  Workspace *old_wkspc = (Workspace *) 0, *wkspc = (Workspace *) 0;

  if (focused_window) {
    old_win = focused_window;
    old_screen = old_win->getScreen();
    old_tbar = old_screen->getToolbar();
    old_wkspc = old_screen->getWorkspace(old_win->getWorkspaceNumber());

    old_win->setFocusFlag(False);
    old_wkspc->getMenu()->setItemSelected(old_win->getWindowNumber(), False);
  }

  if (win && ! win->isIconic()) {
    screen = win->getScreen();
    tbar = screen->getToolbar();
    wkspc = screen->getWorkspace(win->getWorkspaceNumber());

    focused_window = win;

    win->setFocusFlag(True);
    wkspc->getMenu()->setItemSelected(win->getWindowNumber(), True);
  } else {
    focused_window = (OpenboxWindow *) 0;
  }

  if (tbar)
    tbar->redrawWindowLabel(True);
  if (screen)
    screen->updateNetizenWindowFocus();

  if (old_tbar && old_tbar != tbar)
    old_tbar->redrawWindowLabel(True);
  if (old_screen && old_screen != screen)
    old_screen->updateNetizenWindowFocus();
}
