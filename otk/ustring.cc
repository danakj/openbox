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
  : _string(other._string)
{
}

ustring& ustring::operator=(const ustring& other)
{
  _string = other._string;
  return *this;
}

ustring::ustring(const std::string& src)
  : _string(src)
{
}

ustring::ustring(const char* src)
  : _string(src)
{
}

static ustring::size_type find_offset(const char *str, const char *pos)
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
  const char *const pdata = _string.data();
  return find_offset(pdata, pdata + _string.size());
}

ustring::size_type ustring::length() const
{
  const char *const pdata = _string.data();
  return find_offset(pdata, pdata + _string.size());
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

}
