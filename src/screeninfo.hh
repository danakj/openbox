// -*- mode: C++; indent-tabs-mode: nil; -*-
#ifndef   __screeninfo_hh
#define   __screeninfo_hh

#include "util.hh"

extern "C" {
#include <X11/Xlib.h>
}

#include <string>

class BaseDisplay;

class ScreenInfo {
private:
  BaseDisplay *basedisplay;
  Visual *visual;
  Window root_window;
  Colormap colormap;

  int depth;
  unsigned int screen_number;
  std::string display_string;
  Rect rect;
#ifdef XINERAMA
  RectList xinerama_areas;
  bool xinerama_active;
#endif

public:
  ScreenInfo(BaseDisplay *d, unsigned int num);

  inline BaseDisplay *getBaseDisplay(void) const { return basedisplay; }
  inline Visual *getVisual(void) const { return visual; }
  inline Window getRootWindow(void) const { return root_window; }
  inline Colormap getColormap(void) const { return colormap; }
  inline int getDepth(void) const { return depth; }
  inline unsigned int getScreenNumber(void) const
    { return screen_number; }
  inline const Rect& getRect(void) const { return rect; }
  inline unsigned int getWidth(void) const { return rect.width(); }
  inline unsigned int getHeight(void) const { return rect.height(); }
  inline const std::string& displayString(void) const
  { return display_string; }
#ifdef XINERAMA
  inline const RectList &getXineramaAreas(void) const { return xinerama_areas; }
  inline bool isXineramaActive(void) const { return xinerama_active; }
#endif
};

#endif // __screeninfo_hh
