// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

/*! @file main.cc
  @brief Main entry point for the application
*/

#include "config.h"

extern "C" {
#ifdef    HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif // HAVE_UNISTD_H

#include "gettext.h"
}

#include "openbox.hh"
#include "otk/util.hh"

#include <clocale>
#include <cstdio>

int main(int argc, char **argv) {
  // initialize the locale
  if (!setlocale(LC_ALL, ""))
    printf("Couldn't set locale from environment.\n");
  bindtextdomain(PACKAGE, LOCALEDIR);
  bind_textdomain_codeset(PACKAGE, "UTF-8");
  textdomain(PACKAGE);

  ob::Openbox *openbox = new ob::Openbox(argc, argv);
  openbox->eventLoop();

  if (openbox->doRestart()) {
    std::string prog = openbox->restartProgram();

    delete openbox; // shutdown the current one!
    
    if (!prog.empty()) {
      execl("/bin/sh", "/bin/sh", "-c", prog.c_str(), NULL); 
      perror(prog.c_str());
    }
    
    // fall back in case the above execlp doesn't work
    execvp(argv[0], argv);
    execvp(otk::basename(argv[0]).c_str(), argv);
  }

  delete openbox; // shutdown
}
