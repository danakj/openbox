// -*- mode: C++; indent-tabs-mode: nil; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "../version.h"
#include "openbox.hh"
#include "otk/display.hh"

extern "C" {
#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#ifdef    HAVE_SIGNAL_H
#  include <signal.h>
#endif // HAVE_SIGNAL_H

#ifdef    HAVE_FCNTL_H
#  include <fcntl.h>
#endif // HAVE_FCNTL_H

#ifdef    HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif // HAVE_UNISTD_H

#ifdef    HAVE_SYS_SELECT_H
#  include <sys/select.h>
#endif // HAVE_SYS_SELECT_H

#include "gettext.h"
#define _(str) gettext(str)
}

namespace ob {


Openbox *Openbox::instance = (Openbox *) 0;


int Openbox::xerrorHandler(Display *d, XErrorEvent *e)
{
#ifdef DEBUG
  char errtxt[128];

  XGetErrorText(d, e->error_code, errtxt, 128);
  printf("X Error: %s\n", errtxt);
#else
  (void)d;
  (void)e;
#endif

  return false;
}


void Openbox::signalHandler(int signal)
{
  switch (signal) {
  case SIGHUP:
    // XXX: Do something with HUP? Really shouldn't, we get this when X shuts
    //      down and hangs-up on us.
    
  case SIGINT:
  case SIGTERM:
  case SIGPIPE:
    printf("Caught signal %d. Exiting.", signal);
    // XXX: Make Openbox exit
    break;
  case SIGFPE:
  case SIGSEGV:
    printf("Caught signal %d. Aborting and dumping core.", signal);
    abort();
  }
}


Openbox::Openbox(int argc, char **argv)
{
  struct sigaction action;

  _state = State_Starting;

  Openbox::instance = this;

  _displayreq = (char*) 0;
  _argv0 = argv[0];

  parseCommandLine(argc, argv);

  // open the X display (and gets some info about it, and its screens)
  otk::OBDisplay::initialize(_displayreq);
  assert(otk::OBDisplay::display);
    
  // set up the signal handler
  action.sa_handler = Openbox::signalHandler;
  action.sa_mask = sigset_t();
  action.sa_flags = SA_NOCLDSTOP | SA_NODEFER;
  sigaction(SIGPIPE, &action, (struct sigaction *) 0);
  sigaction(SIGSEGV, &action, (struct sigaction *) 0);
  sigaction(SIGFPE, &action, (struct sigaction *) 0);
  sigaction(SIGTERM, &action, (struct sigaction *) 0);
  sigaction(SIGINT, &action, (struct sigaction *) 0);
  sigaction(SIGHUP, &action, (struct sigaction *) 0);


  
  _state = State_Normal; // done starting
}


Openbox::~Openbox()
{
  // close the X display
  otk::OBDisplay::destroy();
}


void Openbox::parseCommandLine(int argc, char **argv)
{
  bool err = false;

  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);

    if (arg == "-display") {
      if (++i >= argc)
        err = true;
      else
        _displayreq = argv[i];
    } else if (arg == "-rc") {
      if (++i >= argc)
        err = true;
      else
        _rcfilepath = argv[i];
    } else if (arg == "-menu") {
      if (++i >= argc)
        err = true;
      else
        _menufilepath = argv[i];
    } else if (arg == "-version") {
      showVersion();
      ::exit(0);
    } else if (arg == "-help") {
      showHelp();
      ::exit(0);
    } else
      err = true;

    if (err) {
      showHelp();
      exit(1);
    }
  }
}


void Openbox::showVersion()
{
  printf(_("Openbox - version %s\n"), OPENBOX_VERSION);
  printf("    (c) 2002 - 2002 Ben Jansens\n\n");
}


void Openbox::showHelp()
{
  showVersion(); // show the version string and copyright

  // print program usage and command line options
  printf(_("Usage: %s [OPTIONS...]\n\
  Options:\n\
  -display <string>  use display connection.\n\
  -rc <string>       use alternate resource file.\n\
  -menu <string>     use alternate menu file.\n\
  -version           display version and exit.\n\
  -help              display this help text and exit.\n\n"), _argv0);

  printf(_("Compile time options:\n\
  Debugging: %s\n\
  Shape:     %s\n\
  Xinerama:  %s\n"),
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

#ifdef    XINERAMA
         _("yes")
#else // !XINERAMA
         _("no")
#endif // XINERAMA
    );
}


void Openbox::eventLoop()
{
  const int xfd = ConnectionNumber(otk::OBDisplay::display);

  while (_state == State_Normal) {
    if (XPending(otk::OBDisplay::display)) {
      XEvent e;
      XNextEvent(otk::OBDisplay::display, &e);
      process_event(&e);
    } else {
      fd_set rfds;
      timeval now, tm, *timeout = (timeval *) 0;

      FD_ZERO(&rfds);
      FD_SET(xfd, &rfds);

/*      if (! timerList.empty()) {
        const BTimer* const timer = timerList.top();

        gettimeofday(&now, 0);
        tm = timer->timeRemaining(now);

        timeout = &tm;
      }

      select(xfd + 1, &rfds, 0, 0, timeout);

      // check for timer timeout
      gettimeofday(&now, 0);

      // there is a small chance for deadlock here:
      // *IF* the timer list keeps getting refreshed *AND* the time between
      // timer->start() and timer->shouldFire() is within the timer's period
      // then the timer will keep firing.  This should be VERY near impossible.
      while (! timerList.empty()) {
        BTimer *timer = timerList.top();
        if (! timer->shouldFire(now))
          break;

        timerList.pop();

        timer->fireTimeout();
        timer->halt();
        if (timer->isRecurring())
          timer->start();
          }*/
    }
  }
}


}

