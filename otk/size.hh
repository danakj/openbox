// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __size_hh
#define __size_hh

namespace otk {

class Size {
  unsigned int _w, _h;
public:
  Size() : _w(1), _h(1) {}
  Size(unsigned int w, unsigned int h) : _w(w), _h(h) {}
  Size(const Size &s) : _w(s._w), _h(s._h) {}

  inline unsigned int width() const { return _w; }
  inline unsigned int height() const { return _h; }

  bool operator==(const Size &o) const { return _w == o._w && _h == o._h; }
  bool operator!=(const Size &o) const { return _w != o._w || _h != o._h; }
};

}

#endif // __size_hh
