// -*- mode: C++; indent-tabs-mode: nil; -*-
// main.cc for Epistrophy - a key handler for NETWM/EWMH window managers.
// Copyright (c) 2002 - 2002 Ben Jansens <ben at orodu.net>
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
#  include "../../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#ifdef    HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif // HAVE_UNISTD_H

#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H
}

#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

#include "../../version.h"
#include "epist.hh"

static void usage();

int main(int argc, char **argv) {
  // parse the command line
  char *display_name = 0;
  char *rc_file = 0;

  for (int i = 1; i < argc; ++i) {
    const string argvi(argv[i]);
    if (argvi == "-display") {
      if (++i >= argc) {
        fprintf(stderr, "error: '-display' requires an argument\n");
        exit(1);
      }
      display_name = argv[i];
    } else if (argvi == "-rc") {
      if (++i >= argc) {
        fprintf(stderr, "error: '-rc' requires an argument\n");
        exit(1);
      }
      rc_file = argv[i];
    } else if (argvi == "-help") {
      usage();
    } else if (argvi == "-version") {
      fprintf(stderr, "epist - shipped with openbox %s\n",
              OPENBOX_VERSION);
      exit(0);
    }
    else
      usage();
  }
  
  epist ep(argv, display_name, rc_file);
  ep.eventLoop();
  return 0;
}

static void usage()
{
  cout << "usage: epist OPTIONS" << endl;
  cout << endl;
  cout << "Options:" << endl;
  cout <<
    "  -rc RCFILE         Specifies the path to an alternate rc file to load"
       << endl <<
    "  -display DISPLAY   Specifies the X display to run on" << endl <<
    "  -help              Display this help and exit" << endl <<
    "  -version           Display the version and exit" << endl <<
    endl;
  exit(0);
}
