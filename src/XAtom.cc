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

#include "XAtom.hh"
#include "blackbox.hh"
#include "Screen.hh"
#include "Util.hh"

XAtom::XAtom(Blackbox *bb) {
  _display = bb->getXDisplay();

  // make sure asserts fire if there is a problem
  memset(_atoms, sizeof(_atoms), 0);

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
  _atoms[net_virtual_roots] = create("_NET_VIRTUAL_ROOTS");

  _atoms[net_close_window] = create("_NET_CLOSE_WINDOW");
  _atoms[net_wm_moveresize] = create("_NET_WM_MOVERESIZE");

  _atoms[net_properties] = create("_NET_PROPERTIES");
  _atoms[net_wm_name] = create("_NET_WM_NAME");
  _atoms[net_wm_desktop] = create("_NET_WM_DESKTOP");
  _atoms[net_wm_window_type] = create("_NET_WM_WINDOW_TYPE");
  _atoms[net_wm_state] = create("_NET_WM_STATE");
  _atoms[net_wm_strut] = create("_NET_WM_STRUT");
  _atoms[net_wm_icon_geometry] = create("_NET_WM_ICON_GEOMETRY");
  _atoms[net_wm_icon] = create("_NET_WM_ICON");
  _atoms[net_wm_pid] = create("_NET_WM_PID");
  _atoms[net_wm_handled_icons] = create("_NET_WM_HANDLED_ICONS");

  _atoms[net_wm_ping] = create("_NET_WM_PING");
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
    _atoms[net_number_of_desktops]
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
  assert(data != (unsigned char *) 0);
  assert(size == 8 || size == 16 || size == 32);
  assert(nelements > 0);
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
           reinterpret_cast<unsigned char*>(&value), 32, 1, false);
}


/*
 * Set an array of 32-bit properties value on a window.
 */
void XAtom::setValue(Window win, Atoms atom, Atoms type,
                     unsigned long value[], int elements) const {
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_ATOMS);
  setValue(win, _atoms[atom], _atoms[type],
           reinterpret_cast<unsigned char*>(value), 32, elements, false);
}


/*
 * Set an string property value on a window.
 */
void XAtom::setValue(Window win, Atoms atom, StringType type,
                     const std::string &value) const {
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_STRING_TYPE);
  assert(win != None); assert(_atoms[atom] != None);

  const char *c = value.c_str();
  XTextProperty textprop;
  if (Success != XmbTextListToTextProperty(_display, const_cast<char**>(&c), 1,
                                           type == ansi ? XStringStyle :
#ifdef X_HAVE_UTF8_STRING
                                           XUTF8StringStyle,
#else
                                           XCompoundTextStyle,
#endif
                                           &textprop)) {
    return;
  }
  
  XSetTextProperty(_display, win, &textprop, _atoms[atom]);

  XFree(textprop.value);
}


/*
 * Internal getValue function used by all of the typed getValue functions.
 * Gets an property's value from a window.
 * Returns true if the property was successfully retrieved; false if the
 * property did not exist on the window, or has a different type/size format
 * than the user tried to retrieve.
 */
bool XAtom::getValue(Window win, Atom atom, Atom type,
                     unsigned long &nelements, unsigned char **value,
                     int size) const {
  assert(win != None); assert(atom != None); assert(type != None);
  assert(size == 8 || size == 16 || size == 32);
  unsigned char *c_val;        // value alloc'd with c malloc
  Atom ret_type;
  int ret_size;
  unsigned long ret_bytes;
  // try get the first element
  XGetWindowProperty(_display, win, atom, 0l, 1l, False, AnyPropertyType,
                     &ret_type, &ret_size, &nelements, &ret_bytes, &c_val);
  if (ret_type == None)
    // the property does not exist on the window
    return false;
  if (ret_type != type || ret_size != size) {
    // wrong data in property
    XFree(c_val);
    return false;
  }
  // the data is correct, now, is there more than 1 element?
  if (ret_bytes == 0) {
    // we got the whole property's value
    *value = new unsigned char[nelements * size/8 + 1];
    memcpy(*value, c_val, nelements * size/8 + 1);
    XFree(c_val);
    return true;    
  }
  // get the entire property since it is larger than one long
  free(c_val);
  // the number of longs that need to be retreived to get the property's entire
  // value. The last + 1 is the first long that we retrieved above.
  const int remain = (ret_bytes - 1)/sizeof(long) + 1 + 1;
  XGetWindowProperty(_display, win, atom, 0l, remain, False, type, &ret_type,
                     &ret_size, &nelements, &ret_bytes, &c_val);
  assert(ret_bytes == 0);
  *value = new unsigned char[nelements * size/8 + 1];
  memcpy(*value, c_val, nelements * size/8 + 1);
  XFree(c_val);
  return true;    
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
 * Gets an string property's value from a window.
 */
bool XAtom::getValue(Window win, Atoms atom, StringType type,
                     std::string &value) const {
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_STRING_TYPE);
  assert(win != None); assert(_atoms[atom] != None);

  XTextProperty textprop;
  if (0 == XGetTextProperty(_display, win, &textprop, _atoms[atom]))
    return false;

  int ret;
  int count;
  char **list;
  if (type == ansi) {
      ret = XmbTextPropertyToTextList(_display, &textprop, &list, &count);
  } else {
#ifdef X_HAVE_UTF8_STRING
    ret = Xutf8TextPropertyToTextList(_display, &textprop, &list, &count);
#else
    ret = XmbTextPropertyToTextList(_display, &textprop, &list, &count);
#endif
  }
 
  if (ret != Success || count < 1) {
    XFree(textprop.value);
    return false;
  }

  value = list[0];

  XFreeStringList(list);
  XFree(textprop.value);
  return true;
}




/*
 * Removes a property entirely from a window.
 */
void XAtom::eraseValue(Window win, Atoms atom) const {
  assert(atom >= 0 && atom < NUM_ATOMS);
  XDeleteProperty(_display, win, _atoms[atom]);
}
