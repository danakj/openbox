// -*- mode: C++; indent-tabs-mode: nil; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "../version.h"
#include "openbox.hh"
#include "otk/property.hh"
#include "otk/display.hh"

extern "C" {
#include <X11/cursorfont.h>

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


void Openbox::signalHandler(int signal)
{
  switch (signal) {
  case SIGHUP:
    // XXX: Do something with HUP? Really shouldn't, we get this when X shuts
    //      down and hangs-up on us.
    
  case SIGINT:
  case SIGTERM:
  case SIGPIPE:
    printf("Caught signal %d. Exiting.\n", signal);
    instance->shutdown();

    break;
  case SIGFPE:
  case SIGSEGV:
    printf("Caught signal %d. Aborting and dumping core.\n", signal);
    abort();
  }
}


Openbox::Openbox(int argc, char **argv)
{
  struct sigaction action;

  _state = State_Starting; // initializing everything

  Openbox::instance = this;

  _displayreq = (char*) 0;
  _argv0 = argv[0];
  _doshutdown = false;

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

  _property = new otk::OBProperty();

  // create the mouse cursors we'll use
  _cursors.session = XCreateFontCursor(otk::OBDisplay::display, XC_left_ptr);
  _cursors.move = XCreateFontCursor(otk::OBDisplay::display, XC_fleur);
  _cursors.ll_angle = XCreateFontCursor(otk::OBDisplay::display, XC_ll_angle);
  _cursors.lr_angle = XCreateFontCursor(otk::OBDisplay::display, XC_lr_angle);
  _cursors.ul_angle = XCreateFontCursor(otk::OBDisplay::display, XC_ul_angle);
  _cursors.ur_angle = XCreateFontCursor(otk::OBDisplay::display, XC_ur_angle);
  
  _state = State_Normal; // done starting
}


Openbox::~Openbox()
{
  _state = State_Exiting; // time to kill everything

  // unmanage all windows
  while (!_clients.empty())
    _xeventhandler.unmanageWindow(_clients.begin()->second);
  
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
  while (!_doshutdown) {
    if (XPending(otk::OBDisplay::display)) {
      XEvent e;
      XNextEvent(otk::OBDisplay::display, &e);
      //process_event(&e);
      _xeventhandler.handle(e);
    } else {
      _timermanager.fire();
    }
  }
}


void Openbox::addClient(Window window, OBClient *client)
{
  _clients[window] = client;
}


void Openbox::removeClient(Window window)
{
  ClientMap::iterator it = _clients.find(window);
  if (it != _clients.end())
    _clients.erase(it);
}


OBClient *Openbox::findClient(Window window)
{
  /*
    NOTE: we dont use _clients[] to find the value because that will insert
    a new null into the hash, which really sucks when we want to clean up the
    hash at shutdown!
  */
  ClientMap::iterator it = _clients.find(window);
  if (it != _clients.end())
    return it->second;
  else
    return (OBClient*) 0;
}

}

