// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "ustring.hh"

extern "C" {
#include <assert.h>
}

namespace otk {

ustring::ustring()
{
}

ustring::~ustring()
{
}

ustring::ustring(const ustring& other)
  : _string(other._string), _utf8(other._utf8)
{
}

ustring& ustring::operator=(const ustring& other)
{
  _string = other._string;
  _utf8 = other._utf8;
  return *this;
}

ustring::ustring(const std::string& src)
  : _string(src), _utf8(true)
{
}

ustring::ustring(const char* src)
  : _string(src), _utf8(true)
{
}

ustring& ustring::operator+=(const ustring& src)
{
  assert(_utf8 == src._utf8);
  _string += src._string;
  return *this;
}

ustring& ustring::operator+=(const char* src)
{
  _string += src;
  return *this;
}

ustring& ustring::operator+=(char c)
{
  _string += c;
  return *this;
}

static ustring::size_type find_utf8_offset(const char *str, const char *pos)
{
  ustring::size_type offset = 0;

  while (str < pos) {
    str += g_utf8_skip[*str];
    offset += g_utf8_skip[*str];
  }

  return offset;
}

ustring::size_type ustring::size() const
{
  if (_utf8) {
    const char *const pdata = _string.data();
    return find_utf8_offset(pdata, pdata + _string.size());
  } else
    return _string.size();
}

ustring::size_type ustring::bytes() const
{
  return _string.size();
}

ustring::size_type ustring::capacity() const
{
  return _string.capacity();
}

ustring::size_type ustring::max_size() const
{
  return _string.max_size();
}


const char* ustring::data() const
{
  return _string.data();
}

const char* ustring::c_str() const
{
  return _string.c_str();
}

bool ustring::utf8() const
{
  return _utf8;
}

void ustring::setUtf8(bool utf8)
{
  _utf8 = utf8;
}

}
