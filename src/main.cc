// -*- mode: C++; indent-tabs-mode: nil; -*-

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

#include "gettext.h"
#define _(str) gettext(str)

#include <string>
using std::string;

#include "blackbox.hh"


static void showHelp(int exitval) {
  // print program usage and command line options
  printf(_("Openbox %s : (c) 2002 - 2002 Ben Jansens\n"),
         OPENBOX_VERSION);
  printf(_("  -display <string>  use display connection.\n\
  -rc <string>       use alternate resource file.\n\
  -menu <string>     use alternate menu file.\n\
  -version           display version and exit.\n\
  -help              display this help text and exit.\n\n"));

  // some people have requested that we print out compile options
  // as well
  printf(_("Compile time options:\n\
  Debugging:\t\t\t%s\n\
  Shape:\t\t\t%s\n\
  Xft:\t\t\t\t%s\n\
  Xinerama:\t\t\t%s\n\
  8bpp Ordered Dithering:\t%s\n\n"),
#ifdef    DEBUG
         _("yes"),
#else // !DEBUG
         _("no"),
#endif // DEBUG

#ifdef    SHAPE
         _("yes"),
#else // !SHAPE
         _("no"),
#endif // SHAPE

#ifdef    XFT
         _("yes"),
#else // !XFT
         _("no"),
#endif // XFT

#ifdef    XINERAMA
         _("yes"),
#else // !XINERAMA
         _("no"),
#endif // XINERAMA

#ifdef    ORDEREDPSEUDO
         _("yes")
#else // !ORDEREDPSEUDO
         _("no")
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
        fprintf(stderr, _("error: '-rc' requires and argument\n"));

        ::exit(1);
      }

      rc_file = argv[i];
    } else if (! strcmp(argv[i], "-menu")) {
      // look for alternative menu file to use

      if ((++i) >= argc) {
        fprintf(stderr, _("error: '-menu' requires and argument\n"));

        ::exit(1);
      }

      menu_file = argv[i];
    } else if (! strcmp(argv[i], "-display")) {
      // check for -display option... to run on a display other than the one
      // set by the environment variable DISPLAY

      if ((++i) >= argc) {
        fprintf(stderr, _("error: '-display' requires an argument\n"));

        ::exit(1);
      }

      session_display = argv[i];
      string dtmp = "DISPLAY=";
      dtmp += session_display;

      if (putenv(const_cast<char*>(dtmp.c_str()))) {
        fprintf(stderr,
                _("warning: couldn't set environment variable 'DISPLAY'\n"));
        perror("putenv()");
      }
    } else if (! strcmp(argv[i], "-version")) {
      // print current version string
      printf(_("Openbox %s : (c) 2002 - 2002 Ben Jansens\n"),
             OPENBOX_VERSION);
      printf("\n");

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
