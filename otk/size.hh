// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __size_hh
#define __size_hh

#include <cassert>

namespace otk {

class Size {
  int _w, _h;
public:
  Size() : _w(1), _h(1) {}
  Size(int w, int h) : _w(w), _h(h) { assert(_w >= 0 && _h >= 0); }
  Size(const Size &s) : _w(s._w), _h(s._h) { assert(_w >= 0 && _h >= 0); }

  inline int width() const { return _w; }
  inline int height() const { return _h; }

  bool operator==(const Size &o) const { return _w == o._w && _h == o._h; }
  bool operator!=(const Size &o) const { return _w != o._w || _h != o._h; }
};

}

#endif // __size_hh
