// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// Util.cc for Blackbox - an X11 Window manager
// Copyright (c) 2002 Sean 'Shaleh' Perry <shaleh@debian.org>
// Copyright (c) 1997 - 2000, 2002 Brad Hughes (bhughes@tcac.net)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#include <X11/Xatom.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef    TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
#else // !TIME_WITH_SYS_TIME
#  ifdef    HAVE_SYS_TIME_H
#    include <sys/time.h>
#  else // !HAVE_SYS_TIME_H
#    include <time.h>
#  endif // HAVE_SYS_TIME_H
#endif // TIME_WITH_SYS_TIME
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif // HAVE_UNISTD_H
#if defined(HAVE_PROCESS_H) && defined(__EMX__)
#  include <process.h>
#endif //   HAVE_PROCESS_H             __EMX__

#include <assert.h>
}

#include <algorithm>

#include "util.hh"

using std::string;


void Rect::setX(int x) {
  _x2 += x - _x1;
  _x1 = x;
}


void Rect::setY(int y)
{
  _y2 += y - _y1;
  _y1 = y;
}


void Rect::setPos(int x, int y) {
  _x2 += x - _x1;
  _x1 = x;
  _y2 += y - _y1;
  _y1 = y;
}


void Rect::setWidth(unsigned int w) {
  _x2 = w + _x1 - 1;
}


void Rect::setHeight(unsigned int h) {
  _y2 = h + _y1 - 1;
}


void Rect::setSize(unsigned int w, unsigned int h) {
  _x2 = w + _x1 - 1;
  _y2 = h + _y1 - 1;
}


void Rect::setRect(int x, int y, unsigned int w, unsigned int h) {
  *this = Rect(x, y, w, h);
}


void Rect::setCoords(int l, int t, int r, int b) {
  _x1 = l;
  _y1 = t;
  _x2 = r;
  _y2 = b;
}


Rect Rect::operator|(const Rect &a) const {
  Rect b;

  b._x1 = std::min(_x1, a._x1);
  b._y1 = std::min(_y1, a._y1);
  b._x2 = std::max(_x2, a._x2);
  b._y2 = std::max(_y2, a._y2);

  return b;
}


Rect Rect::operator&(const Rect &a) const {
  Rect b;

  b._x1 = std::max(_x1, a._x1);
  b._y1 = std::max(_y1, a._y1);
  b._x2 = std::min(_x2, a._x2);
  b._y2 = std::min(_y2, a._y2);

  return b;
}


bool Rect::intersects(const Rect &a) const {
  return std::max(_x1, a._x1) <= std::min(_x2, a._x2) &&
         std::max(_y1, a._y1) <= std::min(_y2, a._y2);
}


bool Rect::contains(int x, int y) const {
  return x >= _x1 && x <= _x2 &&
         y >= _y1 && y <= _y2;
}


bool Rect::contains(const Rect& a) const {
  return a._x1 >= _x1 && a._x2 <= _x2 &&
         a._y1 >= _y1 && a._y2 <= _y2;
}


string expandTilde(const string& s) {
  if (s[0] != '~') return s;

  const char* const home = getenv("HOME");
  if (home == NULL) return s;

  return string(home + s.substr(s.find('/')));
}


void bexec(const string& command, const string& displaystring) {
#ifndef    __EMX__
  if (! fork()) {
    setsid();
    int ret = putenv(const_cast<char *>(displaystring.c_str()));
    assert(ret != -1);
    ret = execl("/bin/sh", "/bin/sh", "-c", command.c_str(), NULL);
    exit(ret);
  }
#else //   __EMX__
  spawnlp(P_NOWAIT, "cmd.exe", "cmd.exe", "/c", command.c_str(), NULL);
#endif // !__EMX__
}


#ifndef   HAVE_BASENAME
string basename (const string& path) {
  string::size_type slash = path.rfind('/');
  if (slash == string::npos)
    return path;
  return path.substr(slash+1);
}
#endif // HAVE_BASENAME


string textPropertyToString(Display *display, XTextProperty& text_prop) {
  string ret;

  if (text_prop.value && text_prop.nitems > 0) {
    if (text_prop.encoding == XA_STRING) {
      ret = (char *) text_prop.value;
    } else {
      text_prop.nitems = strlen((char *) text_prop.value);

      char **list;
      int num;
      if (XmbTextPropertyToTextList(display, &text_prop,
                                    &list, &num) == Success &&
          num > 0 && *list) {
        ret = *list;
        XFreeStringList(list);
      }
    }
  }

  return ret;
}


timeval normalizeTimeval(const timeval &tm) {
  timeval ret = tm;

  while (ret.tv_usec < 0) {
    if (ret.tv_sec > 0) {
      --ret.tv_sec;
      ret.tv_usec += 1000000;
    } else {
      ret.tv_usec = 0;
    }
  }

  if (ret.tv_usec >= 1000000) {
    ret.tv_sec += ret.tv_usec / 1000000;
    ret.tv_usec %= 1000000;
  }

  if (ret.tv_sec < 0) ret.tv_sec = 0;

  return ret;
}


string itostring(unsigned long i) {
  if (i == 0)
    return string("0");
  
  string tmp;
  for (; i > 0; i /= 10)
    tmp.insert(tmp.begin(), "0123456789"[i%10]);
  return tmp;
}


string itostring(long i) {
  std::string tmp = itostring( (unsigned long) std::abs(i));
  if (i < 0)
    tmp.insert(tmp.begin(), '-');
  return tmp;
}
