// -*- mode: C++; indent-tabs-mode: nil; -*-
// main.cc for Blackbox - an X11 Window manager
// Copyright (c) 2001 - 2002 Sean 'Shaleh' Perry <shaleh@debian.org>
// Copyright (c) 1997 - 2000 Brad Hughes (bhughes@tcac.net)
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

#include "../version.h"

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#ifdef HAVE_STRING_H
#  include <string.h>
#endif // HAVE_STRING_H

#ifdef    HAVE_UNISTD_H
#include <sys/types.h>
#endif // HAVE_UNISTD_H

#ifdef    HAVE_SYS_PARAM_H
#  include <sys/param.h>
#endif // HAVE_SYS_PARAM_H
}

#include <string>
using std::string;

#include "blackbox.hh"


static void showHelp(int exitval) {
  // print program usage and command line options
  printf("Openbox %s : (c) 2002 - 2002 Ben Jansens\n"
         "  -display <string>\t\tuse display connection.\n"
         "  -rc <string>\t\t\tuse alternate resource file.\n"
         "  -menu <string>\t\tuse alternate menu file.\n"
         "  -version\t\t\tdisplay version and exit.\n"
         "  -help\t\t\t\tdisplay this help text and exit.\n\n",
         OPENBOX_VERSION);

  // some people have requested that we print out compile options
  // as well
  printf("Compile time options:\n"
         "  Debugging:\t\t\t%s\n"
         "  Shape:\t\t\t%s\n"
         "  Xft:\t\t\t\t%s\n"
         "  Xinerama:\t\t\t%s\n"
         "  8bpp Ordered Dithering:\t%s\n\n",
#ifdef    DEBUG
         "yes",
#else // !DEBUG
         "no",
#endif // DEBUG

#ifdef    SHAPE
         "yes",
#else // !SHAPE
         "no",
#endif // SHAPE

#ifdef    XFT
         "yes",
#else // !XFT
         "no",
#endif // XFT

#ifdef    XINERAMA
         "yes",
#else // !XINERAMA
         "no",
#endif // XINERAMA

#ifdef    ORDEREDPSEUDO
         "yes"
#else // !ORDEREDPSEUDO
         "no"
#endif // ORDEREDPSEUDO
          );

  ::exit(exitval);
}

int main(int argc, char **argv) {
  char *session_display = (char *) 0;
  char *rc_file = (char *) 0;
  char *menu_file = (char *) 0;

  for (int i = 1; i < argc; ++i) {
    if (! strcmp(argv[i], "-rc")) {
      // look for alternative rc file to use

      if ((++i) >= argc) {
        fprintf(stderr, "error: '-rc' requires and argument\n");

        ::exit(1);
      }

      rc_file = argv[i];
    } else if (! strcmp(argv[i], "-menu")) {
      // look for alternative menu file to use

      if ((++i) >= argc) {
        fprintf(stderr, "error: '-menu' requires and argument\n");

        ::exit(1);
      }

      menu_file = argv[i];
    } else if (! strcmp(argv[i], "-display")) {
      // check for -display option... to run on a display other than the one
      // set by the environment variable DISPLAY

      if ((++i) >= argc) {
        fprintf(stderr, "error: '-display' requires an argument\n");

        ::exit(1);
      }

      session_display = argv[i];
      string dtmp = "DISPLAY=";
      dtmp += session_display;

      if (putenv(const_cast<char*>(dtmp.c_str()))) {
        fprintf(stderr,
                "warning: couldn't set environment variable 'DISPLAY'\n");
        perror("putenv()");
      }
    } else if (! strcmp(argv[i], "-version")) {
      // print current version string
      printf("Openbox %s : (c) 2002 - 2002 Ben Jansens\n",
             OPENBOX_VERSION);

      ::exit(0);
    } else if (! strcmp(argv[i], "-help")) {
      showHelp(0);
    } else { // invalid command line option
      showHelp(-1);
    }
  }

#ifdef    __EMX__
  _chdir2(getenv("X11ROOT"));
#endif // __EMX__

  Blackbox blackbox(argv, session_display, rc_file);
  blackbox.eventLoop();

  return(0);
}
