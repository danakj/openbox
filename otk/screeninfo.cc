// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

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
  /*
    If the default depth is at least 8 we will use that,
    otherwise we try to find the largest TrueColor visual.
    Preference is given to 24 bit over larger depths if 24 bit is an option.
  */

  _depth = DefaultDepth(**display, _screen);
  _visual = DefaultVisual(**display, _screen);
  _colormap = DefaultColormap(**display, _screen);
  
  if (_depth < 8) {
    // search for a TrueColor Visual... if we can't find one...
    // we will use the default visual for the screen
    XVisualInfo vinfo_template, *vinfo_return;
    int vinfo_nitems;
    int best = -1;

    vinfo_template.screen = _screen;
    vinfo_template.c_class = TrueColor;

    vinfo_return = XGetVisualInfo(**display,
                                  VisualScreenMask | VisualClassMask,
                                  &vinfo_template, &vinfo_nitems);
    if (vinfo_return) {
      int max_depth = 1;
      for (int i = 0; i < vinfo_nitems; ++i) {
        if (vinfo_return[i].depth > max_depth) {
          if (max_depth == 24 && vinfo_return[i].depth > 24)
            break;          // prefer 24 bit over 32
          max_depth = vinfo_return[i].depth;
          best = i;
        }
      }
      if (max_depth < _depth) best = -1;
    }

    if (best != -1) {
      _depth = vinfo_return[best].depth;
      _visual = vinfo_return[best].visual;
      _colormap = XCreateColormap(**display, _root_window, _visual,
                                  AllocNone);
    }

    XFree(vinfo_return);
  }

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
          _xinerama_active = True;
        }
      }
    }
  }
#endif // XINERAMA
}

}
