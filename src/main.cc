// -*- mode: C++; indent-tabs-mode: nil; -*-

/*! @file main.cc
  @brief Main entry point for the application
*/

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#ifdef    HAVE_LOCALE_H
# include <locale.h>
#endif // HAVE_LOCALE_H

#include "gettext.h"
}

#include <string>
using std::string;

#include "blackbox.hh"
#include "openbox.hh"

int main(int argc, char **argv) {
  // initialize the locale
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);

  ob::Openbox openbox(argc, argv);
  //ob::Blackbox blackbox(argc, argv, 0);

  //Blackbox blackbox(argv, session_display, rc_file);
  openbox.eventLoop();

  return(0);
}
