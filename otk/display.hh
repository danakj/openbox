// -*- mode: C++; indent-tabs-mode: nil; -*-
#ifndef   __display_hh
#define   __display_hh

extern "C" {
#include <X11/Xlib.h>
}

#include <vector>

namespace otk {

class ScreenInfo;
class BGCCache;

class OBDisplay
{
public:
  static Display *display;       // the X display

  typedef std::vector<ScreenInfo> ScreenInfoList;
  
private:
  static bool _shape;          // does the display have the shape extention?
  static int  _shape_event_basep;   // base for shape events

  static bool _xinerama;       // does the display have the xinerama extention?
  static int  _xinerama_event_basep;// base for xinerama events

  static unsigned int _mask_list[8];// a list of all combinations of lock masks

  static ScreenInfoList _screenInfoList; // info for all screens on the display

  static BGCCache *_gccache;

  static int xerrorHandler(Display *d, XErrorEvent *e); // handles X errors duh

  OBDisplay(); // this class cannot be instantiated

public:
  static void initialize(char *name);
  static void destroy();

  //! Returns the GC cache for the application
  inline static BGCCache *gcCache() { return _gccache; }

  /*!
    Returns a ScreenInfo class, which gives information on a screen on the
    display.
    \param snum The screen number of the screen to retrieve info on
    \return Info on the requested screen, in a ScreenInfo class
  */
  inline static const ScreenInfo* screenInfo(int snum) {
    assert(snum >= 0);
    assert(snum < static_cast<int>(_screenInfoList.size()));
    return &_screenInfoList[snum];
  }

  //! Returns if the display has the shape extention available
  inline static bool shape() { return _shape; }
  //! Returns the shape extension's event base
  inline static int shapeEventBase() { return _shape_event_basep; }
  //! Returns if the display has the xinerama extention available
  inline static bool xinerama() { return _xinerama; }
};

}

#endif // __display_hh
