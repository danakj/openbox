// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __display_hh
#define   __display_hh

extern "C" {
#include <X11/Xlib.h>
}

namespace otk {

class RenderControl;

class Display;

//! The display instance for the library
extern Display *display;

//! Manages a single X11 display.
class Display
{
private:
  //! The X display
  ::Display *_display;
  
  //! Does the display have the XKB extension?
  bool _xkb;
  //! Base for events for the XKB extension
  int  _xkb_event_basep;

  //! Does the display have the Shape extension?
  bool _shape;
  //! Base for events for the Shape extension
  int  _shape_event_basep;

  //! Does the display have the Xinerama extension?
  bool _xinerama;
  //! Base for events for the Xinerama extension
  int  _xinerama_event_basep;

  //! A list of all possible combinations of keyboard lock masks
  unsigned int _mask_list[8];

  //! The value of the mask for the NumLock modifier
  unsigned int _num_lock_mask;

  //! The value of the mask for the ScrollLock modifier
  unsigned int _scroll_lock_mask;

  //! The key codes for the modifier keys
  XModifierKeymap *_modmap;
  
  //! The number of requested grabs on the display
  int _grab_count;

  //! When true, X errors will be ignored. Use with care.
  bool _ignore_errors;

  //! The optimal visual for the display
  Visual *_visual;

  //! Our colormap built for the optimal visual
  Colormap _colormap;

  //! The depth of our optimal visual
  int _depth;
  
public:
  //! Wraps an open Display connection
  /*!
    @param d An open Display connection.
  */
  Display(::Display *d);
  //! Destroys the class, closes the X display
  ~Display();

  //! Returns if the display has the xkb extension available
  inline bool xkb() const { return _xkb; }
  //! Returns the xkb extension's event base
  inline int xkbEventBase() const { return _xkb_event_basep; }

  //! Returns if the display has the shape extension available
  inline bool shape() const { return _shape; }
  //! Returns the shape extension's event base
  inline int shapeEventBase() const { return _shape_event_basep; }
  //! Returns if the display has the xinerama extension available
  inline bool xinerama() const { return _xinerama; }

  inline unsigned int numLockMask() const { return _num_lock_mask; }
  inline unsigned int scrollLockMask() const { return _scroll_lock_mask; }
  const XModifierKeymap *modifierMap() const { return _modmap; }

  inline ::Display* operator*() const { return _display; }

  //! When true, X errors will be ignored.
  inline bool ignoreErrors() const { return _ignore_errors; }
  //! Set whether X errors should be ignored. Use with care.
  void setIgnoreErrors(bool t);
  
  //! Grabs the display
  void grab();

  //! Ungrabs the display
  void ungrab();


  
  /* TEMPORARY */
  void grabButton(unsigned int button, unsigned int modifiers,
                  Window grab_window, bool owner_events,
                  unsigned int event_mask, int pointer_mode,
                  int keyboard_mode, Window confine_to, Cursor cursor,
                  bool allow_scroll_lock) const;
  void ungrabButton(unsigned int button, unsigned int modifiers,
                    Window grab_window) const;
  void grabKey(unsigned int keycode, unsigned int modifiers,
               Window grab_window, bool owner_events,
               int pointer_mode, int keyboard_mode,
               bool allow_scroll_lock) const;
  void ungrabKey(unsigned int keycode, unsigned int modifiers,
                 Window grab_window) const;
  void ungrabAllKeys(Window grab_window) const;
};

}

#endif // __display_hh
