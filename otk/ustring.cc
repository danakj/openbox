// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "ustring.hh"

extern "C" {
#include <assert.h>
}

namespace otk {

// helper functions

// The number of bytes to skip to find the next character in the string
static const char utf8_skip[256] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
};

// takes a pointer into a utf8 string and returns a unicode character for the
// first character at the pointer
unichar utf8_get_char (const char *p)
{
  unichar result = static_cast<unsigned char>(*p);

  // if its not a 7-bit ascii character
  if((result & 0x80) != 0) {
    // len is the number of bytes this character takes up in the string
    unsigned char len = utf8_skip[result];
    result &= 0x7F >> len;

    while(--len != 0) {
      result <<= 6;
      result |= static_cast<unsigned char>(*++p) & 0x3F;
    }
  }
    
  return result;
}
  
// takes a pointer into a string and finds its offset
static ustring::size_type utf8_ptr_to_offset(const char *str, const char *pos)
{
  ustring::size_type offset = 0;

  while (str < pos) {
    str += utf8_skip[static_cast<unsigned char>(*str)];
    offset++;
  }

  return offset;
}

// takes an offset into a string and returns a pointer to it
const char *utf8_offset_to_ptr(const char *str, ustring::size_type offset)
{
  while (offset--)
    str += utf8_skip[static_cast<unsigned char>(*str)];
  return str;
}

// First overload: stop on '\0' character.
ustring::size_type utf8_byte_offset(const char* str, ustring::size_type offset)
{
  if(offset == ustring::npos)
    return ustring::npos;

  const char* p = str;

  for(; offset != 0; --offset)
  {
    if(*p == '\0')
      return ustring::npos;

    p += utf8_skip[static_cast<unsigned char>(*p)];
  }

  return (p - str);
}

// Second overload: stop when reaching maxlen.
ustring::size_type utf8_byte_offset(const char* str, ustring::size_type offset,
				    ustring::size_type maxlen)
{
  if(offset == ustring::npos)
    return ustring::npos;

  const char *const pend = str + maxlen;
  const char* p = str;

  for(; offset != 0; --offset)
  {
    if(p >= pend)
      return ustring::npos;

    p += utf8_skip[static_cast<unsigned char>(*p)];
  }

  return (p - str);
}


// ustring methods

ustring::ustring(bool utf8)
  : _utf8(utf8)
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

ustring::size_type ustring::size() const
{
  if (_utf8) {
    const char *const pdata = _string.data();
    return utf8_ptr_to_offset(pdata, pdata + _string.size());
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

bool ustring::empty() const
{
  return _string.empty();
}

void ustring::clear()
{
  _string.erase();
}

ustring& ustring::erase(ustring::size_type i, ustring::size_type n)
{
  if (_utf8) {
    // find a proper offset
    size_type utf_i = utf8_byte_offset(_string.c_str(), i);
    if (utf_i != npos) {
      // if the offset is not npos, find a proper length for 'n'
      size_type utf_n = utf8_byte_offset(_string.data() + utf_i, n,
					 _string.size() - utf_i);
      _string.erase(utf_i, utf_n);
    }
  } else
    _string.erase(i, n);

  return *this;
}

void ustring::resize(ustring::size_type n, char c)
{
  if (_utf8) {
    const size_type size_now = size();
    if(n < size_now)
      erase(n, npos);
    else if(n > size_now)
      _string.append(n - size_now, c);
  } else
    _string.resize(n, c);
}

ustring::value_type ustring::operator[](ustring::size_type i) const
{
  return utf8_get_char(utf8_offset_to_ptr(_string.data(), i));
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
