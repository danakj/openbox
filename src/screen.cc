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
  : _number(screen)
{
  assert(screen >= 0); assert(screen < ScreenCount(otk::OBDisplay::display));
  _info = otk::OBDisplay::screenInfo(screen);

  ::running = false;
  XErrorHandler old = XSetErrorHandler(::anotherWMRunning);
  XSelectInput(otk::OBDisplay::display, _info->getRootWindow(),
               OBScreen::event_mask);
  XSync(otk::OBDisplay::display, false);
  XSetErrorHandler(old);

  _managed = !::running;
  if (! _managed) return; // was unable to manage the screen

  printf(_("Managing screen %d: visual 0x%lx, depth %d\n"),
         _number, XVisualIDFromVisual(_info->getVisual()), _info->getDepth());

  Openbox::instance->property()->set(_info->getRootWindow(),
                                     otk::OBProperty::openbox_pid,
                                     otk::OBProperty::Atom_Cardinal,
                                     (unsigned long) getpid());

  // set the mouse cursor for the root window (the default cursor)
  XDefineCursor(otk::OBDisplay::display, _info->getRootWindow(),
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
  unsigned long geometry[] = { _size.x(),
                               _size.y() };
  Openbox::instance->property()->set(_info->getRootWindow(),
                                     otk::OBProperty::net_desktop_geometry,
                                     otk::OBProperty::Atom_Cardinal,
                                     geometry, 2);
  unsigned long viewport[] = { 0, 0 };
  Openbox::instance->property()->set(_info->getRootWindow(),
                                     otk::OBProperty::net_desktop_viewport,
                                     otk::OBProperty::Atom_Cardinal,
                                     viewport, 2);

  // these may be further updated if any pre-existing windows are found in
  // the manageExising() function
  setClientList();     // initialize the client lists, which will be empty
  calcArea();          // initialize the available working area
}


OBScreen::~OBScreen()
{
  if (! _managed) return;

  // unmanage all windows
  while (!_clients.empty())
    unmanageWindow(_clients[0]);

  delete _image_control;
}


void OBScreen::manageExisting()
{
  unsigned int i, j, nchild;
  Window r, p, *children;
  XQueryTree(otk::OBDisplay::display, _info->getRootWindow(), &r, &p,
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
                _info->getWidth() - (current_left + current_right),
                _info->getHeight() - (current_top + current_bottom));

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
  if (_clients.size() > 0) {
    Window *win_it;
    
    windows = new Window[_clients.size()];
    win_it = windows;
    ClientList::const_iterator it = _clients.begin();
    const ClientList::const_iterator end = _clients.end();
    for (; it != end; ++it, ++win_it)
      *win_it = (*it)->window();
  } else
    windows = (Window*) 0;

  Openbox::instance->property()->set(_info->getRootWindow(),
                                     otk::OBProperty::net_client_list,
                                     otk::OBProperty::Atom_Window,
                                     windows, _clients.size());

  if (_clients.size())
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
  Openbox::instance->property()->set(_info->getRootWindow(),
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

  // choose the events we want to receive on the CLIENT window
  attrib_set.event_mask = OBClient::event_mask;
  attrib_set.do_not_propagate_mask = ButtonPressMask | ButtonReleaseMask |
                                     ButtonMotionMask;
  XChangeWindowAttributes(otk::OBDisplay::display, window,
                          CWEventMask|CWDontPropagate, &attrib_set);

  // create the OBClient class, which gets all of the hints on the window
  Openbox::instance->addClient(window, client = new OBClient(_number, window));

  // we dont want a border on the client
  XSetWindowBorderWidth(otk::OBDisplay::display, window, 0);
  
  // specify that if we exit, the window should not be destroyed and should be
  // reparented back to root automatically
  XChangeSaveSet(otk::OBDisplay::display, window, SetModeInsert);

  if (!client->positionRequested()) {
    // XXX: position the window intelligenty
  }

  // create the decoration frame for the client window
  client->frame = new OBFrame(client, &_style);

  // add all the client's decoration windows as event handlers for the client
  Openbox::instance->addClient(client->frame->window(), client);
  Openbox::instance->addClient(client->frame->titlebar(), client);
  Openbox::instance->addClient(client->frame->buttonIconify(), client);
  Openbox::instance->addClient(client->frame->buttonMax(), client);
  Openbox::instance->addClient(client->frame->buttonStick(), client);
  Openbox::instance->addClient(client->frame->buttonClose(), client);
  Openbox::instance->addClient(client->frame->label(), client);
  Openbox::instance->addClient(client->frame->handle(), client);
  Openbox::instance->addClient(client->frame->gripLeft(), client);
  Openbox::instance->addClient(client->frame->gripRight(), client);
  
  // XXX: if on the current desktop..
  XMapWindow(otk::OBDisplay::display, client->frame->window());
 
  // XXX: handle any requested states such as shaded/maximized


  _clients.push_back(client);
  setClientList();
}


void OBScreen::unmanageWindow(OBClient *client)
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
  // remove the frame's decor elements as event handlers for the client
  Openbox::instance->removeClient(frame->window());
  Openbox::instance->removeClient(frame->titlebar());
  Openbox::instance->removeClient(frame->buttonIconify());
  Openbox::instance->removeClient(frame->buttonMax());
  Openbox::instance->removeClient(frame->buttonStick());
  Openbox::instance->removeClient(frame->buttonClose());
  Openbox::instance->removeClient(frame->label());
  Openbox::instance->removeClient(frame->handle());
  Openbox::instance->removeClient(frame->gripLeft());
  Openbox::instance->removeClient(frame->gripRight());

  delete client->frame;
  client->frame = 0;

  ClientList::iterator it = _clients.begin(), end = _clients.end();
  for (; it != end; ++it)
    if (*it == client) {
      _clients.erase(it);
      break;
    }
  delete client;

  setClientList();
}

}
