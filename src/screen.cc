// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

extern "C" {
#include "gettext.h"
#define _(str) gettext(str)
}

#include "screen.hh"
#include "client.hh"
#include "openbox.hh"
#include "otk/display.hh"

static bool running;
static int anotherWMRunning(Display *display, XErrorEvent *) {
  printf(_("Another window manager already running on display %s.\n"),
         DisplayString(display));
  running = true;
  return -1;
}


namespace ob {


OBScreen::OBScreen(int screen)
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

#ifdef    HAVE_GETPID
  Openbox::instance->property()->set(_info->getRootWindow(),
                                     otk::OBProperty::openbox_pid,
                                     otk::OBProperty::Atom_Cardinal,
                                     (unsigned long) getpid());
#endif // HAVE_GETPID

  // set the mouse cursor for the root window (the default cursor)
  XDefineCursor(otk::OBDisplay::display, _info->getRootWindow(),
                Openbox::instance->cursors().session);
  
  _image_control = new otk::BImageControl(Openbox::instance->timerManager(),
                                          _info, true);
  _image_control->installRootColormap();
  _root_cmap_installed = True;

  
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

  manageExisting();

  // XXX: "change to" the first workspace to initialize stuff
}


OBScreen::~OBScreen()
{
  if (! _managed) return;

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
        // XXX: manageWindow(children[i]);
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


}
