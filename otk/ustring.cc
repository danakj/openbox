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

}
