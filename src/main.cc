// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

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

#include "openbox.hh"

int main(int argc, char **argv) {
  // initialize the locale
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(PACKAGE);

  ob::Openbox openbox(argc, argv);
  openbox.eventLoop();
}
