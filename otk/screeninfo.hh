// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __screeninfo_hh
#define   __screeninfo_hh

#include "size.hh"
#include "rect.hh"

extern "C" {
#include <X11/Xlib.h>
}

#include <string>
#include <vector>

namespace otk {

class ScreenInfo {
private:
  Visual *_visual;
  Window _root_window;
  Colormap _colormap;

  int _depth;
  unsigned int _screen;
  std::string _display_string;
  Size _size;
#ifdef XINERAMA
  std::vector<Rect> _xinerama_areas;
  bool _xinerama_active;
#endif

public:
  ScreenInfo(unsigned int num);

  inline Visual *visual() const { return _visual; }
  inline Window rootWindow() const { return _root_window; }
  inline Colormap colormap() const { return _colormap; }
  inline int depth() const { return _depth; }
  inline unsigned int screen() const { return _screen; }
  inline const Size& size() const { return _size; }
  inline const std::string& displayString() const { return _display_string; }
#ifdef XINERAMA
  inline const std::vector<Rect> &xineramaAreas() const
    { return _xinerama_areas; }
  inline bool isXineramaActive() const { return _xinerama_active; }
#endif
};

}

#endif // __screeninfo_hh
