// XAtom.cc for Openbox
// Copyright (c) 2002 - 2002 Ben Jansens (xor at orodu.net)
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

#include "../config.h"

#include <assert.h>

#include "xatom.hh"
#include "screen.hh"
#include "util.hh"

XAtom::XAtom(Display *d) {
  _display = d;

  // make sure asserts fire if there is a problem
  memset(_atoms, 0, sizeof(_atoms));

  _atoms[cardinal] = XA_CARDINAL;
  _atoms[window] = XA_WINDOW;
  _atoms[pixmap] = XA_PIXMAP;
  _atoms[atom] = XA_ATOM;
  _atoms[string] = XA_STRING;
  _atoms[utf8_string] = create("UTF8_STRING");
  
#ifdef    HAVE_GETPID
  _atoms[blackbox_pid] = create("_BLACKBOX_PID");
#endif // HAVE_GETPID

  _atoms[wm_colormap_windows] = create("WM_COLORMAP_WINDOWS");
  _atoms[wm_protocols] = create("WM_PROTOCOLS");
  _atoms[wm_state] = create("WM_STATE");
  _atoms[wm_change_state] = create("WM_CHANGE_STATE");
  _atoms[wm_delete_window] = create("WM_DELETE_WINDOW");
  _atoms[wm_take_focus] = create("WM_TAKE_FOCUS");
  _atoms[wm_name] = create("WM_NAME");
  _atoms[wm_icon_name] = create("WM_ICON_NAME");
  _atoms[wm_class] = create("WM_CLASS");
  _atoms[motif_wm_hints] = create("_MOTIF_WM_HINTS");
  _atoms[blackbox_hints] = create("_BLACKBOX_HINTS");
  _atoms[blackbox_attributes] = create("_BLACKBOX_ATTRIBUTES");
  _atoms[blackbox_change_attributes] = create("_BLACKBOX_CHANGE_ATTRIBUTES");
  _atoms[blackbox_structure_messages] = create("_BLACKBOX_STRUCTURE_MESSAGES");
  _atoms[blackbox_notify_startup] = create("_BLACKBOX_NOTIFY_STARTUP");
  _atoms[blackbox_notify_window_add] = create("_BLACKBOX_NOTIFY_WINDOW_ADD");
  _atoms[blackbox_notify_window_del] = create("_BLACKBOX_NOTIFY_WINDOW_DEL");
  _atoms[blackbox_notify_current_workspace] = 
    create("_BLACKBOX_NOTIFY_CURRENT_WORKSPACE");
  _atoms[blackbox_notify_workspace_count] =
    create("_BLACKBOX_NOTIFY_WORKSPACE_COUNT");
  _atoms[blackbox_notify_window_focus] =
    create("_BLACKBOX_NOTIFY_WINDOW_FOCUS");
  _atoms[blackbox_notify_window_raise] =
    create("_BLACKBOX_NOTIFY_WINDOW_RAISE");
  _atoms[blackbox_notify_window_lower] =
    create("_BLACKBOX_NOTIFY_WINDOW_LOWER");
  
  _atoms[blackbox_change_workspace] = create("_BLACKBOX_CHANGE_WORKSPACE");
  _atoms[blackbox_change_window_focus] =
    create("_BLACKBOX_CHANGE_WINDOW_FOCUS");
  _atoms[blackbox_cycle_window_focus] = create("_BLACKBOX_CYCLE_WINDOW_FOCUS");

  _atoms[openbox_show_root_menu] = create("_OPENBOX_SHOW_ROOT_MENU");
  _atoms[openbox_show_workspace_menu] = create("_OPENBOX_SHOW_WORKSPACE_MENU");

  _atoms[net_supported] = create("_NET_SUPPORTED");
  _atoms[net_client_list] = create("_NET_CLIENT_LIST");
  _atoms[net_client_list_stacking] = create("_NET_CLIENT_LIST_STACKING");
  _atoms[net_number_of_desktops] = create("_NET_NUMBER_OF_DESKTOPS");
  _atoms[net_desktop_geometry] = create("_NET_DESKTOP_GEOMETRY");
  _atoms[net_desktop_viewport] = create("_NET_DESKTOP_VIEWPORT");
  _atoms[net_current_desktop] = create("_NET_CURRENT_DESKTOP");
  _atoms[net_desktop_names] = create("_NET_DESKTOP_NAMES");
  _atoms[net_active_window] = create("_NET_ACTIVE_WINDOW");
  _atoms[net_workarea] = create("_NET_WORKAREA");
  _atoms[net_supporting_wm_check] = create("_NET_SUPPORTING_WM_CHECK");
//  _atoms[net_virtual_roots] = create("_NET_VIRTUAL_ROOTS");

  _atoms[net_close_window] = create("_NET_CLOSE_WINDOW");
  _atoms[net_wm_moveresize] = create("_NET_WM_MOVERESIZE");

//  _atoms[net_properties] = create("_NET_PROPERTIES");
  _atoms[net_wm_name] = create("_NET_WM_NAME");
  _atoms[net_wm_visible_name] = create("_NET_WM_VISIBLE_NAME");
  _atoms[net_wm_icon_name] = create("_NET_WM_ICON_NAME");
  _atoms[net_wm_visible_icon_name] = create("_NET_WM_VISIBLE_ICON_NAME");
  _atoms[net_wm_desktop] = create("_NET_WM_DESKTOP");
  _atoms[net_wm_window_type] = create("_NET_WM_WINDOW_TYPE");
  _atoms[net_wm_state] = create("_NET_WM_STATE");
  _atoms[net_wm_strut] = create("_NET_WM_STRUT");
//  _atoms[net_wm_icon_geometry] = create("_NET_WM_ICON_GEOMETRY");
//  _atoms[net_wm_icon] = create("_NET_WM_ICON");
//  _atoms[net_wm_pid] = create("_NET_WM_PID");
//  _atoms[net_wm_handled_icons] = create("_NET_WM_HANDLED_ICONS");
  _atoms[net_wm_allowed_actions] = create("_NET_WM_ALLOWED_ACTIONS");

//  _atoms[net_wm_ping] = create("_NET_WM_PING");
  
  _atoms[net_wm_window_type_desktop] = create("_NET_WM_WINDOW_TYPE_DESKTOP");
  _atoms[net_wm_window_type_dock] = create("_NET_WM_WINDOW_TYPE_DOCK");
  _atoms[net_wm_window_type_toolbar] = create("_NET_WM_WINDOW_TYPE_TOOLBAR");
  _atoms[net_wm_window_type_menu] = create("_NET_WM_WINDOW_TYPE_MENU");
  _atoms[net_wm_window_type_utility] = create("_NET_WM_WINDOW_TYPE_UTILITY");
  _atoms[net_wm_window_type_splash] = create("_NET_WM_WINDOW_TYPE_SPLASH");
  _atoms[net_wm_window_type_dialog] = create("_NET_WM_WINDOW_TYPE_DIALOG");
  _atoms[net_wm_window_type_normal] = create("_NET_WM_WINDOW_TYPE_NORMAL");

  _atoms[net_wm_moveresize_size_topleft] =
    create("_NET_WM_MOVERESIZE_SIZE_TOPLEFT");
  _atoms[net_wm_moveresize_size_topright] =
    create("_NET_WM_MOVERESIZE_SIZE_TOPRIGHT");
  _atoms[net_wm_moveresize_size_bottomleft] =
    create("_NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT");
  _atoms[net_wm_moveresize_size_bottomright] =
    create("_NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT");
  _atoms[net_wm_moveresize_move] =
    create("_NET_WM_MOVERESIZE_MOVE");
 
  _atoms[net_wm_action_move] = create("_NET_WM_ACTION_MOVE");
  _atoms[net_wm_action_resize] = create("_NET_WM_ACTION_RESIZE");
  _atoms[net_wm_action_shade] = create("_NET_WM_ACTION_SHADE");
  _atoms[net_wm_action_maximize_horz] = create("_NET_WM_ACTION_MAXIMIZE_HORZ");
  _atoms[net_wm_action_maximize_vert] = create("_NET_WM_ACTION_MAXIMIZE_VERT");
  _atoms[net_wm_action_change_desktop] =
    create("_NET_WM_ACTION_CHANGE_DESKTOP");
  _atoms[net_wm_action_close] = create("_NET_WM_ACTION_CLOSE");
    
  _atoms[net_wm_state_modal] = create("_NET_WM_STATE_MODAL");
  _atoms[net_wm_state_maximized_vert] = create("_NET_WM_STATE_MAXIMIZED_VERT");
  _atoms[net_wm_state_maximized_horz] = create("_NET_WM_STATE_MAXIMIZED_HORZ");
  _atoms[net_wm_state_shaded] = create("_NET_WM_STATE_SHADED");
  _atoms[net_wm_state_skip_taskbar] = create("_NET_WM_STATE_SKIP_TASKBAR");
  _atoms[net_wm_state_skip_pager] = create("_NET_WM_STATE_SKIP_PAGER");
  _atoms[net_wm_state_hidden] = create("_NET_WM_STATE_HIDDEN");
  _atoms[net_wm_state_fullscreen] = create("_NET_WM_STATE_FULLSCREEN");
  
  _atoms[kde_net_system_tray_windows] = create("_KDE_NET_SYSTEM_TRAY_WINDOWS");
  _atoms[kde_net_wm_system_tray_window_for] =
    create("_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR");
  _atoms[kde_net_wm_window_type_override] =
    create("_KDE_NET_WM_WINDOW_TYPE_OVERRIDE");
}


/*
 * clean up the class' members
 */
XAtom::~XAtom() {
  while (!_support_windows.empty()) {
    // make sure we aren't fucking with this somewhere
    assert(_support_windows.back() != None);
    XDestroyWindow(_display, _support_windows.back());
    _support_windows.pop_back();
  }
}


/*
 * Returns an atom from the Xserver, creating it if necessary.
 */
Atom XAtom::create(const char *name) const {
  return XInternAtom(_display, name, False);
}


/*
 * Sets which atoms are supported for NETWM, by Openbox, on the root window.
 */
void XAtom::setSupported(const ScreenInfo *screen) {
  Window root = screen->getRootWindow();

  // create the netwm support window
  Window w = XCreateSimpleWindow(_display, root, 0, 0, 1, 1, 0, 0, 0);
  assert(w != None);
  _support_windows.push_back(w);
  
  // set supporting window
  setValue(root, net_supporting_wm_check, window, w);
 
  //set properties on the supporting window
  setValue(w, net_wm_name, utf8, "Openbox");
  setValue(w, net_supporting_wm_check, window, w);
  
  // we don't support any yet..
  // yes we do!

  Atom supported[] = {
    _atoms[net_current_desktop],
    _atoms[net_number_of_desktops],
    _atoms[net_desktop_geometry],
    _atoms[net_desktop_viewport],
    _atoms[net_active_window],
    _atoms[net_workarea],
    _atoms[net_client_list],
    _atoms[net_client_list_stacking],
    _atoms[net_desktop_names],
    _atoms[net_close_window],
    _atoms[net_wm_name],
    _atoms[net_wm_visible_name],
    _atoms[net_wm_icon_name],
    _atoms[net_wm_visible_icon_name],
    _atoms[net_wm_desktop],
    _atoms[net_wm_strut],
    _atoms[net_wm_window_type],
    _atoms[net_wm_window_type_desktop],
    _atoms[net_wm_window_type_dock],
    _atoms[net_wm_window_type_toolbar],
    _atoms[net_wm_window_type_menu],
    _atoms[net_wm_window_type_utility],
    _atoms[net_wm_window_type_splash],
    _atoms[net_wm_window_type_dialog],
    _atoms[net_wm_window_type_normal],
    _atoms[net_wm_moveresize],
    _atoms[net_wm_moveresize_size_topleft],
    _atoms[net_wm_moveresize_size_topright],
    _atoms[net_wm_moveresize_size_bottomleft],
    _atoms[net_wm_moveresize_size_bottomright],
    _atoms[net_wm_moveresize_move],
    _atoms[net_wm_allowed_actions],
    _atoms[net_wm_action_move],
    _atoms[net_wm_action_resize],
    _atoms[net_wm_action_shade],
    _atoms[net_wm_action_maximize_horz],
    _atoms[net_wm_action_maximize_vert],
    _atoms[net_wm_action_change_desktop],
    _atoms[net_wm_action_close],
    _atoms[net_wm_state],
    _atoms[net_wm_state_modal],
    _atoms[net_wm_state_maximized_vert],
    _atoms[net_wm_state_maximized_horz],
    _atoms[net_wm_state_shaded],
    _atoms[net_wm_state_skip_taskbar],
    _atoms[net_wm_state_skip_pager],
    _atoms[net_wm_state_hidden],
    _atoms[net_wm_state_fullscreen],
  };
  const int num_supported = sizeof(supported)/sizeof(Atom);

  setValue(root, net_supported, atom, supported, num_supported);
}
  

/*
 * Internal setValue.
 * Sets a window property on a window, optionally appending to the existing
 * value.
 */
void XAtom::setValue(Window win, Atom atom, Atom type,
                     unsigned char* data, int size, int nelements,
                     bool append) const {
  assert(win != None); assert(atom != None); assert(type != None);
  assert(nelements == 0 || (nelements > 0 && data != (unsigned char *) 0));
  assert(size == 8 || size == 16 || size == 32);
  XChangeProperty(_display, win, atom, type, size,
                  (append ? PropModeAppend : PropModeReplace),
                  data, nelements);
}


/*
 * Set a 32-bit property value on a window.
 */
void XAtom::setValue(Window win, Atoms atom, Atoms type,
                     unsigned long value) const {
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_ATOMS);
  setValue(win, _atoms[atom], _atoms[type],
           reinterpret_cast<unsigned char*>(&value), 32, 1, False);
}


/*
 * Set an array of 32-bit properties value on a window.
 */
void XAtom::setValue(Window win, Atoms atom, Atoms type,
                     unsigned long value[], int elements) const {
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_ATOMS);
  setValue(win, _atoms[atom], _atoms[type],
           reinterpret_cast<unsigned char*>(value), 32, elements, False);
}


/*
 * Set an string property value on a window.
 */
void XAtom::setValue(Window win, Atoms atom, StringType type,
                     const std::string &value) const {
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_STRING_TYPE);
  
  Atom t;
  switch (type) {
  case ansi: t = _atoms[string]; break;
  case utf8: t = _atoms[utf8_string]; break;
  default: assert(False); return; // unhandled StringType
  }
  setValue(win, _atoms[atom], t,
           reinterpret_cast<unsigned char *>(const_cast<char *>(value.c_str())),
           8, value.size() + 1, False); // add 1 to the size to include the null
}


/*
 * Set an array of string property values on a window.
 */
void XAtom::setValue(Window win, Atoms atom, StringType type,
                     const StringVect &strings) const {
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_STRING_TYPE);

  Atom t;
  switch (type) {
  case ansi: t = _atoms[string]; break;
  case utf8: t = _atoms[utf8_string]; break;
  default: assert(False); return; // unhandled StringType
  }

  std::string value;

  StringVect::const_iterator it = strings.begin();
  const StringVect::const_iterator end = strings.end();
  for (; it != end; ++it)
      value += *it + '\0';

  setValue(win, _atoms[atom], t,
           reinterpret_cast<unsigned char *>(const_cast<char *>(value.c_str())),
           8, value.size(), False);
}


/*
 * Internal getValue function used by all of the typed getValue functions.
 * Gets an property's value from a window.
 * Returns True if the property was successfully retrieved; False if the
 * property did not exist on the window, or has a different type/size format
 * than the user tried to retrieve.
 */
bool XAtom::getValue(Window win, Atom atom, Atom type,
                     unsigned long &nelements, unsigned char **value,
                     int size) const {
  assert(win != None); assert(atom != None); assert(type != None);
  assert(size == 8 || size == 16 || size == 32);
  assert(nelements > 0);
  unsigned char *c_val = 0;        // value alloc'd in Xlib, must be XFree()d
  Atom ret_type;
  int ret_size;
  unsigned long ret_bytes;
  int result;
  unsigned long maxread = nelements;
  bool ret = False;

  // try get the first element
  result = XGetWindowProperty(_display, win, atom, 0l, 1l, False,
                              AnyPropertyType, &ret_type, &ret_size,
                              &nelements, &ret_bytes, &c_val);
  ret = (result == Success && ret_type == type && ret_size == size &&
         nelements > 0);
  if (ret) {
    if (ret_bytes == 0 || maxread <= nelements) {
      // we got the whole property's value
      *value = new unsigned char[nelements * size/8 + 1];
      memcpy(*value, c_val, nelements * size/8 + 1);
    } else {
      // get the entire property since it is larger than one long
      XFree(c_val);
      // the number of longs that need to be retreived to get the property's
      // entire value. The last + 1 is the first long that we retrieved above.
      int remain = (ret_bytes - 1)/sizeof(long) + 1 + 1;
      if (remain > size/8 * (signed)maxread) // dont get more than the max
        remain = size/8 * (signed)maxread;
      result = XGetWindowProperty(_display, win, atom, 0l, remain, False, type,
                                  &ret_type, &ret_size, &nelements, &ret_bytes,
                                  &c_val);
      ret = (result == Success && ret_type == type && ret_size == size &&
             ret_bytes == 0);
      /*
        If the property has changed type/size, or has grown since our first
        read of it, then stop here and try again. If it shrank, then this will
        still work.
      */
      if (! ret)
        return getValue(win, atom, type, maxread, value, size);
  
      *value = new unsigned char[nelements * size/8 + 1];
      memcpy(*value, c_val, nelements * size/8 + 1);
    }    
  }
  if (c_val) XFree(c_val);
  return ret;
}


/*
 * Gets a 32-bit property's value from a window.
 */
bool XAtom::getValue(Window win, Atoms atom, Atoms type,
                         unsigned long &nelements,
                         unsigned long **value) const {
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_ATOMS);
  return getValue(win, _atoms[atom], _atoms[type], nelements,
                  reinterpret_cast<unsigned char **>(value), 32);
}


/*
 * Gets a single 32-bit property's value from a window.
 */
bool XAtom::getValue(Window win, Atoms atom, Atoms type,
                     unsigned long &value) const {
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_ATOMS);
  unsigned long *temp;
  unsigned long num = 1;
  if (! getValue(win, _atoms[atom], _atoms[type], num,
                 reinterpret_cast<unsigned char **>(&temp), 32))
    return False;
  value = temp[0];
  delete [] temp;
  return True;
}


/*
 * Gets an string property's value from a window.
 */
bool XAtom::getValue(Window win, Atoms atom, StringType type,
                     std::string &value) const {
  unsigned long n = 1;
  StringVect s;
  if (getValue(win, atom, type, n, s)) {
    value = s[0];
    return True;
  }
  return False;
}


bool XAtom::getValue(Window win, Atoms atom, StringType type,
                     unsigned long &nelements, StringVect &strings) const {
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_STRING_TYPE);
  assert(win != None); assert(_atoms[atom] != None);
  assert(nelements > 0);

  Atom t;
  switch (type) {
  case ansi: t = _atoms[string]; break;
  case utf8: t = _atoms[utf8_string]; break;
  default: assert(False); return False; // unhandled StringType
  }
  
  unsigned char *value;
  unsigned long elements = (unsigned) -1;
  if (!getValue(win, _atoms[atom], t, elements, &value, 8) || elements < 1)
    return False;

  std::string s(reinterpret_cast<char *>(value), elements);
  delete [] value;

  std::string::const_iterator it = s.begin(), end = s.end();
  unsigned long num = 0;
  while(num < nelements) {
    std::string::const_iterator tmp = it; // current string.begin()
    it = std::find(tmp, end, '\0');       // look for null between tmp and end
    strings.push_back(std::string(tmp, it));   // s[tmp:it)
    ++num;
    if (it == end) break;
    ++it;
    if (it == end) break;
  }

  nelements = num;

  return True;
}


/*
 * Removes a property entirely from a window.
 */
void XAtom::eraseValue(Window win, Atoms atom) const {
  assert(atom >= 0 && atom < NUM_ATOMS);
  XDeleteProperty(_display, win, _atoms[atom]);
}


void XAtom::sendClientMessage(Window target, Atoms type, Window about,
                              long data, long data1, long data2,
                              long data3, long data4) const {
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(target != None);

  XEvent e;
  e.xclient.type = ClientMessage;
  e.xclient.format = 32;
  e.xclient.message_type = _atoms[type];
  e.xclient.window = about;
  e.xclient.data.l[0] = data;
  e.xclient.data.l[1] = data1;
  e.xclient.data.l[2] = data2;
  e.xclient.data.l[3] = data3;
  e.xclient.data.l[4] = data4;

  XSendEvent(_display, target, False,
             SubstructureRedirectMask | SubstructureNotifyMask,
             &e);
}
