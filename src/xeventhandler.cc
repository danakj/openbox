// -*- mode: C++; indent-tabs-mode: nil; -*-

#include "xeventhandler.hh"
#include "client.hh"
#include "frame.hh"
#include "openbox.hh"
#include "otk/display.hh"
#include "otk/rect.hh"

// XXX: REMOVE THIS SOON!!#!
#include "blackbox.hh"
#include "screen.hh"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

namespace ob {


OBXEventHandler::OBXEventHandler()
{
  _lasttime = 1; // 0 is CurrentTime, so set to minimum
}

void OBXEventHandler::buttonPress(const XButtonEvent &e)
{
  _lasttime = e.time;

}


void OBXEventHandler::buttonRelease(const XButtonEvent &e)
{
  _lasttime = e.time;

}


void OBXEventHandler::keyPress(const XKeyEvent &e)
{
  _lasttime = e.time;
}


void OBXEventHandler::motion(const XMotionEvent &e)
{
  _lasttime = e.time;

  // the pointer is on the wrong screen
  if (! e.same_screen) return;

}


void OBXEventHandler::enterNotify(const XCrossingEvent &e)
{
  _lasttime = e.time;

  OBClient *client = Openbox::instance->findClient(e.window);
  if (!client) return;
  
/*
  BScreen *screen = (BScreen *) 0;
  BlackboxWindow *win = (BlackboxWindow *) 0;

  if (e->xcrossing.mode == NotifyGrab) break;

  if ((e->xcrossing.window == e->xcrossing.root) &&
      (screen = searchScreen(e->xcrossing.window))) {
    screen->getImageControl()->installRootColormap();
  } else if ((win = searchWindow(e->xcrossing.window))) {
    if (! no_focus)
      win->enterNotifyEvent(&e->xcrossing);
  }
*/
}


void OBXEventHandler::leaveNotify(const XCrossingEvent &e)
{
  _lasttime = e.time;

  OBClient *client = Openbox::instance->findClient(e.window);
  if (!client) return;
  
/*
  BlackboxWindow *win = (BlackboxWindow *) 0;

  if ((win = searchWindow(e->xcrossing.window)))
    win->leaveNotifyEvent(&e->xcrossing);
*/
}


void OBXEventHandler::configureRequest(const XConfigureRequestEvent &e)
{
  OBClient *client = Openbox::instance->findClient(e.window);
  if (!client) return;
  
/*  BlackboxWindow *win = (BlackboxWindow *) 0;

  if ((win = searchWindow(e->xconfigurerequest.window))) {
    win->configureRequestEvent(&e->xconfigurerequest);
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

      XConfigureWindow(otk::OBDisplay::display, e->xconfigurerequest.window,
                       e->xconfigurerequest.value_mask, &xwc);
    }
  }
*/
}


// XXX: put this into the OBScreen or OBClient class!
void OBXEventHandler::manageWindow(int screen, Window window)
{
  OBClient *client = 0;
  XWMHints *wmhint;
  XSetWindowAttributes attrib_set;

  // XXX: manage the window, i.e. grab events n shit

  // is the window a docking app
  if ((wmhint = XGetWMHints(otk::OBDisplay::display, window))) {
    if ((wmhint->flags & StateHint) &&
        wmhint->initial_state == WithdrawnState) {
      //slit->addClient(w); // XXX: make dock apps work!
      XFree(wmhint);
      return;
    }
    XFree(wmhint);
  }

  // choose the events we want to receive on the CLIENT window
  attrib_set.event_mask = OBClient::event_mask;
  attrib_set.do_not_propagate_mask = ButtonPressMask | ButtonReleaseMask |
                                     ButtonMotionMask;
  XChangeWindowAttributes(otk::OBDisplay::display, window,
                          CWEventMask|CWDontPropagate, &attrib_set);

  // create the OBClient class, which gets all of the hints on the window
  Openbox::instance->addClient(window, client = new OBClient(screen, window));

  // we dont want a border on the client
  XSetWindowBorderWidth(otk::OBDisplay::display, window, 0);
  
  // specify that if we exit, the window should not be destroyed and should be
  // reparented back to root automatically
  XChangeSaveSet(otk::OBDisplay::display, window, SetModeInsert);

  if (!client->positionRequested()) {
    // XXX: position the window intelligenty
  }

  // XXX: store a style somewheres cooler!!
  otk::Style *style = ((Blackbox*)Openbox::instance)->
    searchScreen(RootWindow(otk::OBDisplay::display, screen))->
    getWindowStyle();
  client->frame = new OBFrame(client, style);
  
  // XXX: if on the current desktop..
  XMapWindow(otk::OBDisplay::display, client->frame->window());
 
  // XXX: handle any requested states such as shaded/maximized
}

// XXX: move this to the OBScreen or OBClient class!
void OBXEventHandler::unmanageWindow(OBClient *client)
{
  OBFrame *frame = client->frame;

  // XXX: pass around focus if this window was focused
  
  // remove the window from our save set
  XChangeSaveSet(otk::OBDisplay::display, client->window(), SetModeDelete);

  // we dont want events no more
  XSelectInput(otk::OBDisplay::display, client->window(), NoEventMask);

  XUnmapWindow(otk::OBDisplay::display, frame->window());
  
  // we dont want a border on the client
  XSetWindowBorderWidth(otk::OBDisplay::display, client->window(),
                        client->borderWidth());

  // remove the client class from the search list
  Openbox::instance->removeClient(client->window());

  delete client->frame;
  client->frame = 0;

  delete client;
}

void OBXEventHandler::mapRequest(const XMapRequestEvent &e)
{
#ifdef    DEBUG
  printf("MapRequest for 0x%lx\n", e.window);
#endif // DEBUG

  OBClient *client = Openbox::instance->findClient(e.window);

  if (client) {
    // XXX: uniconify and/or unshade the window
  } else {
    int screen = INT_MAX;

    for (int i = 0; i < ScreenCount(otk::OBDisplay::display); ++i)
      if (otk::OBDisplay::screenInfo(i)->getRootWindow() == e.parent) {
        screen = i;
        break;
      }

    if (screen >= ScreenCount(otk::OBDisplay::display)) {
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
      if (! XGetWindowAttributes(otk::OBDisplay::display, e.window,
                                 &wattrib)) {
        // failed to get the window attributes, perhaps the window has
        // now been destroyed?
        return;
      }

      for (int i = 0; i < ScreenCount(otk::OBDisplay::display); ++i)
        if (otk::OBDisplay::screenInfo(i)->getRootWindow() == wattrib.root) {
          screen = i;
          break;
        }
    }

    assert(screen < ScreenCount(otk::OBDisplay::display));

    manageWindow(screen, e.window);
  }
  
/*
  BlackboxWindow *win = searchWindow(e->xmaprequest.window);

  if (win) {
    bool focus = False;
    if (win->isIconic()) {
      win->deiconify();
      focus = True;
    }
    if (win->isShaded()) {
      win->shade();
      focus = True;
    }

    if (focus && (win->isTransient() || win->getScreen()->doFocusNew()) &&
        win->isVisible())
      win->setInputFocus();
  } else {
    BScreen *screen = searchScreen(e->xmaprequest.parent);

    if (! screen) {
*/
      /*
        we got a map request for a window who's parent isn't root. this
        can happen in only one circumstance:

        a client window unmapped a managed window, and then remapped it
        somewhere between unmapping the client window and reparenting it
        to root.

        regardless of how it happens, we need to find the screen that
        the window is on
      */
/*
      XWindowAttributes wattrib;
      if (! XGetWindowAttributes(otk::OBDisplay::display,
                                 e->xmaprequest.window,
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
*/
}


void OBXEventHandler::unmapNotify(const XUnmapEvent &e)
{
  OBClient *client = Openbox::instance->findClient(e.window);
  if (!client) return;
  
  unmanageWindow(client);
}


void OBXEventHandler::destroyNotify(const XDestroyWindowEvent &e)
{
  // XXX: window group leaders can come through here too!
  
  OBClient *client = Openbox::instance->findClient(e.window);
  if (!client) return;
  
  unmanageWindow(client);
}


void OBXEventHandler::reparentNotify(const XReparentEvent &e)
{
  /*
    this event is quite rare and is usually handled in unmapNotify
    however, if the window is unmapped when the reparent event occurs
    the window manager never sees it because an unmap event is not sent
    to an already unmapped window.
  */
  OBClient *client = Openbox::instance->findClient(e.window);
  if (!client) return;

/*
  BlackboxWindow *win = searchWindow(e->xreparent.window);
  if (win)
    win->reparentNotifyEvent(&e->xreparent);
*/
}


void OBXEventHandler::propertyNotify(const XPropertyEvent &e)
{
  _lasttime = e.time;

  OBClient *client = Openbox::instance->findClient(e.window);
  if (!client) return;

  client->update(e);
}


void OBXEventHandler::expose(const XExposeEvent &first)
{
  OBClient *client = Openbox::instance->findClient(first.window);
  if (!client) return;

  // compress expose events
  XEvent e; e.xexpose = first;
  unsigned int i = 0;
  otk::Rect area(e.xexpose.x, e.xexpose.y, e.xexpose.width,
                 e.xexpose.height);
  while (XCheckTypedWindowEvent(otk::OBDisplay::display,
                                e.xexpose.window, Expose, &e)) {
    i++;
    // merge expose area
    area |= otk::Rect(e.xexpose.x, e.xexpose.y, e.xexpose.width,
                      e.xexpose.height);
  }
  if ( i > 0 ) {
    // use the merged area
    e.xexpose.x = area.x();
    e.xexpose.y = area.y();
    e.xexpose.width = area.width();
    e.xexpose.height = area.height();
  }

  // XXX: make the decorations redraw!
}


void OBXEventHandler::colormapNotify(const XColormapEvent &e)
{
  (void)e;
/*
  BScreen *screen = searchScreen(e->xcolormap.window);
  if (screen)
    screen->setRootColormapInstalled((e->xcolormap.state ==
                                      ColormapInstalled) ? True : False);
*/
}


void OBXEventHandler::focusIn(const XFocusChangeEvent &e)
{
  if (e.detail != NotifyNonlinear &&
      e.detail != NotifyAncestor) {
    /*
      don't process FocusIns when:
      1. the new focus window isn't an ancestor or inferior of the old
      focus window (NotifyNonlinear)
      make sure to allow the FocusIn when the old focus window was an
      ancestor but didn't have a parent, such as root (NotifyAncestor)
    */
    return;
  }
/*
  BlackboxWindow *win = searchWindow(e.window);
  if (win) {
    if (! win->isFocused())
      win->setFocusFlag(True);
*/
    /*
      set the event window to None.  when the FocusOut event handler calls
      this function recursively, it uses this as an indication that focus
      has moved to a known window.
    */
/*
    e->xfocus.window = None;

    no_focus = False;   // focusing is back on
  }
*/
}


void OBXEventHandler::focusOut(const XFocusChangeEvent &e)
{
  if (e.detail != NotifyNonlinear) {
    /*
      don't process FocusOuts when:
      2. the new focus window isn't an ancestor or inferior of the old
      focus window (NotifyNonlinear)
    */
    return;
  }

/*
  BlackboxWindow *win = searchWindow(e->xfocus.window);
  if (win && win->isFocused()) {
*/
    /*
      before we mark "win" as unfocused, we need to verify that focus is
      going to a known location, is in a known location, or set focus
      to a known location.
    */
/*
    XEvent event;
    // don't check the current focus if FocusOut was generated during a grab
    bool check_focus = (e->xfocus.mode == NotifyNormal);
*/
    /*
      First, check if there is a pending FocusIn event waiting.  if there
      is, process it and determine if focus has moved to another window
      (the FocusIn event handler sets the window in the event
      structure to None to indicate this).
    */
/*
    if (XCheckTypedEvent(otk::OBDisplay::display, FocusIn, &event)) {

      process_event(&event);
      if (event.xfocus.window == None) {
        // focus has moved
        check_focus = False;
      }
    }

    if (check_focus) {
*/
      /*
        Second, we query the X server for the current input focus.
        to make sure that we keep a consistent state.
      */
/*
      BlackboxWindow *focus;
      Window w;
      int revert;
      XGetInputFocus(otk::OBDisplay::display, &w, &revert);
      focus = searchWindow(w);
      if (focus) {
*/
        /*
          focus got from "win" to "focus" under some very strange
          circumstances, and we need to make sure that the focus indication
          is correct.
        */
/*
        setFocusedWindow(focus);
      } else {
        // we have no idea where focus went... so we set it to somewhere
        setFocusedWindow(0);
      }
    }
  }
*/
}


#ifdef    SHAPE
void OBXEventHandler::shapeEvent(const XShapeEvent &e)
{
  XShapeEvent *shape_event = (XShapeEvent *) e;
  BlackboxWindow *win = searchWindow(e->xany.window);

  if (win && shape_event->kind == ShapeBounding)
    win->shapeEvent(shape_event);
}
#endif // SHAPE


void OBXEventHandler::clientMessage(const XClientMessageEvent &e)
{
  if (e.format != 32)
    return;
/*  
  } else if (e->xclient.message_type == 
             xatom->getAtom(XAtom::blackbox_change_workspace) || 
             e->xclient.message_type == 
             xatom->getAtom(XAtom::net_current_desktop)) {
    // NET_CURRENT_DESKTOP message
    BScreen *screen = searchScreen(e->xclient.window);

    unsigned int workspace = e->xclient.data.l[0];
    if (screen && workspace < screen->getWorkspaceCount())
      screen->changeWorkspaceID(workspace);
  } else if (e->xclient.message_type == 
             xatom->getAtom(XAtom::net_active_window)) {
    // NET_ACTIVE_WINDOW
    BlackboxWindow *win = searchWindow(e->xclient.window);

    if (win) {
      BScreen *screen = win->getScreen();

      if (win->isIconic())
        win->deiconify(False, False);
      if (! win->isStuck() &&
          (win->getWorkspaceNumber() != screen->getCurrentWorkspaceID())) {
        no_focus = True;
        screen->changeWorkspaceID(win->getWorkspaceNumber());
      }
      if (win->isVisible() && win->setInputFocus()) {
        win->getScreen()->getWorkspace(win->getWorkspaceNumber())->
          raiseWindow(win);
        win->installColormap(True);
      }
    }
  } else if (e->xclient.message_type == 
             xatom->getAtom(XAtom::net_number_of_desktops)) {
    // NET_NUMBER_OF_DESKTOPS
    BScreen *screen = searchScreen(e->xclient.window);
        
    if (e->xclient.data.l[0] > 0)
      screen->changeWorkspaceCount((unsigned) e->xclient.data.l[0]);
  } else if (e->xclient.message_type ==
             xatom->getAtom(XAtom::net_close_window)) {
    // NET_CLOSE_WINDOW
    BlackboxWindow *win = searchWindow(e->xclient.window);
    if (win && win->validateClient())
      win->close(); // could this be smarter?
  } else if (e->xclient.message_type ==
             xatom->getAtom(XAtom::net_wm_moveresize)) {
    // NET_WM_MOVERESIZE
    BlackboxWindow *win = searchWindow(e->xclient.window);
    if (win && win->validateClient()) {
      int x_root = e->xclient.data.l[0],
        y_root = e->xclient.data.l[1];
      if ((Atom) e->xclient.data.l[2] ==
          xatom->getAtom(XAtom::net_wm_moveresize_move)) {
        win->beginMove(x_root, y_root);
      } else {
        if ((Atom) e->xclient.data.l[2] ==
            xatom->getAtom(XAtom::net_wm_moveresize_size_topleft))
          win->beginResize(x_root, y_root, BlackboxWindow::TopLeft);
        else if ((Atom) e->xclient.data.l[2] ==
                 xatom->getAtom(XAtom::net_wm_moveresize_size_topright))
          win->beginResize(x_root, y_root, BlackboxWindow::TopRight);
        else if ((Atom) e->xclient.data.l[2] ==
                 xatom->getAtom(XAtom::net_wm_moveresize_size_bottomleft))
          win->beginResize(x_root, y_root, BlackboxWindow::BottomLeft);
        else if ((Atom) e->xclient.data.l[2] ==
                 xatom->getAtom(XAtom::net_wm_moveresize_size_bottomright))
          win->beginResize(x_root, y_root, BlackboxWindow::BottomRight);
      }
    }
  }
*/
}


void OBXEventHandler::handle(const XEvent &e)
{
  /* mouse button events can get translated into:
       press - button was pressed down
       release - buttons was released
       click - button was pressed and released on the same window
       double click - clicked twice on the same widget in a given time and area

     key events are only bindable to presses. key releases are ignored.

     mouse enter/leave can be bound to for the entire window
  */
  
  switch (e.type) {

    // These types of XEvent's can be bound to actions by the user, and so end
    // up getting passed off to the OBBindingMapper class at some point
  case ButtonPress:
    buttonPress(e.xbutton);
    break;
  case ButtonRelease:
    buttonRelease(e.xbutton);
    break;
  case KeyPress:
    keyPress(e.xkey);
    break;
  case MotionNotify:
    motion(e.xmotion);
    break;
  case EnterNotify:
    enterNotify(e.xcrossing);
    break;
  case LeaveNotify:
    leaveNotify(e.xcrossing);
    break;


    // These types of XEvent's can not be bound to actions by the user and so
    // will simply be handled in this class
  case ConfigureRequest:
    configureRequest(e.xconfigurerequest);
    break;

  case MapRequest:
    mapRequest(e.xmaprequest);
    break;
    
  case UnmapNotify:
    unmapNotify(e.xunmap);
    break;

  case DestroyNotify:
    destroyNotify(e.xdestroywindow);
    break;

  case ReparentNotify:
    reparentNotify(e.xreparent);
    break;

  case PropertyNotify:
    propertyNotify(e.xproperty);
    break;

  case Expose:
    expose(e.xexpose);
    break;

  case ColormapNotify:
    colormapNotify(e.xcolormap);
    break;
    
  case FocusIn:
    focusIn(e.xfocus);
    break;

  case FocusOut:
    focusOut(e.xfocus);
    break;

  case ClientMessage:
    clientMessage(e.xclient);

  default:
#ifdef    SHAPE
    if (e.type == otk::OBDisplay::shapeEventBase())
      shapeEvent(e);
#endif // SHAPE
    break;
    
/*
  case ClientMessage: {
    break;
  }

*/
  } // switch
}


}
