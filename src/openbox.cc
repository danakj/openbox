// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "openbox.hh"
#include "client.hh"
#include "screen.hh"
#include "actions.hh"
#include "bindings.hh"
#include "python.hh"
#include "otk/property.hh"
#include "otk/assassin.hh"
#include "otk/util.hh"

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

#include <algorithm>

namespace ob {

Openbox *openbox = (Openbox *) 0;


void Openbox::signalHandler(int signal)
{
  switch (signal) {
  case SIGUSR1:
    printf("Caught SIGUSR1 signal. Restarting.\n");
    openbox->restart();
    break;

  case SIGHUP:
  case SIGINT:
  case SIGTERM:
  case SIGPIPE:
    printf("Caught signal %d. Exiting.\n", signal);
    openbox->shutdown();
    break;

  case SIGFPE:
  case SIGSEGV:
    printf("Caught signal %d. Aborting and dumping core.\n", signal);
    abort();
  }
}


Openbox::Openbox(int argc, char **argv)
  : otk::EventDispatcher(),
    otk::EventHandler(),
    _display()
{
  struct sigaction action;

  _state = State_Starting; // initializing everything

  openbox = this;

  _displayreq = (char*) 0;
  _argv = argv;
  _shutdown = false;
  _restart = false;
  _rcfilepath = otk::expandTilde("~/.openbox/rc3");
  _scriptfilepath = otk::expandTilde("~/.openbox/user.py");
  _focused_client = 0;
  _sync = false;

  parseCommandLine(argc, argv);

  XSynchronize(**otk::display, _sync);
  
  // set up the signal handler
  action.sa_handler = Openbox::signalHandler;
  action.sa_mask = sigset_t();
  action.sa_flags = SA_NOCLDSTOP | SA_NODEFER;
  sigaction(SIGUSR1, &action, (struct sigaction *) 0);
  sigaction(SIGPIPE, &action, (struct sigaction *) 0);
  sigaction(SIGSEGV, &action, (struct sigaction *) 0);
  sigaction(SIGFPE, &action, (struct sigaction *) 0);
  sigaction(SIGTERM, &action, (struct sigaction *) 0);
  sigaction(SIGINT, &action, (struct sigaction *) 0);
  sigaction(SIGHUP, &action, (struct sigaction *) 0);

  otk::Timer::initialize();
  _property = new otk::Property();
  _actions = new Actions();
  _bindings = new Bindings();

  setMasterHandler(_actions); // set as the master event handler

  // create the mouse cursors we'll use
  _cursors.session = XCreateFontCursor(**otk::display, XC_left_ptr);
  _cursors.move = XCreateFontCursor(**otk::display, XC_fleur);
  _cursors.ll_angle = XCreateFontCursor(**otk::display, XC_ll_angle);
  _cursors.lr_angle = XCreateFontCursor(**otk::display, XC_lr_angle);
  _cursors.ul_angle = XCreateFontCursor(**otk::display, XC_ul_angle);
  _cursors.ur_angle = XCreateFontCursor(**otk::display, XC_ur_angle);

  // initialize scripting
  python_init(argv[0]);
  
  // load config values
  python_exec(SCRIPTDIR"/config.py"); // load openbox config values
  // run all of the python scripts
  python_exec(SCRIPTDIR"/builtins.py"); // builtin callbacks
  // run the user's script or the system defaults if that fails
  if (!python_exec(_scriptfilepath.c_str()))
    python_exec(SCRIPTDIR"/defaults.py"); // system default bahaviors

  // initialize all the screens
  Screen *screen;
  int i = _single ? DefaultScreen(**otk::display) : 0;
  int max = _single ? i + 1 : ScreenCount(**otk::display);
  for (; i < max; ++i) {
    screen = new Screen(i);
    if (screen->managed())
      _screens.push_back(screen);
    else
      delete screen;
  }

  if (_screens.empty()) {
    printf(_("No screens were found without a window manager. Exiting.\n"));
    ::exit(1);
  }

  ScreenList::iterator it, end = _screens.end();
  for (it = _screens.begin(); it != end; ++it) {
    (*it)->manageExisting();
  }
 
  // grab any keys set up before the screens existed
  _bindings->grabKeys(true);

  // set up input focus
  _focused_screen = _screens[0];
  setFocusedClient(0);
  
  _state = State_Normal; // done starting
}


Openbox::~Openbox()
{
  _state = State_Exiting; // time to kill everything

  int first_screen = _screens.front()->number();
  
  std::for_each(_screens.begin(), _screens.end(), otk::PointerAssassin());

  delete _bindings;
  delete _actions;
  delete _property;

  python_destroy();

  XSetInputFocus(**otk::display, PointerRoot, RevertToNone,
                 CurrentTime);
  XSync(**otk::display, false);

  // this tends to block.. i honestly am not sure why. causing an x error in
  // the shutdown process unblocks it. blackbox simply did a ::exit(0), so
  // all im gunna do is the same.
  //otk::display->destroy();

  otk::Timer::destroy();

  if (_restart) {
    if (!_restart_prog.empty()) {
      const std::string &dstr =
        otk::display->screenInfo(first_screen)->displayString();
      otk::putenv(const_cast<char *>(dstr.c_str()));
      execlp(_restart_prog.c_str(), _restart_prog.c_str(), NULL);
      perror(_restart_prog.c_str());
    }
    
    // fall back in case the above execlp doesn't work
    execvp(_argv[0], _argv);
    execvp(otk::basename(_argv[0]).c_str(), _argv);
  }
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
    } else if (arg == "-script") {
      if (++i >= argc)
        err = true;
      else
        _scriptfilepath = argv[i];
    } else if (arg == "-sync") {
      _sync = true;
    } else if (arg == "-single") {
      _single = true;
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
  printf(_("Openbox - version %s\n"), VERSION);
  printf("    (c) 2002 - 2002 Ben Jansens\n\n");
}


void Openbox::showHelp()
{
  showVersion(); // show the version string and copyright

  // print program usage and command line options
  printf(_("Usage: %s [OPTIONS...]\n\
  Options:\n\
  -display <string>  use display connection.\n\
  -single            run on a single screen (default is to run every one).\n\
  -rc <string>       use alternate resource file.\n\
  -menu <string>     use alternate menu file.\n\
  -script <string>   use alternate startup script file.\n\
  -sync              run in synchronous mode (for debugging).\n\
  -version           display version and exit.\n\
  -help              display this help text and exit.\n\n"), _argv[0]);

  printf(_("Compile time options:\n\
  Debugging: %s\n\
  Shape:     %s\n\
  Xinerama:  %s\n\
  Xkb:       %s\n"),
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
         _("yes"),
#else // !XINERAMA
         _("no"),
#endif // XINERAMA

#ifdef    XKB
         _("yes")
#else // !XKB
         _("no")
#endif // XKB
    );
}


void Openbox::eventLoop()
{
  while (true) {
    dispatchEvents(); // from otk::EventDispatcher
    XFlush(**otk::display); // flush here before we go wait for timers
    // don't wait if we're to shutdown
    if (_shutdown) break;
    otk::Timer::dispatchTimers(!_sync); // wait if not in sync mode
  }
}


void Openbox::addClient(Window window, Client *client)
{
  _clients[window] = client;
}


void Openbox::removeClient(Window window)
{
  _clients.erase(window);
}


Client *Openbox::findClient(Window window)
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
    return (Client*) 0;
}


void Openbox::setFocusedClient(Client *c)
{
  _focused_client = c;
  if (c) {
    _focused_screen = _screens[c->screen()];
  } else {
    assert(_focused_screen);
    XSetInputFocus(**otk::display, _focused_screen->focuswindow(),
                   RevertToNone, CurrentTime);
  }
  // set the NET_ACTIVE_WINDOW hint for all screens
  ScreenList::iterator it, end = _screens.end();
  for (it = _screens.begin(); it != end; ++it) {
    int num = (*it)->number();
    Window root = otk::display->screenInfo(num)->rootWindow();
    _property->set(root, otk::Property::net_active_window,
                   otk::Property::Atom_Window,
                   (c && _focused_screen == *it) ? c->window() : None);
  }

  // call the python Focus callbacks
  EventData data(_focused_screen->number(), c, EventFocus, 0);
  _bindings->fireEvent(&data);
}

void Openbox::execute(int screen, const std::string &bin)
{
  if (screen >= ScreenCount(**otk::display))
    screen = 0;
  otk::bexec(bin, otk::display->screenInfo(screen)->displayString());
}

}

