// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "screen.hh"
#include "client.hh"
#include "openbox.hh"
#include "frame.hh"
#include "bindings.hh"
#include "python.hh"
#include "otk/display.hh"
#include "otk/property.hh"
#include "otk/util.hh"

extern "C" {
#ifdef    HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif // HAVE_UNISTD_H

#include "gettext.h"
#define _(str) gettext(str)
}

#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstring>

static bool running;
static int anotherWMRunning(Display *display, XErrorEvent *) {
  printf(_("Another window manager already running on display %s.\n"),
         DisplayString(display));
  running = true;
  return -1;
}


namespace ob {


Screen::Screen(int screen)
  : _number(screen)
{
  assert(screen >= 0); assert(screen < ScreenCount(**otk::display));
  _info = otk::display->screenInfo(screen);

  ::running = false;
  XErrorHandler old = XSetErrorHandler(::anotherWMRunning);
  XSelectInput(**otk::display, _info->rootWindow(),
               Screen::event_mask);
  XSync(**otk::display, false);
  XSetErrorHandler(old);

  _managed = !::running;
  if (! _managed) return; // was unable to manage the screen

#ifdef DEBUG
  printf(_("Managing screen %d: visual 0x%lx, depth %d\n"),
         _number, XVisualIDFromVisual(_info->visual()), _info->depth());
#endif

  otk::Property::set(_info->rootWindow(), otk::Property::atoms.openbox_pid,
                     otk::Property::atoms.cardinal, (unsigned long) getpid());

  // set the mouse cursor for the root window (the default cursor)
  XDefineCursor(**otk::display, _info->rootWindow(),
                openbox->cursors().session);

  // set up notification of netwm support
  changeSupportedAtoms();

  // Set the netwm properties for geometry
  unsigned long geometry[] = { _info->size().width(),
                               _info->size().height() };
  otk::Property::set(_info->rootWindow(),
                     otk::Property::atoms.net_desktop_geometry,
                     otk::Property::atoms.cardinal, geometry, 2);

  _desktop = 0;

  changeNumDesktops(4); // set the hint
  changeDesktop(0); // set the hint

  // don't start in showing-desktop mode
  _showing_desktop = false;
  otk::Property::set(_info->rootWindow(),
                     otk::Property::atoms.net_showing_desktop,
                     otk::Property::atoms.cardinal, 0);

  // create the window which gets focus when no clients get it
  XSetWindowAttributes attr;
  attr.override_redirect = true;
  _focuswindow = XCreateWindow(**otk::display, _info->rootWindow(),
                               -100, -100, 1, 1, 0, 0, InputOnly,
                               _info->visual(), CWOverrideRedirect, &attr);
  XMapRaised(**otk::display, _focuswindow);
  
  // these may be further updated if any pre-existing windows are found in
  // the manageExising() function
  changeClientList();  // initialize the client lists, which will be empty

  updateDesktopLayout();

  // register this class as the event handler for the root window
  openbox->registerHandler(_info->rootWindow(), this);

  // call the python Startup callbacks
  EventData data(_number, 0, EventAction::Startup, 0);
  openbox->bindings()->fireEvent(&data);
}


Screen::~Screen()
{
  if (! _managed) return;

  XSelectInput(**otk::display, _info->rootWindow(), NoEventMask);
  
  // unmanage all windows
  while (!clients.empty())
    unmanageWindow(clients.front());

  // call the python Shutdown callbacks
  EventData data(_number, 0, EventAction::Shutdown, 0);
  openbox->bindings()->fireEvent(&data);

  XDestroyWindow(**otk::display, _focuswindow);
  XDestroyWindow(**otk::display, _supportwindow);
}


void Screen::manageExisting()
{
  unsigned int i, j, nchild;
  Window r, p, *children;
  XQueryTree(**otk::display, _info->rootWindow(), &r, &p,
             &children, &nchild);

  // preen the window list of all icon windows... for better dockapp support
  for (i = 0; i < nchild; i++) {
    if (children[i] == None) continue;

    XWMHints *wmhints = XGetWMHints(**otk::display,
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
    if (XGetWindowAttributes(**otk::display, children[i], &attrib)) {
      if (attrib.override_redirect) continue;

      if (attrib.map_state != IsUnmapped) {
        manageWindow(children[i]);
      }
    }
  }

  XFree(children);
}

void Screen::updateDesktopLayout()
{
  //const unsigned long _NET_WM_ORIENTATION_HORZ = 0;
  const unsigned long _NET_WM_ORIENTATION_VERT = 1;
  //const unsigned long _NET_WM_TOPLEFT = 0;
  const unsigned long _NET_WM_TOPRIGHT = 1;
  const unsigned long _NET_WM_BOTTOMRIGHT = 2;
  const unsigned long _NET_WM_BOTTOMLEFT = 3;
  
  // defaults
  _layout.orientation = DesktopLayout::Horizontal;
  _layout.start_corner = DesktopLayout::TopLeft;
  _layout.rows = 1;
  _layout.columns = _num_desktops;

  unsigned long *data, num = 4;
  if (otk::Property::get(_info->rootWindow(),
                         otk::Property::atoms.net_desktop_layout,
                         otk::Property::atoms.cardinal,
                         &num, &data)) {
    if (num == 4) {
      if (data[0] == _NET_WM_ORIENTATION_VERT)
        _layout.orientation = DesktopLayout::Vertical;
      if (data[3] == _NET_WM_TOPRIGHT)
        _layout.start_corner = DesktopLayout::TopRight;
      else if (data[3] == _NET_WM_BOTTOMRIGHT)
        _layout.start_corner = DesktopLayout::BottomRight;
      else if (data[3] == _NET_WM_BOTTOMLEFT)
        _layout.start_corner = DesktopLayout::BottomLeft;

      // fill in a zero rows/columns
      if (!(data[1] == 0 && data[2] == 0)) { // both 0's is bad data..
        if (data[1] == 0) {
          data[1] = (_num_desktops + _num_desktops % data[2]) / data[2];
        } else if (data[2] == 0) {
          data[2] = (_num_desktops + _num_desktops % data[1]) / data[1];
        }
        _layout.columns = data[1];
        _layout.rows = data[2];
      }

      // bounds checking
      if (_layout.orientation == DesktopLayout::Horizontal) {
        if (_layout.rows > _num_desktops) _layout.rows = _num_desktops;
        if (_layout.columns > (_num_desktops + _num_desktops % _layout.rows) /
            _layout.rows)
          _layout.columns = (_num_desktops + _num_desktops % _layout.rows) /
            _layout.rows;
      } else {
        if (_layout.columns > _num_desktops) _layout.columns = _num_desktops;
        if (_layout.rows > (_num_desktops + _num_desktops % _layout.columns) /
            _layout.columns)
          _layout.rows = (_num_desktops + _num_desktops % _layout.columns) /
            _layout.columns;
      }
    }
    delete [] data;
  }
}

void Screen::updateStruts()
{
  struct ApplyStrut {
    void operator()(otk::Strut &self, const otk::Strut &other) {
      self.left = std::max(self.left, other.left);
      self.right = std::max(self.right, other.right);
      self.top = std::max(self.top, other.top);
      self.bottom = std::max(self.bottom, other.bottom);
    }
  } apply;

  StrutList::iterator sit, send = _struts.end();
  // reset them all
  for (sit = _struts.begin(); sit != send; ++sit)
    sit->left = sit->right = sit->top = sit->bottom = 0;

  std::list<Client*>::const_iterator it, end = clients.end();
  for (it = clients.begin(); it != end; ++it) {
    if ((*it)->iconic()) continue; // these dont count in the strut
    
    unsigned int desk = (*it)->desktop();
    const otk::Strut &s = (*it)->strut();

    if (desk == 0xffffffff)
      for (unsigned int i = 0, e = _struts.size(); i < e; ++i)
        apply(_struts[i], s);
    else if (desk < _struts.size())
      apply(_struts[desk], s);
    else
      assert(false); // invalid desktop otherwise..
    // apply to the 'all desktops' strut
    apply(_struts.back(), s);
  }
  changeWorkArea();
}


void Screen::changeWorkArea()
{
  unsigned long *dims = new unsigned long[4 * _num_desktops];
  for (unsigned int i = 0; i < _num_desktops + 1; ++i) {
    otk::Rect old_area = _area[i];
/*
#ifdef    XINERAMA
  // reset to the full areas
  if (isXineramaActive())
    xineramaUsableArea = getXineramaAreas();
#endif // XINERAMA
*/
  
    _area[i] = otk::Rect(_struts[i].left, _struts[i].top,
                         _info->size().width() - (_struts[i].left +
                                                  _struts[i].right),
                         _info->size().height() - (_struts[i].top +
                                                   _struts[i].bottom));
    
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
    if (old_area != _area[i]) {
      // the area has changed, adjust all the maximized windows
      std::list<Client*>::iterator it, end = clients.end();
      for (it = clients.begin(); it != end; ++it)
        if (i < _num_desktops) {
          if ((*it)->desktop() == i)
            (*it)->remaximize();
        } else {
          // the 'all desktops' size
          if ((*it)->desktop() == 0xffffffff)
            (*it)->remaximize();
        }
    }

    // don't set these for the 'all desktops' area
    if (i < _num_desktops) {
      dims[(i * 4) + 0] = _area[i].x();
      dims[(i * 4) + 1] = _area[i].y();
      dims[(i * 4) + 2] = _area[i].width();
      dims[(i * 4) + 3] = _area[i].height();
    }
  }
  otk::Property::set(_info->rootWindow(), otk::Property::atoms.net_workarea,
                     otk::Property::atoms.cardinal, dims, 4 * _num_desktops);
  delete [] dims;
}


void Screen::changeSupportedAtoms()
{
  // create the netwm support window
  _supportwindow = XCreateSimpleWindow(**otk::display,
                                       _info->rootWindow(),
                                       0, 0, 1, 1, 0, 0, 0);

  // set supporting window
  otk::Property::set(_info->rootWindow(),
                     otk::Property::atoms.net_supporting_wm_check,
                     otk::Property::atoms.window, _supportwindow);

  //set properties on the supporting window
  otk::Property::set(_supportwindow, otk::Property::atoms.net_wm_name,
                     otk::Property::utf8, "Openbox");
  otk::Property::set(_supportwindow,
                     otk::Property::atoms.net_supporting_wm_check,
                     otk::Property::atoms.window, _supportwindow);

  
  Atom supported[] = {
    otk::Property::atoms.net_current_desktop,
    otk::Property::atoms.net_number_of_desktops,
    otk::Property::atoms.net_desktop_geometry,
    otk::Property::atoms.net_desktop_viewport,
    otk::Property::atoms.net_active_window,
    otk::Property::atoms.net_workarea,
    otk::Property::atoms.net_client_list,
    otk::Property::atoms.net_client_list_stacking,
    otk::Property::atoms.net_desktop_names,
    otk::Property::atoms.net_close_window,
    otk::Property::atoms.net_desktop_layout,
    otk::Property::atoms.net_showing_desktop,
    otk::Property::atoms.net_wm_name,
    otk::Property::atoms.net_wm_visible_name,
    otk::Property::atoms.net_wm_icon_name,
    otk::Property::atoms.net_wm_visible_icon_name,
    otk::Property::atoms.net_wm_desktop,
    otk::Property::atoms.net_wm_strut,
    otk::Property::atoms.net_wm_window_type,
    otk::Property::atoms.net_wm_window_type_desktop,
    otk::Property::atoms.net_wm_window_type_dock,
    otk::Property::atoms.net_wm_window_type_toolbar,
    otk::Property::atoms.net_wm_window_type_menu,
    otk::Property::atoms.net_wm_window_type_utility,
    otk::Property::atoms.net_wm_window_type_splash,
    otk::Property::atoms.net_wm_window_type_dialog,
    otk::Property::atoms.net_wm_window_type_normal,
/*
    otk::Property::atoms.net_wm_moveresize,
    otk::Property::atoms.net_wm_moveresize_size_topleft,
    otk::Property::atoms.net_wm_moveresize_size_topright,
    otk::Property::atoms.net_wm_moveresize_size_bottomleft,
    otk::Property::atoms.net_wm_moveresize_size_bottomright,
    otk::Property::atoms.net_wm_moveresize_move,
*/
    otk::Property::atoms.net_wm_allowed_actions,
    otk::Property::atoms.net_wm_action_move,
    otk::Property::atoms.net_wm_action_resize,
    otk::Property::atoms.net_wm_action_minimize,
    otk::Property::atoms.net_wm_action_shade,
/*    otk::Property::atoms.net_wm_action_stick,*/
    otk::Property::atoms.net_wm_action_maximize_horz,
    otk::Property::atoms.net_wm_action_maximize_vert,
    otk::Property::atoms.net_wm_action_fullscreen,
    otk::Property::atoms.net_wm_action_change_desktop,
    otk::Property::atoms.net_wm_action_close,

    otk::Property::atoms.net_wm_state,
    otk::Property::atoms.net_wm_state_modal,
    otk::Property::atoms.net_wm_state_maximized_vert,
    otk::Property::atoms.net_wm_state_maximized_horz,
    otk::Property::atoms.net_wm_state_shaded,
    otk::Property::atoms.net_wm_state_skip_taskbar,
    otk::Property::atoms.net_wm_state_skip_pager,
    otk::Property::atoms.net_wm_state_hidden,
    otk::Property::atoms.net_wm_state_fullscreen,
    otk::Property::atoms.net_wm_state_above,
    otk::Property::atoms.net_wm_state_below,
  };
  const int num_supported = sizeof(supported)/sizeof(Atom);

  otk::Property::set(_info->rootWindow(), otk::Property::atoms.net_supported,
                     otk::Property::atoms.atom, supported, num_supported);
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
    std::list<Client*>::const_iterator it = clients.begin();
    const std::list<Client*>::const_iterator end = clients.end();
    for (; it != end; ++it, ++win_it)
      *win_it = (*it)->window();
  } else
    windows = (Window*) 0;

  otk::Property::set(_info->rootWindow(), otk::Property::atoms.net_client_list,
                     otk::Property::atoms.window, windows, size);

  if (size)
    delete [] windows;

  changeStackingList();
}


void Screen::changeStackingList()
{
  Window *windows;
  unsigned int size = _stacking.size();

  assert(size == clients.size()); // just making sure.. :)

  
  // create an array of the window ids (from bottom to top, reverse order!)
  if (size > 0) {
    Window *win_it;
    
    windows = new Window[size];
    win_it = windows;
    std::list<Client*>::const_reverse_iterator it = _stacking.rbegin();
    const std::list<Client*>::const_reverse_iterator end = _stacking.rend();
    for (; it != end; ++it, ++win_it)
      *win_it = (*it)->window();
  } else
    windows = (Window*) 0;

  otk::Property::set(_info->rootWindow(),
                     otk::Property::atoms.net_client_list_stacking,
                     otk::Property::atoms.window, windows, size);

  if (size)
    delete [] windows;
}


void Screen::manageWindow(Window window)
{
  Client *client = 0;
  XWMHints *wmhint;
  XSetWindowAttributes attrib_set;
  XEvent e;
  XWindowAttributes attrib;

  otk::display->grab();

  // check if it has already been unmapped by the time we started mapping
  // the grab does a sync so we don't have to here
  if (XCheckTypedWindowEvent(**otk::display, window, DestroyNotify, &e) ||
      XCheckTypedWindowEvent(**otk::display, window, UnmapNotify, &e)) {
    XPutBackEvent(**otk::display, &e);
    
    otk::display->ungrab();
    return; // don't manage it
  }
  
  if (!XGetWindowAttributes(**otk::display, window, &attrib) ||
      attrib.override_redirect) {
    otk::display->ungrab();
    return; // don't manage it
  }
  
  // is the window a docking app
  if ((wmhint = XGetWMHints(**otk::display, window))) {
    if ((wmhint->flags & StateHint) &&
        wmhint->initial_state == WithdrawnState) {
      //slit->addClient(w); // XXX: make dock apps work!

      otk::display->ungrab();
      XFree(wmhint);
      return;
    }
    XFree(wmhint);
  }

  // choose the events we want to receive on the CLIENT window
  attrib_set.event_mask = Client::event_mask;
  attrib_set.do_not_propagate_mask = Client::no_propagate_mask;
  XChangeWindowAttributes(**otk::display, window,
                          CWEventMask|CWDontPropagate, &attrib_set);

  // create the Client class, which gets all of the hints on the window
  client = new Client(_number, window);
  // register for events
  openbox->registerHandler(window, client);
  // add to the wm's map
  openbox->addClient(window, client);

  // we dont want a border on the client
  client->toggleClientBorder(false);
  
  // specify that if we exit, the window should not be destroyed and should be
  // reparented back to root automatically
  XChangeSaveSet(**otk::display, window, SetModeInsert);

  // create the decoration frame for the client window
  client->frame = new Frame(client);
  // register the plate for events (map req's)
  // this involves removing itself from the handler list first, since it is
  // auto added to the list, being a widget. we won't get any events on the
  // plate except for events for the client (SubstructureRedirectMask)
  openbox->clearHandler(client->frame->plate());
  openbox->registerHandler(client->frame->plate(), client);

  // add to the wm's map
  Window *w = client->frame->allWindows();
  for (unsigned int i = 0; w[i]; ++i)
    openbox->addClient(w[i], client);
  delete [] w;

  // reparent the client to the frame
  client->frame->grabClient();

  if (openbox->state() != Openbox::State_Starting) {
    // position the window intelligenty .. hopefully :)
    // call the python PLACEWINDOW binding
    EventData data(_number, client, EventAction::PlaceWindow, 0);
    openbox->bindings()->fireEvent(&data);
  }

  EventData ddata(_number, client, EventAction::DisplayingWindow, 0);
  openbox->bindings()->fireEvent(&ddata);

  client->showhide();

  client->applyStartupState();

  otk::display->ungrab();

  // add to the screen's list
  clients.push_back(client);
  // once the client is in the list, update our strut to include the new
  // client's (it is good that this happens after window placement!)
  updateStruts();
  // this puts into the stacking order, then raises it
  _stacking.push_back(client);
  raiseWindow(client);
  // update the root properties
  changeClientList();

  openbox->bindings()->grabButtons(true, client);

  EventData ndata(_number, client, EventAction::NewWindow, 0);
  openbox->bindings()->fireEvent(&ndata);

#ifdef DEBUG
  printf("Managed window 0x%lx frame 0x%lx\n",
         window, client->frame->window());
#endif
}


void Screen::unmanageWindow(Client *client)
{
  Frame *frame = client->frame;

  // call the python CLOSEWINDOW binding 
  EventData data(_number, client, EventAction::CloseWindow, 0);
  openbox->bindings()->fireEvent(&data);

  openbox->bindings()->grabButtons(false, client);

  // remove from the wm's map
  openbox->removeClient(client->window());
  Window *w = frame->allWindows();
  for (unsigned int i = 0; w[i]; ++i)
    openbox->addClient(w[i], client);
  delete [] w;
  // unregister for handling events
  openbox->clearHandler(client->window());
  
  // remove the window from our save set
  XChangeSaveSet(**otk::display, client->window(), SetModeDelete);

  // we dont want events no more
  XSelectInput(**otk::display, client->window(), NoEventMask);

  frame->hide();

  // give the client its border back
  client->toggleClientBorder(true);

  // reparent the window out of the frame
  frame->releaseClient();

#ifdef DEBUG
  Window framewin = client->frame->window();
#endif
  delete client->frame;
  client->frame = 0;

  // remove from the stacking order
  _stacking.remove(client);

  // remove from the screen's list
  clients.remove(client);

  // once the client is out of the list, update our strut to remove it's
  // influence
  updateStruts();

  // unset modal before dropping our focus
  client->_modal = false;
  
  // unfocus the client (calls the focus callbacks)
  if (client->focused()) client->unfocus();

#ifdef DEBUG
  printf("Unmanaged window 0x%lx frame 0x%lx\n", client->window(), framewin);
#endif
  
  delete client;

  // update the root properties
  changeClientList();
}

void Screen::lowerWindow(Client *client)
{
  Window wins[2];  // only ever restack 2 windows.

  assert(!_stacking.empty()); // this would be bad

  std::list<Client*>::iterator it = --_stacking.end();
  const std::list<Client*>::iterator end = _stacking.begin();

  if (client->modal() && client->transientFor()) {
    // don't let a modal window lower below its transient_for
    it = std::find(_stacking.begin(), _stacking.end(), client->transientFor());
    assert(it != _stacking.end());

    wins[0] = (it == _stacking.begin() ? _focuswindow :
               ((*(--std::list<Client*>::const_iterator(it)))->
                frame->window()));
    wins[1] = client->frame->window();
    if (wins[0] == wins[1]) return; // already right above the window

    _stacking.remove(client);
    _stacking.insert(it, client);
  } else {
    for (; it != end && (*it)->layer() < client->layer(); --it);
    if (*it == client) return;          // already the bottom, return

    wins[0] = (*it)->frame->window();
    wins[1] = client->frame->window();

    _stacking.remove(client);
    _stacking.insert(++it, client);
  }

  XRestackWindows(**otk::display, wins, 2);
  changeStackingList();
}

void Screen::raiseWindow(Client *client)
{
  Window wins[2];  // only ever restack 2 windows.

  assert(!_stacking.empty()); // this would be bad

  Client *m = client->findModalChild();
  // if we have a modal child, raise it instead, we'll go along tho later
  if (m) raiseWindow(m);
  
  // remove the client before looking so we can't run into ourselves
  _stacking.remove(client);
  
  std::list<Client*>::iterator it = _stacking.begin();
  const std::list<Client*>::iterator end = _stacking.end();

  // the stacking list is from highest to lowest
  for (; it != end && ((*it)->layer() > client->layer() || m == *it); ++it);

  /*
    if our new position is the top, we want to stack under the _focuswindow
    otherwise, we want to stack under the previous window in the stack.
  */
  wins[0] = (it == _stacking.begin() ? _focuswindow :
             ((*(--std::list<Client*>::const_iterator(it)))->frame->window()));
  wins[1] = client->frame->window();

  _stacking.insert(it, client);

  XRestackWindows(**otk::display, wins, 2);

  changeStackingList(); 
}

void Screen::changeDesktop(unsigned int desktop)
{
  if (desktop >= _num_desktops) return;

  printf("Moving to desktop %u\n", desktop);
  
  unsigned int old = _desktop;
  
  _desktop = desktop;
  otk::Property::set(_info->rootWindow(),
                     otk::Property::atoms.net_current_desktop,
                     otk::Property::atoms.cardinal, _desktop);

  if (old == _desktop) return;

  std::list<Client*>::iterator it, end = clients.end();
  for (it = clients.begin(); it != end; ++it)
    (*it)->showhide();

  // force the callbacks to fire
  if (!openbox->focusedClient())
    openbox->setFocusedClient(0);
}

void Screen::changeNumDesktops(unsigned int num)
{
  assert(num > 0);
  
  if (!(num > 0)) return;

  // move windows on desktops that will no longer exist!
  std::list<Client*>::iterator it, end = clients.end();
  for (it = clients.begin(); it != end; ++it) {
    unsigned int d = (*it)->desktop();
    if (d >= num && d != 0xffffffff) {
      XEvent ce;
      ce.xclient.type = ClientMessage;
      ce.xclient.message_type = otk::Property::atoms.net_wm_desktop;
      ce.xclient.display = **otk::display;
      ce.xclient.window = (*it)->window();
      ce.xclient.format = 32;
      ce.xclient.data.l[0] = num - 1;
      XSendEvent(**otk::display, _info->rootWindow(), false,
                 SubstructureNotifyMask | SubstructureRedirectMask, &ce);
    }
  }

  _num_desktops = num;
  otk::Property::set(_info->rootWindow(),
                     otk::Property::atoms.net_number_of_desktops,
                     otk::Property::atoms.cardinal, _num_desktops);

  // set the viewport hint
  unsigned long *viewport = new unsigned long[_num_desktops * 2];
  memset(viewport, 0, sizeof(unsigned long) * _num_desktops * 2);
  otk::Property::set(_info->rootWindow(),
                     otk::Property::atoms.net_desktop_viewport,
                     otk::Property::atoms.cardinal,
                     viewport, _num_desktops * 2);
  delete [] viewport;

  // change our struts/area to match
  _area.resize(_num_desktops + 1);
  _struts.resize(_num_desktops + 1);
  updateStruts();

  // the number of rows/columns will differ
  updateDesktopLayout();

  // may be some unnamed desktops that we need to fill in with names
  updateDesktopNames();

  // change our desktop if we're on one that no longer exists!
  if (_desktop >= _num_desktops)
    changeDesktop(_num_desktops - 1);
}


void Screen::updateDesktopNames()
{
  unsigned long num;
  
  if (!otk::Property::get(_info->rootWindow(),
                          otk::Property::atoms.net_desktop_names,
                          otk::Property::utf8, &num, &_desktop_names))
    _desktop_names.clear();
  while (_desktop_names.size() < _num_desktops)
    _desktop_names.push_back("Unnamed");
}


const otk::Rect& Screen::area(unsigned int desktop) const {
  assert(desktop < _num_desktops || desktop == 0xffffffff);
  if (desktop < _num_desktops)
    return _area[desktop];
  else
    return _area[_num_desktops];
}

void Screen::installColormap(bool install) const
{
  if (install)
    XInstallColormap(**otk::display, _info->colormap());
  else
    XUninstallColormap(**otk::display, _info->colormap());
}

void Screen::showDesktop(bool show)
{
  if (show == _showing_desktop) return; // no change

  // save the window focus, and restore it when leaving the show-desktop mode
  static Window saved_focus = 0;
  if (show) {
    Client *c = openbox->focusedClient();
    if (c) saved_focus = c->window();
  }
  
  _showing_desktop = show;

  std::list<Client*>::iterator it, end = clients.end();
  for (it = clients.begin(); it != end; ++it) {
    if ((*it)->type() == Client::Type_Desktop) {
      if (show)
        (*it)->focus();
    } else
      (*it)->showhide();
  }

  if (!show) {
    Client *f = openbox->focusedClient();
    if (!f || f->type() == Client::Type_Desktop) {
      Client *c = openbox->findClient(saved_focus);
      if (c) c->focus();
    }
  }

  otk::Property::set(_info->rootWindow(),
                     otk::Property::atoms.net_showing_desktop,
                     otk::Property::atoms.cardinal,
                     show ? 1 : 0);
}

void Screen::propertyHandler(const XPropertyEvent &e)
{
  otk::EventHandler::propertyHandler(e);

  // compress changes to a single property into a single change
  XEvent ce;
  while (XCheckTypedWindowEvent(**otk::display, _info->rootWindow(),
                                e.type, &ce)) {
    // XXX: it would be nice to compress ALL changes to a property, not just
    //      changes in a row without other props between.
    if (ce.xproperty.atom != e.atom) {
      XPutBackEvent(**otk::display, &ce);
      break;
    }
  }

  if (e.atom == otk::Property::atoms.net_desktop_names)
    updateDesktopNames();
  else if (e.atom == otk::Property::atoms.net_desktop_layout)
    updateDesktopLayout();
}


void Screen::clientMessageHandler(const XClientMessageEvent &e)
{
  otk::EventHandler::clientMessageHandler(e);

  if (e.format != 32) return;

  if (e.message_type == otk::Property::atoms.net_current_desktop) {
    changeDesktop(e.data.l[0]);
  } else if (e.message_type == otk::Property::atoms.net_number_of_desktops) {
    changeNumDesktops(e.data.l[0]);
  } else if (e.message_type == otk::Property::atoms.net_showing_desktop) {
    showDesktop(e.data.l[0] != 0);
  }
}


void Screen::mapRequestHandler(const XMapRequestEvent &e)
{
  otk::EventHandler::mapRequestHandler(e);

#ifdef    DEBUG
  printf("MapRequest for 0x%lx\n", e.window);
#endif // DEBUG

  Client *c = openbox->findClient(e.window);
  if (c) {
#ifdef DEBUG
    printf("DEBUG: MAP REQUEST CAUGHT IN SCREEN. IGNORED.\n");
#endif
  } else {
    if (_showing_desktop)
      showDesktop(false); // leave showing-the-desktop mode
    manageWindow(e.window);
  }
}

}
