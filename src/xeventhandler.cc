// -*- mode: C++; indent-tabs-mode: nil; -*-

#include "xeventhandler.hh"
#include "otk/display.hh"
#include "otk/rect.hh"

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
/*
  BlackboxWindow *win = (BlackboxWindow *) 0;

  if ((win = searchWindow(e->xcrossing.window)))
    win->leaveNotifyEvent(&e->xcrossing);
*/
}


void OBXEventHandler::configureRequest(const XConfigureRequestEvent &e)
{
  (void)e;
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


void OBXEventHandler::mapRequest(const XMapRequestEvent &e)
{
#ifdef    DEBUG
  printf("MapRequest for 0x%lx\n", e.window);
#endif // DEBUG
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
  (void)e;
/*
  BlackboxWindow *win = (BlackboxWindow *) 0;
  BScreen *screen = (BScreen *) 0;

  if ((win = searchWindow(e->xunmap.window))) {
    win->unmapNotifyEvent(&e->xunmap);
  } else if ((screen = searchSystrayWindow(e->xunmap.window))) {
    screen->removeSystrayWindow(e->xunmap.window);
  }
*/
}


void OBXEventHandler::destroyNotify(const XDestroyWindowEvent &e)
{
  (void)e;
/*
  BlackboxWindow *win = (BlackboxWindow *) 0;
  BScreen *screen = (BScreen *) 0;
  BWindowGroup *group = (BWindowGroup *) 0;

  if ((win = searchWindow(e->xdestroywindow.window))) {
    win->destroyNotifyEvent(&e->xdestroywindow);
  } else if ((group = searchGroup(e->xdestroywindow.window))) {
    delete group;
  } else if ((screen = searchSystrayWindow(e->xunmap.window))) {
    screen->removeSystrayWindow(e->xunmap.window);
  }
*/
}


void OBXEventHandler::reparentNotify(const XReparentEvent &e)
{
  (void)e;
  /*
    this event is quite rare and is usually handled in unmapNotify
    however, if the window is unmapped when the reparent event occurs
    the window manager never sees it because an unmap event is not sent
    to an already unmapped window.
  */
/*
  BlackboxWindow *win = searchWindow(e->xreparent.window);
  if (win)
    win->reparentNotifyEvent(&e->xreparent);
*/
}


void OBXEventHandler::propertyNotify(const XPropertyEvent &e)
{
  _lasttime = e.time;
/*
  BlackboxWindow *win = (BlackboxWindow *) 0;
  BScreen *screen = (BScreen *) 0;

  if ((win = searchWindow(e->xproperty.window)))
    win->propertyNotifyEvent(&e->xproperty);
  else if ((screen = searchScreen(e->xproperty.window)))
    screen->propertyNotifyEvent(&e->xproperty);
*/
}


void OBXEventHandler::expose(const XExposeEvent &first)
{
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
/*
    BlackboxWindow *win = (BlackboxWindow *) 0;

    if ((win = searchWindow(e->xexpose.window)))
      win->exposeEvent(&e->xexpose);
*/
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

  default:
#ifdef    SHAPE
    if (e.type == otk::OBDisplay::shapeEventBase())
      shapeEvent(e);
#endif // SHAPE
    break;
    
/*
  case ClientMessage: {
    if (e->xclient.format == 32) {
      if (e->xclient.message_type == xatom->getAtom(XAtom::wm_change_state)) {
        // WM_CHANGE_STATE message
        BlackboxWindow *win = searchWindow(e->xclient.window);
        if (! win || ! win->validateClient()) return;

        if (e->xclient.data.l[0] == IconicState)
          win->iconify();
        if (e->xclient.data.l[0] == NormalState)
          win->deiconify();
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
                 xatom->getAtom(XAtom::blackbox_change_window_focus)) {
        // TEMP HACK TO KEEP BBKEYS WORKING
        BlackboxWindow *win = searchWindow(e->xclient.window);

        if (win && win->isVisible() && win->setInputFocus())
          win->installColormap(True);
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
                 xatom->getAtom(XAtom::blackbox_cycle_window_focus)) {
        // BLACKBOX_CYCLE_WINDOW_FOCUS
        BScreen *screen = searchScreen(e->xclient.window);

        if (screen) {
          if (! e->xclient.data.l[0])
            screen->prevFocus();
          else
            screen->nextFocus();
        }
      } else if (e->xclient.message_type == 
                 xatom->getAtom(XAtom::net_wm_desktop)) {
        // NET_WM_DESKTOP
        BlackboxWindow *win = searchWindow(e->xclient.window);

        if (win) {
          BScreen *screen = win->getScreen();
          unsigned long wksp = (unsigned) e->xclient.data.l[0];
          if (wksp < screen->getWorkspaceCount()) {
            if (win->isIconic()) win->deiconify(False, True);
            if (win->isStuck()) win->stick();
            if (wksp != screen->getCurrentWorkspaceID())
              win->withdraw();
            else
              win->show();
            screen->reassociateWindow(win, wksp, True);
          } else if (wksp == 0xfffffffe || // XXX: BUG, BUT DOING THIS SO KDE WORKS FOR NOW!!
                     wksp == 0xffffffff) {
            if (win->isIconic()) win->deiconify(False, True);
            if (! win->isStuck()) win->stick();
            if (! win->isVisible()) win->show();
          }
        }
      } else if (e->xclient.message_type == 
                 xatom->getAtom(XAtom::blackbox_change_attributes)) {
        // BLACKBOX_CHANGE_ATTRIBUTES
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
      } else if (e->xclient.message_type ==
                 xatom->getAtom(XAtom::net_wm_state)) {
        // NET_WM_STATE
        BlackboxWindow *win = searchWindow(e->xclient.window);
        if (win && win->validateClient()) {
          const Atom action = (Atom) e->xclient.data.l[0];
          const Atom state[] = { (Atom) e->xclient.data.l[1],
                                 (Atom) e->xclient.data.l[2] };
          
          for (int i = 0; i < 2; ++i) {
            if (! state[i])
              continue;

            if ((Atom) e->xclient.data.l[0] == 1) {
              // ADD
              if (state[i] == xatom->getAtom(XAtom::net_wm_state_modal)) {
                win->setModal(True);
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_maximized_vert)) {
                if (win->isMaximizedHoriz()) {
                  win->maximize(0); // unmaximize
                  win->maximize(1); // full
                } else if (! win->isMaximized()) {
                  win->maximize(2); // vert
                }
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_maximized_horz)) {
                if (win->isMaximizedVert()) {
                  win->maximize(0); // unmaximize
                  win->maximize(1); // full
                } else if (! win->isMaximized()) {
                  win->maximize(3); // horiz
                }
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_shaded)) {
                if (! win->isShaded())
                  win->shade();
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_skip_taskbar)) {
                win->setSkipTaskbar(True);
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_skip_pager)) {
                win->setSkipPager(True);
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_fullscreen)) {
                win->setFullscreen(True);
              }
            } else if (action == 0) {
              // REMOVE
              if (state[i] == xatom->getAtom(XAtom::net_wm_state_modal)) {
                win->setModal(False);
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_maximized_vert)) {
                if (win->isMaximizedFull()) {
                  win->maximize(0); // unmaximize
                  win->maximize(3); // horiz
                } else if (win->isMaximizedVert()) {
                  win->maximize(0); // unmaximize
                }
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_maximized_horz)) {
                if (win->isMaximizedFull()) {
                  win->maximize(0); // unmaximize
                  win->maximize(2); // vert
                } else if (win->isMaximizedHoriz()) {
                  win->maximize(0); // unmaximize
                }
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_shaded)) {
                if (win->isShaded())
                  win->shade();
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_skip_taskbar)) {
                win->setSkipTaskbar(False);
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_skip_pager)) {
                win->setSkipPager(False);
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_fullscreen)) {
                win->setFullscreen(False);
              }
            } else if (action == 2) {
              // TOGGLE
              if (state[i] == xatom->getAtom(XAtom::net_wm_state_modal)) {
                win->setModal(! win->isModal());
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_maximized_vert)) {
                if (win->isMaximizedFull()) {
                  win->maximize(0); // unmaximize
                  win->maximize(3); // horiz
                } else if (win->isMaximizedVert()) {
                  win->maximize(0); // unmaximize
                } else if (win->isMaximizedHoriz()) {
                  win->maximize(0); // unmaximize
                  win->maximize(1); // full
                } else {
                  win->maximize(2); // vert
                }
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_maximized_horz)) {
                if (win->isMaximizedFull()) {
                  win->maximize(0); // unmaximize
                  win->maximize(2); // vert
                } else if (win->isMaximizedHoriz()) {
                  win->maximize(0); // unmaximize
                } else if (win->isMaximizedVert()) {
                  win->maximize(0); // unmaximize
                  win->maximize(1); // full
                } else {
                  win->maximize(3); // horiz
                }
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_shaded)) {
                win->shade();
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_skip_taskbar)) {
                win->setSkipTaskbar(! win->skipTaskbar());
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_skip_pager)) {
                win->setSkipPager(! win->skipPager());
              } else if (state[i] ==
                         xatom->getAtom(XAtom::net_wm_state_fullscreen)) {
                win->setFullscreen(! win->isFullscreen());
              }
            }
          }
        }
      }
    }

    break;
  }

  case NoExpose:
  case ConfigureNotify:
  case MapNotify:
    break; // not handled, just ignore
*/
  } // switch
}


}
