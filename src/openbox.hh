// -*- mode: C++; indent-tabs-mode: nil; -*-
#ifndef   __openbox_hh
#define   __openbox_hh

extern "C" {
#include <X11/Xlib.h>
}

#include <string>
#include <vector>

#include "otk/screeninfo.hh"

namespace ob {

class Openbox
{
public:
  static Openbox *instance;  // there can only be ONE instance of this class in
                             // the program, and it is held in here

  typedef std::vector<otk::ScreenInfo> ScreenInfoList;

  enum RunState {
    State_Starting,
    State_Normal,
    State_Exiting
  };
  
private:
  std::string _rcfilepath;   // path to the config file to use/in use
  std::string _menufilepath; // path to the menu file to use/in use
  char *_displayreq;         // display requested by the user
  char *_argv0;              // argv[0], how the program was called

  RunState _state;           // the state of the window manager

  ScreenInfoList _screenInfoList; // info for all screens on the display

  void parseCommandLine(int argv, char **argv);
  void showVersion();
  void showHelp();

  static int xerrorHandler(Display *d, XErrorEvent *e);
  static void signalHandler(int signal);

public:
  //! Openbox constructor.
  /*!
    \param argc Number of command line arguments, as received in main()
    \param argv The command line arguments, as received in main()
  */
  Openbox(int argc, char **argv);
  //! Openbox destructor.
  virtual ~Openbox();

  //! Returns the state of the window manager (starting, exiting, etc).
  inline RunState state() const { return _state; }

  void eventLoop();

  // XXX: TEMPORARY!#!@%*!^#*!#!#!
  virtual void process_event(XEvent *) = 0;

  //! Requests that the window manager exit.
  inline void shutdown() { _state = State_Exiting; }
};

}

#endif // __openbox_hh
