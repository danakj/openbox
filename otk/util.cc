// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

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

namespace otk {

string expandTilde(const string& s) {
  if (s[0] != '~') return s;

  const char* const home = getenv("HOME");
  if (home == NULL) return s;
printf("tilde: getenv(DISPLAY)=%s\n", getenv("DISPLAY"));

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

string basename (const string& path) {
  string::size_type slash = path.rfind('/');
  if (slash == string::npos)
    return path;
  return path.substr(slash+1);
}

}

