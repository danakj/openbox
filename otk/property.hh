// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __atom_hh
#define   __atom_hh

/*! @file property.hh
  @brief Provides access to window properties
*/

#include "ustring.hh"
#include "screeninfo.hh"

extern "C" {
#include <X11/Xlib.h>
}

#include <vector>
#include <cassert>

namespace otk {

//! The atoms on the X server which this class will cache
struct Atoms {
  // types
  Atom cardinal; //!< The atom which represents the Cardinal data type
  Atom window;   //!< The atom which represents window ids
  Atom pixmap;   //!< The atom which represents pixmap ids
  Atom atom;     //!< The atom which represents atom values
  Atom string;   //!< The atom which represents ascii strings
  Atom utf8;     //!< The atom which represents utf8-encoded strings

  Atom openbox_pid;

  // window hints
  Atom wm_colormap_windows;
  Atom wm_protocols;
  Atom wm_state;
  Atom wm_delete_window;
  Atom wm_take_focus;
  Atom wm_change_state;
  Atom wm_name;
  Atom wm_icon_name;
  Atom wm_class;
  Atom wm_window_role;
  Atom motif_wm_hints;

  Atom openbox_show_root_menu;
  Atom openbox_show_workspace_menu;

  // NETWM atoms
  // root window properties
  Atom net_supported;
  Atom net_client_list;
  Atom net_client_list_stacking;
  Atom net_number_of_desktops;
  Atom net_desktop_geometry;
  Atom net_desktop_viewport;
  Atom net_current_desktop;
  Atom net_desktop_names;
  Atom net_active_window;
  Atom net_workarea;
  Atom net_supporting_wm_check;
//  Atom net_virtual_roots;
  Atom net_desktop_layout;
  Atom net_showing_desktop;
  // root window messages
  Atom net_close_window;
  Atom net_wm_moveresize;
  // application window properties
//  Atom net_properties;
  Atom net_wm_name;
  Atom net_wm_visible_name;
  Atom net_wm_icon_name;
  Atom net_wm_visible_icon_name;
  Atom net_wm_desktop;
  Atom net_wm_window_type;
  Atom net_wm_state;
  Atom net_wm_strut;
//  Atom net_wm_icon_geometry;
  Atom net_wm_icon;
//  Atom net_wm_pid;
//  Atom net_wm_handled_icons;
  Atom net_wm_allowed_actions;
  // application protocols
//  Atom   Atom net_wm_ping;

  Atom net_wm_window_type_desktop;
  Atom net_wm_window_type_dock;
  Atom net_wm_window_type_toolbar;
  Atom net_wm_window_type_menu;
  Atom net_wm_window_type_utility;
  Atom net_wm_window_type_splash;
  Atom net_wm_window_type_dialog;
  Atom net_wm_window_type_normal;

  Atom net_wm_moveresize_size_topleft;
  Atom net_wm_moveresize_size_topright;
  Atom net_wm_moveresize_size_bottomleft;
  Atom net_wm_moveresize_size_bottomright;
  Atom net_wm_moveresize_move;

  Atom net_wm_action_move;
  Atom net_wm_action_resize;
  Atom net_wm_action_minimize;
  Atom net_wm_action_shade;
  Atom net_wm_action_stick;
  Atom net_wm_action_maximize_horz;
  Atom net_wm_action_maximize_vert;
  Atom net_wm_action_fullscreen;
  Atom net_wm_action_change_desktop;
  Atom net_wm_action_close;

  Atom net_wm_state_modal;
  Atom net_wm_state_sticky;
  Atom net_wm_state_maximized_vert;
  Atom net_wm_state_maximized_horz;
  Atom net_wm_state_shaded;
  Atom net_wm_state_skip_taskbar;
  Atom net_wm_state_skip_pager;
  Atom net_wm_state_hidden;
  Atom net_wm_state_fullscreen;
  Atom net_wm_state_above;
  Atom net_wm_state_below;

  Atom net_wm_state_add;
  Atom net_wm_state_remove;
  Atom net_wm_state_toggle;

  Atom kde_net_system_tray_windows;
  Atom kde_net_wm_system_tray_window_for;
  Atom kde_net_wm_window_type_override;

  Atom kwm_win_icon;

  Atom rootpmapid;
  Atom esetrootid;

  Atom openbox_premax;
  Atom openbox_active_window;
  Atom openbox_restack_window;
};


//! Provides easy access to window properties.
class Property {
public:
  
  //! The possible types/encodings of strings
  enum StringType {
    ascii, //!< Standard 8-bit ascii string
    utf8,  //!< Utf8-encoded string
#ifndef DOXYGEN_IGNORE
    NUM_STRING_TYPE
#endif
  };

  //! A list of ustrings
  typedef std::vector<ustring> StringVect;

  //! The value of all atoms on the X server that exist in the
  //! Atoms struct
  static Atoms atoms;

private:
  //! Sets a property on a window
  static void set(Window win, Atom atom, Atom type, unsigned char *data,
                  int size, int nelements, bool append);
  //! Gets a property's value from a window
  static bool get(Window win, Atom atom, Atom type,
                  unsigned long *nelements, unsigned char **value,
                  int size);

public:
  //! Initializes the Property class.
  /*!
    CAUTION: This function uses otk::Display, so ensure that
    otk::Display::initialize has been called before initializing this class!
  */
  static void initialize();

  //! Sets a single-value property on a window to a new value
  /*!
    @param win The window id of the window on which to set the property's value
    @param atom The Atom value of the property to set. This can be found in the
                struct returned by Property::atoms.
    @param type The Atom value of the property type. This can be found in the
                struct returned by Property::atoms.
    @param value The value to set the property to
  */
  static void set(Window win, Atom atom, Atom type, unsigned long value);
  //! Sets an multiple-value property on a window to a new value
  /*!
    @param win The window id of the window on which to set the property's value
    @param atom The Atom value of the property to set. This can be found in the
                struct returned by Property::atoms.
    @param type The Atom value of the property type. This can be found in the
                struct returned by Property::atoms.
    @param value Any array of values to set the property to. The array must
                 contain <i>elements</i> number of elements
    @param elements The number of elements in the <i>value</i> array
  */
  static void set(Window win, Atom atom, Atom type,
                  unsigned long value[], int elements);
  //! Sets a string property on a window to a new value
  /*!
    @param win The window id of the window on which to set the property's value
    @param atom The Atom value of the property to set. This can be found in the
                struct returned by Property::atoms.
    @param type A member of the Property::StringType enum that specifies the
                type of the string the property is being set to
    @param value The string to set the property to
  */
  static void set(Window win, Atom atom, StringType type,
                  const ustring &value);
  //! Sets a string-array property on a window to a new value
  /*!
    @param win The window id of the window on which to set the property's value
    @param atom The Atom value of the property to set. This can be found in the
                struct returned by Property::atoms.
    @param type A member of the Property::StringType enum that specifies the
                type of the string the property is being set to
    @param strings A list of strings to set the property to
  */
  static void set(Window win, Atom atom, StringType type,
                  const StringVect &strings);

  //! Gets the value of a property on a window
  /*!
    @param win The window id of the window to get the property value from
    @param atom The Atom value of the property to set. This can be found in the
                struct returned by Property::atoms.
    @param type The Atom value of the property type. This can be found in the
                struct returned by Property::atoms.
    @param nelements When the function returns, if it returns true, this will
                     contain the actual number of elements retrieved.<br>
    @param value If the function returns true, then this contains an array of
                 retrieved values for the property.<br>
                 The <i>value</i> is allocated inside the function and
                 <b>delete[]</b> value needs to be called when you are done
                 with it.<br>
                 The <i>value</i> array returned is null terminated, and has
                 <i>nelements</i> elements in it plus the terminating null.
    @return true if retrieval of the specified property with the specified
            type was successful; otherwise, false
  */
  static bool get(Window win, Atom atom, Atom type,
                  unsigned long *nelements, unsigned long **value);
  //! Gets a single element from the value of a property on a window
  /*!
    @param win The window id of the window to get the property value from
    @param atom The Atom value of the property to set. This can be found in the
                struct returned by Property::atoms.
    @param type The Atom value of the property type. This can be found in the
                struct returned by Property::atoms.
    @param value If the function returns true, then this contains the first
                 (and possibly only) element in the value of the specified
                 property.
    @return true if retrieval of the specified property with the specified
            type was successful; otherwise, false
  */
  static bool get(Window win, Atom atom, Atom type, unsigned long *value);
  //! Gets a single string from the value of a property on a window
  /*!
    @param win The window id of the window to get the property value from
    @param atom The Atom value of the property to set. This can be found in the
                struct returned by Property::atoms.
    @param type A member of the Property::StringType enum that specifies the
                type of the string property to retrieve
    @param value If the function returns true, then this contains the first
                 (and possibly only) string in the value of the specified
                 property.
    @return true if retrieval of the specified property with the specified
            type was successful; otherwise, false
  */
  static bool get(Window win, Atom atom, StringType type, ustring *value);
  //! Gets strings from the value of a property on a window
  /*!
    @param win The window id of the window to get the property value from
    @param atom The Atom value of the property to set. This can be found in the
                struct returned by Property::atoms.
    @param type A member of the Property::StringType enum that specifies the
                type of the string property to retrieve
    @param nelements When the function returns, if it returns true, this will
                     contain the actual number of strings retrieved.<br>
    @param strings If the function returns true, then this contains all of the
                   strings retrieved from the property's value.
    @return true if retrieval of the specified property with the specified
            type was successful; otherwise, false
  */
  static bool get(Window win, Atom atom, StringType type,
                  unsigned long *nelements, StringVect *strings);

  //! Removes a property from a window
  /*!
    @param win The window id of the window to remove the property from
    @param atom The Atom value of the property to set. This can be found in the
                struct returned by Property::atoms.
  */
  static void erase(Window win, Atom atom);
};

}

#endif // __atom_hh
