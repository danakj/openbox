// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __util_hh
#define __util_hh

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

#include <string>
#include <vector>

namespace otk {

std::string expandTilde(const std::string& s);

void bexec(const std::string& command, const std::string& displaystring);

std::string itostring(unsigned long i);
std::string itostring(long i);
inline std::string itostring(unsigned int i)
  { return itostring((unsigned long) i); }
inline std::string itostring(int i)
  { return itostring((long) i); }

void putenv(const std::string &data);

std::string basename(const std::string& path);

}

#endif // __util_hh
