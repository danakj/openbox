// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef _BLACKBOX_UTIL_HH
#define _BLACKBOX_UTIL_HH

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>

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
}


#include <string>
#include <vector>

namespace otk {

/* XXX: this needs autoconf help */
const unsigned int BSENTINEL = 65535;

std::string expandTilde(const std::string& s);

void bexec(const std::string& command, const std::string& displaystring);

std::string textPropertyToString(Display *display, XTextProperty& text_prop);

std::string itostring(unsigned long i);
std::string itostring(long i);
inline std::string itostring(unsigned int i)
  { return itostring((unsigned long) i); }
inline std::string itostring(int i)
  { return itostring((long) i); }

void putenv(const std::string &data);

std::string basename(const std::string& path);

}

#endif
