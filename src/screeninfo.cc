// -*- mode: C++; indent-tabs-mode: nil; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "screeninfo.hh"
#include "basedisplay.hh"

using std::string;

ScreenInfo::ScreenInfo(BaseDisplay *d, unsigned int num) {
  basedisplay = d;
  screen_number = num;

  root_window = RootWindow(basedisplay->getXDisplay(), screen_number);

  rect.setSize(WidthOfScreen(ScreenOfDisplay(basedisplay->getXDisplay(),
                                             screen_number)),
               HeightOfScreen(ScreenOfDisplay(basedisplay->getXDisplay(),
                                              screen_number)));
  /*
    If the default depth is at least 8 we will use that,
    otherwise we try to find the largest TrueColor visual.
    Preference is given to 24 bit over larger depths if 24 bit is an option.
  */

  depth = DefaultDepth(basedisplay->getXDisplay(), screen_number);
  visual = DefaultVisual(basedisplay->getXDisplay(), screen_number);
  colormap = DefaultColormap(basedisplay->getXDisplay(), screen_number);
  
  if (depth < 8) {
    // search for a TrueColor Visual... if we can't find one...
    // we will use the default visual for the screen
    XVisualInfo vinfo_template, *vinfo_return;
    int vinfo_nitems;
    int best = -1;

    vinfo_template.screen = screen_number;
    vinfo_template.c_class = TrueColor;

    vinfo_return = XGetVisualInfo(basedisplay->getXDisplay(),
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
      if (max_depth < depth) best = -1;
    }

    if (best != -1) {
      depth = vinfo_return[best].depth;
      visual = vinfo_return[best].visual;
      colormap = XCreateColormap(basedisplay->getXDisplay(), root_window,
                                 visual, AllocNone);
    }

    XFree(vinfo_return);
  }

  // get the default display string and strip the screen number
  string default_string = DisplayString(basedisplay->getXDisplay());
  const string::size_type pos = default_string.rfind(".");
  if (pos != string::npos)
    default_string.resize(pos);

  display_string = string("DISPLAY=") + default_string + '.' +
    itostring(static_cast<unsigned long>(screen_number));
  
#ifdef    XINERAMA
  xinerama_active = False;

  if (d->hasXineramaExtensions()) {
    if (d->getXineramaMajorVersion() == 1) {
      // we know the version 1(.1?) protocol

      /*
         in this version of Xinerama, we can't query on a per-screen basis, but
         in future versions we should be able, so the 'activeness' is checked
         on a pre-screen basis anyways.
      */
      if (XineramaIsActive(d->getXDisplay())) {
        /*
           If Xinerama is being used, there there is only going to be one screen
           present. We still, of course, want to use the screen class, but that
           is why no screen number is used in this function call. There should
           never be more than one screen present with Xinerama active.
        */
        int num;
        XineramaScreenInfo *info = XineramaQueryScreens(d->getXDisplay(), &num);
        if (num > 0 && info) {
          xinerama_areas.reserve(num);
          for (int i = 0; i < num; ++i) {
            xinerama_areas.push_back(Rect(info[i].x_org, info[i].y_org,
                                          info[i].width, info[i].height));
          }
          XFree(info);

          // if we can't find any xinerama regions, then we act as if it is not
          // active, even though it said it was
          xinerama_active = True;
        }
      }
    }
  }
#endif // XINERAMA
}
