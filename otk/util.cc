// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#include <X11/Xatom.h>

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif // HAVE_UNISTD_H

#if defined(HAVE_PROCESS_H) && defined(__EMX__)
#  include <process.h>
#endif //   HAVE_PROCESS_H             __EMX__

#include "../src/gettext.h"
#define _(str) gettext(str)

#include <assert.h>
}

#include <algorithm>

#include "util.hh"

using std::string;

namespace otk {

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
    putenv(displaystring);
    int ret = execl("/bin/sh", "/bin/sh", "-c", command.c_str(), NULL);
    exit(ret);
  }
#else //   __EMX__
  spawnlp(P_NOWAIT, "cmd.exe", "cmd.exe", "/c", command.c_str(), NULL);
#endif // !__EMX__
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

void putenv(const std::string &data)
{
  char *c = new char[data.size() + 1];
  std::string::size_type i, max;
  for (i = 0, max = data.size(); i < max; ++i)
    c[i] = data[i];
  c[i] = 0;
  if (::putenv(c)) {
    printf(_("warning: couldn't set environment variable\n"));
    perror("putenv()");
  }
}

string basename (const string& path) {
  string::size_type slash = path.rfind('/');
  if (slash == string::npos)
    return path;
  return path.substr(slash+1);
}

}

