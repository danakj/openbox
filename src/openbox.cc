// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "openbox.hh"
#include "client.hh"
#include "screen.hh"
#include "actions.hh"
#include "bindings.hh"
#include "python.hh"
#include "otk/property.hh"
#include "otk/assassin.hh"
#include "otk/property.hh"
#include "otk/util.hh"
#include "otk/rendercolor.hh"
#include "otk/renderstyle.hh"
#include "otk/messagedialog.hh"

extern "C" {
#include <X11/cursorfont.h>

#ifdef    HAVE_SIGNAL_H
#  include <signal.h>
#endif // HAVE_SIGNAL_H

#ifdef    HAVE_FCNTL_H
#  include <fcntl.h>
#endif // HAVE_FCNTL_H

#ifdef    HAVE_SYS_WAIT_H
#  include <sys/wait.h>
#endif // HAVE_SYS_WAIT_H

#include "gettext.h"
#define _(str) gettext(str)
}

#include <algorithm>
#include <cstdio>
#include <cstdlib>

namespace otk {
extern void initialize();
extern void destroy();
}

namespace ob {

Openbox *openbox = (Openbox *) 0;


void Openbox::signalHandler(int signal)
{
  switch (signal) {
  case SIGUSR1:
    printf("Caught SIGUSR1 signal. Restarting.\n");
    openbox->restart();
    break;

  case SIGCHLD:
    wait(NULL);
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
    otk::EventHandler()
{
  struct sigaction action;

  _state = State_Starting; // initializing everything

  openbox = this;

  _argv = argv;
  _shutdown = false;
  _restart = false;
  _rcfilepath = otk::expandTilde("~/.openbox/rc3");
  _scriptfilepath = otk::expandTilde("~/.openbox/user.py");
  _focused_client = 0;
  _sync = false;
  _single = false;

  parseCommandLine(argc, argv);

  otk::initialize();
  
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
  sigaction(SIGCHLD, &action, (struct sigaction *) 0);

  // anything that died while we were restarting won't give us a SIGCHLD
  while (waitpid(-1, NULL, WNOHANG) > 0);

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

  // initialize all the screens
  _focused_screen = 0;

  for (int i = 0, max = ScreenCount(**otk::display); i < max; ++i) {
    Screen *screen;
    // in single mode skip the screens we don't want to manage
    if (_single && i != DefaultScreen(**otk::display)) {
      _screens.push_back(0);
      continue;
    }
    // try manage the screen
    screen = new Screen(i);
    if (screen->managed()) {
      _screens.push_back(screen);
      if (!_focused_screen) // set this to the first screen managed
        _focused_screen = screen;
    } else {
      delete screen;
      _screens.push_back(0);
    }
  }

  if (_screens.empty()) {
    printf(_("No screens were found without a window manager. Exiting.\n"));
    ::exit(1);
  }

  assert(_focused_screen);

  ScreenList::iterator it, end = _screens.end();
  
  // run the user's script or the system defaults if that fails
  bool pyerr, doretry;
  do {
    // initialize scripting
    python_init(argv[0]);

    // load all of the screens' configs here so we have a theme set if
    // we decide to show the dialog below
    for (it = _screens.begin(); it != end; ++it)
      (*it)->config().load(); // load the defaults from config.py
    
    pyerr = doretry = false;

    // reset all the python stuff
    _bindings->removeAllKeys();
    _bindings->removeAllButtons();
    _bindings->removeAllEvents();

    int ret = python_exec(_scriptfilepath.c_str());
    if (ret == 2)
      pyerr = true;

    if (ret) {
      // reset all the python stuff
      _bindings->removeAllKeys();
      _bindings->removeAllButtons();
      _bindings->removeAllEvents();
      
      if (python_exec(SCRIPTDIR"/defaults.py")) // system default bahaviors
        pyerr = true;
    }

    if (pyerr) {
      std::string msg;
      msg += _("An error occured while executing the python scripts.");
      msg += "\n\n";
      msg += _("See the exact error message in Openbox's output for details.");
      otk::MessageDialog dia(this, _("Python Error"), msg);
      otk::DialogButton ok(_("Okay"), true);
      otk::DialogButton retry(_("Retry"));
      dia.addButton(ok);
      dia.addButton(retry);
      dia.show();
      dia.focus();
      const otk::DialogButton &res = dia.run();
      if (res == retry) {
        doretry = true;
        python_destroy(); // kill all the python modules so they reinit right
      }
    }
  } while (pyerr && doretry);
    
  for (it = _screens.begin(); it != end; ++it) {
    (*it)->config().load(); // load the config as the scripts may change it
    (*it)->manageExisting();
  }
  
  // grab any keys set up before the screens existed
  //_bindings->grabKeys(true);

  // set up input focus
  setFocusedClient(0);
  
  _state = State_Normal; // done starting
}


Openbox::~Openbox()
{
  _state = State_Exiting; // time to kill everything

  std::for_each(_screens.begin(), _screens.end(), otk::PointerAssassin());

  delete _bindings;
  delete _actions;

  python_destroy();

  XSetInputFocus(**otk::display, PointerRoot, RevertToNone,
                 CurrentTime);
  XSync(**otk::display, false);

  otk::destroy();
}


void Openbox::parseCommandLine(int argc, char **argv)
{
  bool err = false;

  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);

    if (arg == "-rc") {
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
  -sync              run in synchronous mode (for debugging X errors).\n\
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
  // sometimes this is called with the already-focused window, this is
  // important for the python scripts to work (eg, c = 0 twice). don't just
  // return if _focused_client == c

  assert(_focused_screen);

  // uninstall the old colormap
  if (_focused_client)
    _focused_client->installColormap(false);
  else
    _focused_screen->installColormap(false);
  
  _focused_client = c;
  if (c) {
    _focused_screen = _screens[c->screen()];

    // install the client's colormap
    c->installColormap(true);
  } else {
    XSetInputFocus(**otk::display, _focused_screen->focuswindow(),
                   RevertToNone, CurrentTime);

    // install the root window colormap
    _focused_screen->installColormap(true);
  }
  // set the NET_ACTIVE_WINDOW hint for all screens
  ScreenList::iterator it, end = _screens.end();
  for (it = _screens.begin(); it != end; ++it) {
    int num = (*it)->number();
    Window root = otk::display->screenInfo(num)->rootWindow();
    otk::Property::set(root, otk::Property::atoms.net_active_window,
                       otk::Property::atoms.window,
                       (c && _focused_screen == *it) ? c->window() : None);
  }

  // call the python Focus callbacks
  EventData data(_focused_screen->number(), c, EventAction::Focus, 0);
  _bindings->fireEvent(&data);
}

}

