// -*- mode: C++; indent-tabs-mode: nil; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
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

  //ob::Openbox openbox(argc, argv);
  ob::Blackbox blackbox(argc, argv, 0);
  
  //Blackbox blackbox(argv, session_display, rc_file);
  blackbox.eventLoop();

  return(0);
}
