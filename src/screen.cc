// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

extern "C" {
#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef    HAVE_STRING_H
#  include <string.h>
#endif // HAVE_STRING_H

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
#include "python.hh"
#include "otk/display.hh"

#include <vector>
#include <algorithm>

static bool running;
static int anotherWMRunning(Display *display, XErrorEvent *) {
  printf(_("Another window manager already running on display %s.\n"),
         DisplayString(display));
  running = true;
  return -1;
}


namespace ob {


Screen::Screen(int screen)
  : WidgetBase(WidgetBase::Type_Root),
    _number(screen)
{
  assert(screen >= 0); assert(screen < ScreenCount(otk::Display::display));
  _info = otk::Display::screenInfo(screen);

  ::running = false;
  XErrorHandler old = XSetErrorHandler(::anotherWMRunning);
  XSelectInput(otk::Display::display, _info->rootWindow(),
               Screen::event_mask);
  XSync(otk::Display::display, false);
  XSetErrorHandler(old);

  _managed = !::running;
  if (! _managed) return; // was unable to manage the screen

  printf(_("Managing screen %d: visual 0x%lx, depth %d\n"),
         _number, XVisualIDFromVisual(_info->visual()), _info->depth());

  Openbox::instance->property()->set(_info->rootWindow(),
                                     otk::Property::openbox_pid,
                                     otk::Property::Atom_Cardinal,
                                     (unsigned long) getpid());

  // set the mouse cursor for the root window (the default cursor)
  XDefineCursor(otk::Display::display, _info->rootWindow(),
                Openbox::instance->cursors().session);

  // initialize the shit that is used for all drawing on the screen
  _image_control = new otk::ImageControl(Openbox::instance->timerManager(),
                                         _info, true);
  _image_control->installRootColormap();
  _root_cmap_installed = True;

  // initialize the screen's style
  _style.setImageControl(_image_control);
  otk::ustring stylepath;
  python_get_string("theme", &stylepath);
  otk::Configuration sconfig(false);
  sconfig.setFile(otk::expandTilde(stylepath.c_str()));
  if (!sconfig.load()) {
    sconfig.setFile(otk::expandTilde(DEFAULTSTYLE));
    if (!sconfig.load()) {
      printf(_("Unable to load default style: %s. Aborting.\n"), DEFAULTSTYLE);
      ::exit(1);
    }
  }
  _style.load(sconfig);

  // set up notification of netwm support
  changeSupportedAtoms();

  // Set the netwm properties for geometry
  unsigned long geometry[] = { _info->width(),
                               _info->height() };
  Openbox::instance->property()->set(_info->rootWindow(),
                                     otk::Property::net_desktop_geometry,
                                     otk::Property::Atom_Cardinal,
                                     geometry, 2);

  // Set the net_desktop_names property
  std::vector<otk::ustring> names;
  python_get_stringlist("desktop_names", &names);
  Openbox::instance->property()->set(_info->rootWindow(),
                                     otk::Property::net_desktop_names,
                                     otk::Property::utf8,
                                     names);
  // the above set() will cause the updateDesktopNames to fire right away so
  // we have a list of desktop names

  if (!python_get_long("number_of_desktops", &_num_desktops))
    _num_desktops = 1;
  changeNumDesktops(_num_desktops); // set the hint

  _desktop = 0;
  changeDesktop(0); // set the hint

  // create the window which gets focus when no clients get it
  XSetWindowAttributes attr;
  attr.override_redirect = true;
  _focuswindow = XCreateWindow(otk::Display::display, _info->rootWindow(),
                               -100, -100, 1, 1, 0, 0, InputOnly,
                               _info->visual(), CWOverrideRedirect, &attr);
  XMapWindow(otk::Display::display, _focuswindow);
  
  // these may be further updated if any pre-existing windows are found in
  // the manageExising() function
  changeClientList();  // initialize the client lists, which will be empty
  calcArea();          // initialize the available working area

  // register this class as the event handler for the root window
  Openbox::instance->registerHandler(_info->rootWindow(), this);

  // call the python Startup callbacks
  EventData data(_number, 0, EventShutdown, 0);
  Openbox::instance->bindings()->fireEvent(&data);
}


Screen::~Screen()
{
  if (! _managed) return;

  XSelectInput(otk::Display::display, _info->rootWindow(), NoEventMask);
  
  // unmanage all windows
  while (!clients.empty())
    unmanageWindow(clients.front());

  // call the python Shutdown callbacks
  EventData data(_number, 0, EventShutdown, 0);
  Openbox::instance->bindings()->fireEvent(&data);

  XDestroyWindow(otk::Display::display, _focuswindow);
  XDestroyWindow(otk::Display::display, _supportwindow);

  delete _image_control;
}


void Screen::manageExisting()
{
  unsigned int i, j, nchild;
  Window r, p, *children;
  XQueryTree(otk::Display::display, _info->rootWindow(), &r, &p,
             &children, &nchild);

  // preen the window list of all icon windows... for better dockapp support
  for (i = 0; i < nchild; i++) {
    if (children[i] == None) continue;

    XWMHints *wmhints = XGetWMHints(otk::Display::display,
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
    if (XGetWindowAttributes(otk::Display::display, children[i], &attrib)) {
      if (attrib.override_redirect) continue;

      if (attrib.map_state != IsUnmapped) {
        manageWindow(children[i]);
      }
    }
  }

  XFree(children);
}


void Screen::updateStrut()
{
  _strut.left = _strut.right = _strut.top = _strut.bottom = 0;

  Client::List::iterator it, end = clients.end();
  for (it = clients.begin(); it != end; ++it) {
    const otk::Strut &s = (*it)->strut();
    _strut.left = std::max(_strut.left, s.left);
    _strut.right = std::max(_strut.right, s.right);
    _strut.top = std::max(_strut.top, s.top);
    _strut.bottom = std::max(_strut.bottom, s.bottom);
  }
  calcArea();
}


void Screen::calcArea()
{
  otk::Rect old_area = _area;

/*
#ifdef    XINERAMA
  // reset to the full areas
  if (isXineramaActive())
    xineramaUsableArea = getXineramaAreas();
#endif // XINERAMA
*/
  
  _area.setRect(_strut.left, _strut.top,
                _info->width() - (_strut.left + _strut.right),
                _info->height() - (_strut.top + _strut.bottom));

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

  changeWorkArea();
}


void Screen::changeSupportedAtoms()
{
  // create the netwm support window
  _supportwindow = XCreateSimpleWindow(otk::Display::display,
                                       _info->rootWindow(),
                                       0, 0, 1, 1, 0, 0, 0);

  // set supporting window
  Openbox::instance->property()->set(_info->rootWindow(),
                                     otk::Property::net_supporting_wm_check,
                                     otk::Property::Atom_Window,
                                     _supportwindow);

  //set properties on the supporting window
  Openbox::instance->property()->set(_supportwindow,
                                     otk::Property::net_wm_name,
                                     otk::Property::utf8,
                                     "Openbox");
  Openbox::instance->property()->set(_supportwindow,
                                     otk::Property::net_supporting_wm_check,
                                     otk::Property::Atom_Window,
                                     _supportwindow);

  
  Atom supported[] = {
      otk::Property::net_current_desktop,
      otk::Property::net_number_of_desktops,
      otk::Property::net_desktop_geometry,
      otk::Property::net_desktop_viewport,
      otk::Property::net_active_window,
      otk::Property::net_workarea,
      otk::Property::net_client_list,
      otk::Property::net_client_list_stacking,
      otk::Property::net_desktop_names,
      otk::Property::net_close_window,
      otk::Property::net_wm_name,
      otk::Property::net_wm_visible_name,
      otk::Property::net_wm_icon_name,
      otk::Property::net_wm_visible_icon_name,
/*
      otk::Property::net_wm_desktop,
*/
      otk::Property::net_wm_strut,
      otk::Property::net_wm_window_type,
      otk::Property::net_wm_window_type_desktop,
      otk::Property::net_wm_window_type_dock,
      otk::Property::net_wm_window_type_toolbar,
      otk::Property::net_wm_window_type_menu,
      otk::Property::net_wm_window_type_utility,
      otk::Property::net_wm_window_type_splash,
      otk::Property::net_wm_window_type_dialog,
      otk::Property::net_wm_window_type_normal,
/*
      otk::Property::net_wm_moveresize,
      otk::Property::net_wm_moveresize_size_topleft,
      otk::Property::net_wm_moveresize_size_topright,
      otk::Property::net_wm_moveresize_size_bottomleft,
      otk::Property::net_wm_moveresize_size_bottomright,
      otk::Property::net_wm_moveresize_move,
*/
/*
      otk::Property::net_wm_allowed_actions,
      otk::Property::net_wm_action_move,
      otk::Property::net_wm_action_resize,
      otk::Property::net_wm_action_shade,
      otk::Property::net_wm_action_maximize_horz,
      otk::Property::net_wm_action_maximize_vert,
      otk::Property::net_wm_action_change_desktop,
      otk::Property::net_wm_action_close,
*/
      otk::Property::net_wm_state,
      otk::Property::net_wm_state_modal,
      otk::Property::net_wm_state_maximized_vert,
      otk::Property::net_wm_state_maximized_horz,
      otk::Property::net_wm_state_shaded,
      otk::Property::net_wm_state_skip_taskbar,
      otk::Property::net_wm_state_skip_pager,
      otk::Property::net_wm_state_hidden,
      otk::Property::net_wm_state_fullscreen,
      otk::Property::net_wm_state_above,
      otk::Property::net_wm_state_below,
    };
  const int num_supported = sizeof(supported)/sizeof(Atom);

  // convert to the atom values
  for (int i = 0; i < num_supported; ++i)
    supported[i] =
      Openbox::instance->property()->atom((otk::Property::Atoms)supported[i]);
  
  Openbox::instance->property()->set(_info->rootWindow(),
                                     otk::Property::net_supported,
                                     otk::Property::Atom_Atom,
                                     supported, num_supported);
}


void Screen::changeClientList()
{
  Window *windows;
  unsigned int size = clients.size();

  // create an array of the window ids
  if (size > 0) {
    Window *win_it;
    
    windows = new Window[size];
    win_it = windows;
    Client::List::const_iterator it = clients.begin();
    const Client::List::const_iterator end = clients.end();
    for (; it != end; ++it, ++win_it)
      *win_it = (*it)->window();
  } else
    windows = (Window*) 0;

  Openbox::instance->property()->set(_info->rootWindow(),
                                     otk::Property::net_client_list,
                                     otk::Property::Atom_Window,
                                     windows, size);

  if (size)
    delete [] windows;

  changeStackingList();
}


void Screen::changeStackingList()
{
  Window *windows;
  unsigned int size = _stacking.size();

  assert(size == clients.size()); // just making sure.. :)

  
  // create an array of the window ids
  if (size > 0) {
    Window *win_it;
    
    windows = new Window[size];
    win_it = windows;
    Client::List::const_iterator it = _stacking.begin();
    const Client::List::const_iterator end = _stacking.end();
    for (; it != end; ++it, ++win_it)
      *win_it = (*it)->window();
  } else
    windows = (Window*) 0;

  Openbox::instance->property()->set(_info->rootWindow(),
                                     otk::Property::net_client_list_stacking,
                                     otk::Property::Atom_Window,
                                     windows, size);

  if (size)
    delete [] windows;
}


void Screen::changeWorkArea() {
  unsigned long *dims = new unsigned long[4 * _num_desktops];
  for (long i = 0; i < _num_desktops; ++i) {
    // XXX: this could be different for each workspace
    dims[(i * 4) + 0] = _area.x();
    dims[(i * 4) + 1] = _area.y();
    dims[(i * 4) + 2] = _area.width();
    dims[(i * 4) + 3] = _area.height();
  }
  Openbox::instance->property()->set(_info->rootWindow(),
                                     otk::Property::net_workarea,
                                     otk::Property::Atom_Cardinal,
                                     dims, 4 * _num_desktops);
  delete [] dims;
}


void Screen::manageWindow(Window window)
{
  Client *client = 0;
  XWMHints *wmhint;
  XSetWindowAttributes attrib_set;

  otk::Display::grab();

  // is the window a docking app
  if ((wmhint = XGetWMHints(otk::Display::display, window))) {
    if ((wmhint->flags & StateHint) &&
        wmhint->initial_state == WithdrawnState) {
      //slit->addClient(w); // XXX: make dock apps work!
      otk::Display::ungrab();

      XFree(wmhint);
      return;
    }
    XFree(wmhint);
  }

  // choose the events we want to receive on the CLIENT window
  attrib_set.event_mask = Client::event_mask;
  attrib_set.do_not_propagate_mask = Client::no_propagate_mask;
  XChangeWindowAttributes(otk::Display::display, window,
                          CWEventMask|CWDontPropagate, &attrib_set);

  // create the Client class, which gets all of the hints on the window
  client = new Client(_number, window);
  // register for events
  Openbox::instance->registerHandler(window, client);
  // add to the wm's map
  Openbox::instance->addClient(window, client);

  // we dont want a border on the client
  client->toggleClientBorder(false);
  
  // specify that if we exit, the window should not be destroyed and should be
  // reparented back to root automatically
  XChangeSaveSet(otk::Display::display, window, SetModeInsert);

  if (!(Openbox::instance->state() == Openbox::State_Starting ||
        client->positionRequested())) {
    // position the window intelligenty .. hopefully :)
    // call the python PLACEWINDOW binding
    EventData data(_number, client, EventPlaceWindow, 0);
    Openbox::instance->bindings()->fireEvent(&data);
  }

  // create the decoration frame for the client window
  client->frame = new Frame(client, &_style);

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

  // reparent the client to the frame
  client->frame->grabClient();

  // if on the current desktop.. (or all desktops)
  if (client->desktop() == _desktop ||
      client->desktop() == (signed)0xffffffff) {
    client->frame->show();
  }
 
  // XXX: handle any requested states such as maximized

  otk::Display::ungrab();

  // add to the screen's list
  clients.push_back(client);
  // this puts into the stacking order, then raises it
  _stacking.push_back(client);
  restack(true, client);
  // update the root properties
  changeClientList();

  Openbox::instance->bindings()->grabButtons(true, client);

  // call the python NEWWINDOW binding
  EventData data(_number, client, EventNewWindow, 0);
  Openbox::instance->bindings()->fireEvent(&data);

#ifdef DEBUG
  printf("Managed window 0x%lx\n", window);
#endif
}


void Screen::unmanageWindow(Client *client)
{
  Frame *frame = client->frame;

  // call the python CLOSEWINDOW binding 
  EventData data(_number, client, EventCloseWindow, 0);
  Openbox::instance->bindings()->fireEvent(&data);

  Openbox::instance->bindings()->grabButtons(false, client);

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
  XChangeSaveSet(otk::Display::display, client->window(), SetModeDelete);

  // we dont want events no more
  XSelectInput(otk::Display::display, client->window(), NoEventMask);

  frame->hide();

  // give the client its border back
  client->toggleClientBorder(true);

  // reparent the window out of the frame
  frame->releaseClient();

  delete client->frame;
  client->frame = 0;

  // remove from the stacking order
  _stacking.remove(client);

  // remove from the screen's list
  clients.remove(client);

  // unfocus the client (calls the focus callbacks)
  client->unfocus();

#ifdef DEBUG
  printf("Unmanaged window 0x%lx\n", client->window());
#endif
  
  delete client;

  // update the root properties
  changeClientList();
}

void Screen::restack(bool raise, Client *client)
{
  const int layer = client->layer();
  std::vector<Window> wins;

  _stacking.remove(client);

  // the stacking list is from highest to lowest
  
  Client::List::iterator it = _stacking.begin(), end = _stacking.end();
  // insert the windows above this window
  for (; it != end; ++it) {
    if ((*it)->layer() < layer || (raise && (*it)->layer() == layer))
      break;
    wins.push_back((*it)->frame->window());
  }
  // insert our client
  wins.push_back(client->frame->window());
  _stacking.insert(it, client);
  // insert the remaining below this window
  for (; it != end; ++it)
    wins.push_back((*it)->frame->window());

  XRestackWindows(otk::Display::display, &wins[0], wins.size());
  changeStackingList();
}

void Screen::changeDesktop(long desktop)
{
  if (!(desktop >= 0 && desktop < _num_desktops)) return;

  printf("Moving to desktop %ld\n", desktop);
  
  long old = _desktop;
  
  _desktop = desktop;
  Openbox::instance->property()->set(_info->rootWindow(),
                                     otk::Property::net_current_desktop,
                                     otk::Property::Atom_Cardinal,
                                     _desktop);

  if (old == _desktop) return;

  Client::List::iterator it, end = clients.end();
  for (it = clients.begin(); it != end; ++it) {
    if ((*it)->desktop() == old) {
      (*it)->frame->hide();
    } else if ((*it)->desktop() == _desktop) {
      (*it)->frame->show();
    }
  }

  // force the callbacks to fire
  if (!Openbox::instance->focusedClient())
    Openbox::instance->setFocusedClient(0);
}

void Screen::changeNumDesktops(long num)
{
  assert(num > 0);
  
  if (!(num > 0)) return;

  // XXX: move windows on desktops that will no longer exist!
  
  _num_desktops = num;
  Openbox::instance->property()->set(_info->rootWindow(),
                                     otk::Property::net_number_of_desktops,
                                     otk::Property::Atom_Cardinal,
                                     _num_desktops);

  // set the viewport hint
  unsigned long *viewport = new unsigned long[_num_desktops * 2];
  memset(viewport, 0, sizeof(unsigned long) * _num_desktops * 2);
  Openbox::instance->property()->set(_info->rootWindow(),
                                     otk::Property::net_desktop_viewport,
                                     otk::Property::Atom_Cardinal,
                                     viewport, _num_desktops * 2);
  delete [] viewport;

  // update the work area hint
  changeWorkArea();
}


void Screen::updateDesktopNames()
{
  const otk::Property *property = Openbox::instance->property();

  unsigned long num = (unsigned) -1;
  
  if (!property->get(_info->rootWindow(),
                     otk::Property::net_desktop_names,
                     otk::Property::utf8, &num, &_desktop_names))
    _desktop_names.clear();
  while ((long)_desktop_names.size() < _num_desktops)
    _desktop_names.push_back("Unnamed");
}


void Screen::setDesktopName(long i, const otk::ustring &name)
{
  assert(i >= 0);

  if (i >= _num_desktops) return;

  const otk::Property *property = Openbox::instance->property();
  
  otk::Property::StringVect newnames = _desktop_names;
  newnames[i] = name;
  property->set(_info->rootWindow(), otk::Property::net_desktop_names,
                otk::Property::utf8, newnames);
}


void Screen::propertyHandler(const XPropertyEvent &e)
{
  otk::EventHandler::propertyHandler(e);

  const otk::Property *property = Openbox::instance->property();

  // compress changes to a single property into a single change
  XEvent ce;
  while (XCheckTypedEvent(otk::Display::display, e.type, &ce)) {
    // XXX: it would be nice to compress ALL changes to a property, not just
    //      changes in a row without other props between.
    if (ce.xproperty.atom != e.atom) {
      XPutBackEvent(otk::Display::display, &ce);
      break;
    }
  }

  if (e.atom == property->atom(otk::Property::net_desktop_names)) 
    updateDesktopNames();
}


void Screen::clientMessageHandler(const XClientMessageEvent &e)
{
  otk::EventHandler::clientMessageHandler(e);

  if (e.format != 32) return;

  const otk::Property *property = Openbox::instance->property();

  if (e.message_type == property->atom(otk::Property::net_current_desktop)) {
    changeDesktop(e.data.l[0]);
  } else if (e.message_type ==
             property->atom(otk::Property::net_number_of_desktops)) {
    changeNumDesktops(e.data.l[0]);
  }
  // XXX: so many client messages to handle here! ..or not.. they go to clients
}


void Screen::mapRequestHandler(const XMapRequestEvent &e)
{
  otk::EventHandler::mapRequestHandler(e);

#ifdef    DEBUG
  printf("MapRequest for 0x%lx\n", e.window);
#endif // DEBUG

  /*
    MapRequest events come here even after the window exists instead of going
    right to the client window, because of how they are sent and their struct
    layout.
  */
  Client *c = Openbox::instance->findClient(e.window);

  if (c) {
    // send a net_active_window message
    XEvent ce;
    ce.xclient.type = ClientMessage;
    ce.xclient.message_type =
      Openbox::instance->property()->atom(otk::Property::net_active_window);
    ce.xclient.display = otk::Display::display;
    ce.xclient.window = c->window();
    ce.xclient.format = 32;
    ce.xclient.data.l[0] = 0l;
    ce.xclient.data.l[1] = 0l;
    ce.xclient.data.l[2] = 0l;
    ce.xclient.data.l[3] = 0l;
    ce.xclient.data.l[4] = 0l;
    XSendEvent(otk::Display::display, _info->rootWindow(), false,
               SubstructureRedirectMask | SubstructureNotifyMask,
               &ce);
  } else
    manageWindow(e.window);
}
}
