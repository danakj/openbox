// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

#include "screeninfo.hh"
#include "display.hh"
#include "util.hh"

using std::string;

namespace otk {

ScreenInfo::ScreenInfo(int num) {
  assert(num >= 0 && num < ScreenCount(**display));
  
  _screen = num;

  _root_window = RootWindow(**display, _screen);

  _size = Size(WidthOfScreen(ScreenOfDisplay(**display,
                                             _screen)),
               HeightOfScreen(ScreenOfDisplay(**display,
                                              _screen)));
  // get the default display string and strip the screen number
  string default_string = DisplayString(**display);
  const string::size_type pos = default_string.rfind(".");
  if (pos != string::npos)
    default_string.resize(pos);

  _display_string = string("DISPLAY=") + default_string + '.' +
    itostring(static_cast<unsigned long>(_screen));

#if 0 //def    XINERAMA
  _xinerama_active = False;

  if (d->hasXineramaExtensions()) {
    if (d->getXineramaMajorVersion() == 1) {
      // we know the version 1(.1?) protocol

      /*
        in this version of Xinerama, we can't query on a per-screen basis, but
        in future versions we should be able, so the 'activeness' is checked
        on a pre-screen basis anyways.
      */
      if (XineramaIsActive(**display)) {
        /*
          If Xinerama is being used, there there is only going to be one screen
          present. We still, of course, want to use the screen class, but that
          is why no screen number is used in this function call. There should
          never be more than one screen present with Xinerama active.
        */
        int num;
        XineramaScreenInfo *info = XineramaQueryScreens(**display, &num);
        if (num > 0 && info) {
          _xinerama_areas.reserve(num);
          for (int i = 0; i < num; ++i) {
            _xinerama_areas.push_back(Rect(info[i].x_org, info[i].y_org,
                                           info[i].width, info[i].height));
          }
          XFree(info);

          // if we can't find any xinerama regions, then we act as if it is not
          // active, even though it said it was
          _xinerama_active = true;
        }
      }
    }
  }
#else
  _xinerama_active = false;
#endif // XINERAMA
  if (!_xinerama_active)
    _xinerama_areas.push_back(Rect(Point(0, 0), _size));
}

}
