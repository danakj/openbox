// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __display_hh
#define   __display_hh

extern "C" {
#include <X11/Xlib.h>
}

#include <vector>

namespace otk {

class ScreenInfo;
class GCCache;

//! Manages a single X11 display.
/*!
  This class is static, and cannot be instantiated.
  Use the initialize() method to open the display and ready it for use.
  Use the destroy() method to close it and clean up the class' data.
*/
class Display
{
public:
  //! The X display
  static ::Display *display;
  
  //! A List of ScreenInfo instances
  typedef std::vector<ScreenInfo> ScreenInfoList;

private:
  //! Does the display have the XKB extension?
  static bool _xkb;
  //! Base for events for the XKB extension
  static int  _xkb_event_basep;

  //! Does the display have the Shape extension?
  static bool _shape;
  //! Base for events for the Shape extension
  static int  _shape_event_basep;

  //! Does the display have the Xinerama extension?
  static bool _xinerama;
  //! Base for events for the Xinerama extension
  static int  _xinerama_event_basep;

  //! A list of all possible combinations of keyboard lock masks
  static unsigned int _mask_list[8];

  //! The value of the mask for the NumLock modifier
  static unsigned int _numLockMask;

  //! The value of the mask for the ScrollLock modifier
  static unsigned int _scrollLockMask;

  //! The number of requested grabs on the display
  static int _grab_count;

  //! A list of information for all screens on the display
  static ScreenInfoList _screenInfoList;

  //! A cache for re-using GCs, used by the drawing objects
  /*!
    @see Pen
    @see Font
    @see Image
    @see ImageControl
    @see Texture
  */
  static GCCache *_gccache;

  // Handles X errors on the display
  /*
    Displays the error if compiled for debugging.
  */
  //static int xerrorHandler(::Display *d, XErrorEvent *e);

  //! Prevents instantiation of the class
  Display();

public:
  //! Initializes the class, opens the X display
  /*!
    @see Display::display
    @param name The name of the X display to open. If it is null, the DISPLAY
                environment variable is used instead.
  */
  static void initialize(char *name);
  //! Destroys the class, closes the X display
  static void destroy();

  //! Returns the GC cache for the application
  inline static GCCache *gcCache() { return _gccache; }

  //! Gets information on a specific screen
  /*!
    Returns a ScreenInfo class, which contains information for a screen on the
    display.
    @param snum The screen number of the screen to retrieve info on
    @return Info on the requested screen, in a ScreenInfo class
  */
  static const ScreenInfo* screenInfo(int snum);

  //! Find a ScreenInfo based on a root window
  static const ScreenInfo* findScreen(Window root);

  //! Returns if the display has the xkb extension available
  inline static bool xkb() { return _xkb; }
  //! Returns the xkb extension's event base
  inline static int xkbEventBase() { return _xkb_event_basep; }

  //! Returns if the display has the shape extension available
  inline static bool shape() { return _shape; }
  //! Returns the shape extension's event base
  inline static int shapeEventBase() { return _shape_event_basep; }
  //! Returns if the display has the xinerama extension available
  inline static bool xinerama() { return _xinerama; }

  inline static unsigned int numLockMask() { return _numLockMask; }
  inline static unsigned int scrollLockMask() { return _scrollLockMask; }

  //! Grabs the display
  static void grab();

  //! Ungrabs the display
  static void ungrab();


  
  /* TEMPORARY */
  static void grabButton(unsigned int button, unsigned int modifiers,
                  Window grab_window, bool owner_events,
                  unsigned int event_mask, int pointer_mode,
                  int keyboard_mode, Window confine_to, Cursor cursor,
                  bool allow_scroll_lock);
  static void ungrabButton(unsigned int button, unsigned int modifiers,
                    Window grab_window);
  static void grabKey(unsigned int keycode, unsigned int modifiers,
                  Window grab_window, bool owner_events,
                  int pointer_mode, int keyboard_mode, bool allow_scroll_lock);
  static void ungrabKey(unsigned int keycode, unsigned int modifiers,
                        Window grab_window);
};

}

#endif // __display_hh
