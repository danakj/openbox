// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

extern "C" {
#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef    HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif // HAVE_UNISTD_H

#include "gettext.h"
#define _(str) gettext(str)
}

#include "screen.hh"
#include "client.hh"
#include "openbox.hh"
#include "frame.hh"
#include "bindings.hh"
#include "otk/display.hh"

static bool running;
static int anotherWMRunning(Display *display, XErrorEvent *) {
  printf(_("Another window manager already running on display %s.\n"),
         DisplayString(display));
  running = true;
  return -1;
}


namespace ob {


OBScreen::OBScreen(int screen, const otk::Configuration &config)
  : _number(screen),
    _root(screen)
{
  assert(screen >= 0); assert(screen < ScreenCount(otk::OBDisplay::display));
  _info = otk::OBDisplay::screenInfo(screen);

  ::running = false;
  XErrorHandler old = XSetErrorHandler(::anotherWMRunning);
  XSelectInput(otk::OBDisplay::display, _info->rootWindow(),
               OBScreen::event_mask);
  XSync(otk::OBDisplay::display, false);
  XSetErrorHandler(old);

  _managed = !::running;
  if (! _managed) return; // was unable to manage the screen

  printf(_("Managing screen %d: visual 0x%lx, depth %d\n"),
         _number, XVisualIDFromVisual(_info->visual()), _info->depth());

  Openbox::instance->property()->set(_info->rootWindow(),
                                     otk::OBProperty::openbox_pid,
                                     otk::OBProperty::Atom_Cardinal,
                                     (unsigned long) getpid());

  // set the mouse cursor for the root window (the default cursor)
  XDefineCursor(otk::OBDisplay::display, _info->rootWindow(),
                Openbox::instance->cursors().session);

  // initialize the shit that is used for all drawing on the screen
  _image_control = new otk::BImageControl(Openbox::instance->timerManager(),
                                          _info, true);
  _image_control->installRootColormap();
  _root_cmap_installed = True;

  // initialize the screen's style
  _style.setImageControl(_image_control);
  _style.load(config);

  
  // Set the netwm atoms for geomtery and viewport
  unsigned long geometry[] = { _info->width(),
                               _info->height() };
  Openbox::instance->property()->set(_info->rootWindow(),
                                     otk::OBProperty::net_desktop_geometry,
                                     otk::OBProperty::Atom_Cardinal,
                                     geometry, 2);
  unsigned long viewport[] = { 0, 0 };
  Openbox::instance->property()->set(_info->rootWindow(),
                                     otk::OBProperty::net_desktop_viewport,
                                     otk::OBProperty::Atom_Cardinal,
                                     viewport, 2);

  // create the window which gets focus when no clients get it
  XSetWindowAttributes attr;
  attr.override_redirect = true;
  _focuswindow = XCreateWindow(otk::OBDisplay::display, _info->rootWindow(),
                               -100, -100, 1, 1, 0, 0, InputOnly,
                               _info->visual(), CWOverrideRedirect, &attr);
  XMapWindow(otk::OBDisplay::display, _focuswindow);
  
  // these may be further updated if any pre-existing windows are found in
  // the manageExising() function
  setClientList();     // initialize the client lists, which will be empty
  calcArea();          // initialize the available working area
}


OBScreen::~OBScreen()
{
  if (! _managed) return;

  // unmanage all windows
  while (!clients.empty())
    unmanageWindow(clients.front());

  delete _image_control;
}


void OBScreen::manageExisting()
{
  unsigned int i, j, nchild;
  Window r, p, *children;
  XQueryTree(otk::OBDisplay::display, _info->rootWindow(), &r, &p,
             &children, &nchild);

  // preen the window list of all icon windows... for better dockapp support
  for (i = 0; i < nchild; i++) {
    if (children[i] == None) continue;

    XWMHints *wmhints = XGetWMHints(otk::OBDisplay::display,
                                    children[i]);

    if (wmhints) {
      if ((wmhints->flags & IconWindowHint) &&
          (wmhints->icon_window != children[i])) {
        for (j = 0; j < nchild; j++) {
          if (children[j] == wmhints->icon_window) {
            children[j] = None;
            break;
          }
        }
      }

      XFree(wmhints);
    }
  }

  // manage shown windows
  for (i = 0; i < nchild; ++i) {
    if (children[i] == None)
      continue;

    XWindowAttributes attrib;
    if (XGetWindowAttributes(otk::OBDisplay::display, children[i], &attrib)) {
      if (attrib.override_redirect) continue;

      if (attrib.map_state != IsUnmapped) {
        manageWindow(children[i]);
      }
    }
  }

  XFree(children);
}


//! Adds a window's strut to the screen's list of reserved spaces
void OBScreen::addStrut(otk::Strut *strut)
{
  _struts.push_back(strut);
}


//! Removes a window's strut from the screen's list of reserved spaces
void OBScreen::removeStrut(otk::Strut *strut)
{
  _struts.remove(strut);
}


void OBScreen::calcArea()
{
  otk::Rect old_area = _area;

/*
#ifdef    XINERAMA
  // reset to the full areas
  if (isXineramaActive())
    xineramaUsableArea = getXineramaAreas();
#endif // XINERAMA
*/
  
  /* these values represent offsets from the screen edge
   * we look for the biggest offset on each edge and then apply them
   * all at once
   * do not be confused by the similarity to the names of Rect's members
   */
  unsigned int current_left = 0, current_right = 0, current_top = 0,
    current_bottom = 0;

  StrutList::const_iterator it = _struts.begin(), end = _struts.end();

  for(; it != end; ++it) {
    otk::Strut *strut = *it;
    if (strut->left > current_left)
      current_left = strut->left;
    if (strut->top > current_top)
      current_top = strut->top;
    if (strut->right > current_right)
      current_right = strut->right;
    if (strut->bottom > current_bottom)
      current_bottom = strut->bottom;
  }

  _area.setRect(current_left, current_top,
                _info->width() - (current_left + current_right),
                _info->height() - (current_top + current_bottom));

/*
#ifdef    XINERAMA
  if (isXineramaActive()) {
    // keep each of the ximerama-defined areas inside the strut
    RectList::iterator xit, xend = xineramaUsableArea.end();
    for (xit = xineramaUsableArea.begin(); xit != xend; ++xit) {
      if (xit->x() < usableArea.x()) {
        xit->setX(usableArea.x());
        xit->setWidth(xit->width() - usableArea.x());
      }
      if (xit->y() < usableArea.y()) {
        xit->setY(usableArea.y());
        xit->setHeight(xit->height() - usableArea.y());
      }
      if (xit->x() + xit->width() > usableArea.width())
        xit->setWidth(usableArea.width() - xit->x());
      if (xit->y() + xit->height() > usableArea.height())
        xit->setHeight(usableArea.height() - xit->y());
    }
  }
#endif // XINERAMA
*/
  
  if (old_area != _area)
    // XXX: re-maximize windows

  setWorkArea();
}


void OBScreen::setClientList()
{
  Window *windows;

  // create an array of the window ids
  if (clients.size() > 0) {
    Window *win_it;
    
    windows = new Window[clients.size()];
    win_it = windows;
    ClientList::const_iterator it = clients.begin();
    const ClientList::const_iterator end = clients.end();
    for (; it != end; ++it, ++win_it)
      *win_it = (*it)->window();
  } else
    windows = (Window*) 0;

  Openbox::instance->property()->set(_info->rootWindow(),
                                     otk::OBProperty::net_client_list,
                                     otk::OBProperty::Atom_Window,
                                     windows, clients.size());

  if (clients.size())
    delete [] windows;

  setStackingList();
}


void OBScreen::setStackingList()
{
  // The below comment is wrong now hopefully :> but ill keep it here for
  // reference anyways
  /*
    Get the stacking order from all of the workspaces.
    We start with the current workspace so that the sticky windows will be
    in the right order on the current workspace.
  */
  /*
  Openbox::instance->property()->set(_info->getRootWindow(),
                                     otk::OBProperty::net_client_list_stacking,
                                     otk::OBProperty::Atom_Window,
                                     _stacking, _stacking.size());
  */
}


void OBScreen::setWorkArea() {
  unsigned long area[] = { _area.x(), _area.y(),
                           _area.width(), _area.height() };
  Openbox::instance->property()->set(_info->rootWindow(),
                                     otk::OBProperty::net_workarea,
                                     otk::OBProperty::Atom_Cardinal,
                                     area, 4);
  /*
  if (workspacesList.size() > 0) {
    unsigned long *dims = new unsigned long[4 * workspacesList.size()];
    for (unsigned int i = 0, m = workspacesList.size(); i < m; ++i) {
      // XXX: this could be different for each workspace
      const otk::Rect &area = availableArea();
      dims[(i * 4) + 0] = area.x();
      dims[(i * 4) + 1] = area.y();
      dims[(i * 4) + 2] = area.width();
      dims[(i * 4) + 3] = area.height();
    }
    xatom->set(getRootWindow(), otk::OBProperty::net_workarea,
               otk::OBProperty::Atom_Cardinal,
               dims, 4 * workspacesList.size());
    delete [] dims;
  } else
    xatom->set(getRootWindow(), otk::OBProperty::net_workarea,
               otk::OBProperty::Atom_Cardinal, 0, 0);
  */
}


void OBScreen::loadStyle(const otk::Configuration &config)
{
  _style.load(config);

  // XXX: make stuff redraw!
}


void OBScreen::manageWindow(Window window)
{
  OBClient *client = 0;
  XWMHints *wmhint;
  XSetWindowAttributes attrib_set;

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

  otk::OBDisplay::grab();

  // choose the events we want to receive on the CLIENT window
  attrib_set.event_mask = OBClient::event_mask;
  attrib_set.do_not_propagate_mask = OBClient::no_propagate_mask;
  XChangeWindowAttributes(otk::OBDisplay::display, window,
                          CWEventMask|CWDontPropagate, &attrib_set);

  // create the OBClient class, which gets all of the hints on the window
  client = new OBClient(_number, window);
  // register for events
  Openbox::instance->registerHandler(window, client);
  // add to the wm's map
  Openbox::instance->addClient(window, client);

  // we dont want a border on the client
  client->toggleClientBorder(false);
  
  // specify that if we exit, the window should not be destroyed and should be
  // reparented back to root automatically
  XChangeSaveSet(otk::OBDisplay::display, window, SetModeInsert);

  if (!client->positionRequested()) {
    // XXX: position the window intelligenty
  }

  // create the decoration frame for the client window
  client->frame = new OBFrame(client, &_style);

  // add to the wm's map
  Openbox::instance->addClient(client->frame->window(), client);
  Openbox::instance->addClient(client->frame->plate(), client);
  Openbox::instance->addClient(client->frame->titlebar(), client);
  Openbox::instance->addClient(client->frame->label(), client);
  Openbox::instance->addClient(client->frame->button_max(), client);
  Openbox::instance->addClient(client->frame->button_iconify(), client);
  Openbox::instance->addClient(client->frame->button_stick(), client);
  Openbox::instance->addClient(client->frame->button_close(), client);
  Openbox::instance->addClient(client->frame->handle(), client);
  Openbox::instance->addClient(client->frame->grip_left(), client);
  Openbox::instance->addClient(client->frame->grip_right(), client);

  // XXX: if on the current desktop..
  client->frame->show();
 
  // XXX: handle any requested states such as shaded/maximized

  otk::OBDisplay::ungrab();

  // add to the screen's list
  clients.push_back(client);
  // update the root properties
  setClientList();
}


void OBScreen::unmanageWindow(OBClient *client)
{
  OBFrame *frame = client->frame;

  // XXX: pass around focus if this window was focused

  // remove from the wm's map
  Openbox::instance->removeClient(client->window());
  Openbox::instance->removeClient(frame->window());
  Openbox::instance->removeClient(frame->plate());
  Openbox::instance->removeClient(frame->titlebar());
  Openbox::instance->removeClient(frame->label());
  Openbox::instance->removeClient(frame->button_max());
  Openbox::instance->removeClient(frame->button_iconify());
  Openbox::instance->removeClient(frame->button_stick());
  Openbox::instance->removeClient(frame->button_close());
  Openbox::instance->removeClient(frame->handle());
  Openbox::instance->removeClient(frame->grip_left());
  Openbox::instance->removeClient(frame->grip_right());
  // unregister for handling events
  Openbox::instance->clearHandler(client->window());
  
  // remove the window from our save set
  XChangeSaveSet(otk::OBDisplay::display, client->window(), SetModeDelete);

  // we dont want events no more
  XSelectInput(otk::OBDisplay::display, client->window(), NoEventMask);

  frame->hide();

  // give the client its border back
  client->toggleClientBorder(true);

  delete client->frame;
  client->frame = 0;

  // remove from the screen's list
  clients.remove(client);
  delete client;

  // update the root properties
  setClientList();
}

}
