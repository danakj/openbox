// XAtom.h for Openbox
// Copyright (c) 2002 - 2002 Ben Janens (ben at orodu.net)
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

#ifndef   __XAtom_h
#define   __XAtom_h

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <assert.h>

#include <vector>
#include <string>

class Blackbox;
class ScreenInfo;

class XAtom {
public:
  enum Atoms {
    // types
    cardinal,
    window,
    pixmap,
    atom,
    string,
    utf8_string,
    
#ifdef    HAVE_GETPID
    blackbox_pid,
#endif // HAVE_GETPID

    // window hints
    wm_colormap_windows,
    wm_protocols,
    wm_state,
    wm_delete_window,
    wm_take_focus,
    wm_change_state,
    wm_name,
    wm_icon_name,
    wm_class,
    motif_wm_hints,
    blackbox_attributes,
    blackbox_change_attributes,
    blackbox_hints,

    // blackbox-protocol atoms (wm -> client)
    blackbox_structure_messages,
    blackbox_notify_startup,
    blackbox_notify_window_add,
    blackbox_notify_window_del,
    blackbox_notify_window_focus,
    blackbox_notify_current_workspace,
    blackbox_notify_workspace_count,
    blackbox_notify_window_raise,
    blackbox_notify_window_lower,
    // blackbox-protocol atoms (client -> wm)
    blackbox_change_workspace,
    blackbox_change_window_focus,
    blackbox_cycle_window_focus,

    // NETWM atoms
    // root window properties
    net_supported,
    net_client_list,
    net_client_list_stacking,
    net_number_of_desktops,
    net_desktop_geometry,
    net_desktop_viewport,
    net_current_desktop,
    net_desktop_names,
    net_active_window,
    net_workarea,
    net_supporting_wm_check,
//    net_virtual_roots,
    // root window messages
    net_close_window,
    net_wm_moveresize,
    // application window properties
//    net_properties,
    net_wm_name,
    net_wm_visible_name,
    net_wm_icon_name,
    net_wm_visible_icon_name,
    net_wm_desktop,
    net_wm_window_type,
    net_wm_state,
    net_wm_strut,
//  net_wm_icon_geometry,
//  net_wm_icon,
//  net_wm_pid,
//  net_wm_handled_icons,
    net_wm_allowed_actions,
    // application protocols
//    net_wm_ping,

    net_wm_window_type_desktop,
    net_wm_window_type_dock,
    net_wm_window_type_toolbar,
    net_wm_window_type_menu,
    net_wm_window_type_utility,
    net_wm_window_type_splash,
    net_wm_window_type_dialog,
    net_wm_window_type_normal,

    net_wm_moveresize_size_topleft,
    net_wm_moveresize_size_topright,
    net_wm_moveresize_size_bottomleft,
    net_wm_moveresize_size_bottomright,
    net_wm_moveresize_move,

    net_wm_action_move,
    net_wm_action_resize,
    net_wm_action_shade,
    net_wm_action_maximize_horz,
    net_wm_action_maximize_vert,
    net_wm_action_change_desktop,
    net_wm_action_close,

    net_wm_state_modal,
    net_wm_state_maximized_vert,
    net_wm_state_maximized_horz,
    net_wm_state_shaded,
    net_wm_state_skip_taskbar,
    net_wm_state_skip_pager,
    net_wm_state_hidden,
    net_wm_state_fullscreen,

    kde_net_system_tray_windows,
    kde_net_wm_system_tray_window_for,
    kde_net_wm_window_type_override,
 
    // constant for how many atoms exist in the enumerator
    NUM_ATOMS
  };

  enum StringType {
    ansi,
    utf8,
    NUM_STRING_TYPE
  };

private:
  typedef std::vector<Window> SupportWindows;
  
  Display              *_display;
  // windows used to specify support for NETWM
  SupportWindows        _support_windows;
  Atom                  _atoms[NUM_ATOMS];

  Atom create(const char *name) const;

  void setValue(Window win, Atom atom, Atom type, unsigned char *data,
                int size, int nelements, bool append) const;
  bool getValue(Window win, Atom atom, Atom type,
                unsigned long &nelements, unsigned char **value,
                int size) const;

  // no copying!!
  XAtom(const XAtom &);
  XAtom& operator=(const XAtom&);

public:
  typedef std::vector<std::string> StringVect;
  
  XAtom(Display *d);
  virtual ~XAtom();

  // setup support on a screen, each screen should call this once in its
  // constructor.
  void setSupported(const ScreenInfo *screen);
  
  void setValue(Window win, Atoms atom, Atoms type, unsigned long value) const;
  void setValue(Window win, Atoms atom, Atoms type,
                unsigned long value[], int elements) const;
  void setValue(Window win, Atoms atom, StringType type,
                const std::string &value) const;
  void setValue(Window win, Atoms atom, StringType type,
                const StringVect &strings) const;

  // the 'value' is allocated inside the function and
  // delete [] value needs to be called when you are done with it.
  // the 'value' array returned is null terminated, and has 'nelements'
  // elements in it plus the null.
  // nelements must be set to the maximum number of elements to read from
  // the property.
  bool getValue(Window win, Atoms atom, Atoms type,
                unsigned long &nelements, unsigned long **value) const;
  bool getValue(Window win, Atoms atom, Atoms type, unsigned long &value) const;
  bool getValue(Window win, Atoms atom, StringType type,
                std::string &value) const;
  bool getValue(Window win, Atoms atom, StringType type,
                unsigned long &nelements, StringVect &strings) const;
  
  void eraseValue(Window win, Atoms atom) const;

  // sends a client message a window
  void sendClientMessage(Window target, Atoms type, Window about,
                         long data = 0, long data1 = 0, long data2 = 0,
                         long data3 = 0) const;

  // temporary function!! remove when not used in blackbox.hh anymore!!
  inline Atom getAtom(Atoms a)
  { assert(a >= 0 && a < NUM_ATOMS); Atom ret = _atoms[a];
    assert(ret != 0); return ret; }
};

#endif // __XAtom_h
