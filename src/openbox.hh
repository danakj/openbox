// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __openbox_hh
#define   __openbox_hh

/*! @file openbox.hh
  @brief The main class for the Openbox window manager
*/

/*
  cuz girls look soooo goood.. on the end of my DICK
*/

extern "C" {
#include <X11/Xlib.h>
}

#include <string>
#include <vector>
#include <map>

#include "otk/screeninfo.hh"
#include "otk/timerqueuemanager.hh"
#include "otk/property.hh"
#include "otk/configuration.hh"
#include "otk/eventdispatcher.hh"
#include "otk/eventhandler.hh"

namespace ob {

class OBScreen;
class OBClient;
class OBActions;

//! Mouse cursors used throughout Openbox
struct Cursors {
  Cursor session;  //!< The default mouse cursor
  Cursor move;     //!< For moving a window
  Cursor ll_angle; //!< For resizing the bottom left corner of a window
  Cursor lr_angle; //!< For resizing the bottom right corner of a window
  Cursor ul_angle; //!< For resizing the top left corner of a window
  Cursor ur_angle; //!< For resizing the right corner of a window
};


//! The main class for the Openbox window manager
/*!
  Only a single instance of the Openbox class may be used in the application. A
  pointer to this instance is held in the Openbox::instance static member
  variable.
  Instantiation of this class begins the window manager. After instantiation,
  the Openbox::eventLoop function should be called. The eventLoop method does
  not exit until the window manager is ready to be destroyed. Destruction of
  the Openbox class instance will shutdown the window manager.
*/
class Openbox : public otk::OtkEventDispatcher, public otk::OtkEventHandler
{
public:
  //! The single instance of the Openbox class for the application
  /*!
    Since this variable is globally available in the application, the Openbox
    class does not need to be passed around to any of the other classes.
  */
  static Openbox *instance;

  //! The posible running states of the window manager
  enum RunState {
    State_Starting, //!< The window manager is starting up (being created)
    State_Normal,   //!< The window manager is running in its normal state
    State_Exiting   //!< The window manager is exiting (being destroyed)
  };

  //! A map for looking up a specific client class from the window id
  typedef std::map<Window, OBClient *> ClientMap;

  //! A list of OBScreen classes
  typedef std::vector<OBScreen *> ScreenList;
  
private:
  // stuff that can be passed on the command line
  //! Path to the config file to use/in use
  /*!
    Defaults to $(HOME)/.openbox/rc3
  */
  std::string _rcfilepath;
  //! Path to the menu file to use/in use
  /*!
    Defaults to $(HOME)/.openbox/menu3
  */
  std::string _menufilepath;
  //! The display requested by the user, or null to use the DISPLAY env var
  char *_displayreq;
  //! The value of argv[0], i.e. how this application was executed
  char *_argv0;

  //! A list of all managed clients
  ClientMap _clients;

  //! A list of all the managed screens
  ScreenList _screens;
  
  //! Manages all timers for the application
  /*!
    Use of the otk::OBTimerQueueManager::fire funtion in this object ensures
    that all timers fire when their times elapse.
  */
  otk::OBTimerQueueManager _timermanager;

  //! Cached atoms on the display
  /*!
    This is a pointer because the OBProperty class uses otk::OBDisplay::display
    in its constructor, so, it needs to be initialized <b>after</b> the display
    is initialized in this class' constructor.
  */
  otk::OBProperty *_property;

  //! The action interface through which all user-available actions occur
  OBActions *_actions;

  //! The running state of the window manager
  RunState _state;

  //! Mouse cursors used throughout Openbox
  Cursors _cursors;

  //! When set to true, the Openbox::eventLoop function will stop and return
  bool _doshutdown;

  //! The configuration of the application. TEMPORARY
  otk::Configuration _config;

  //! Parses the command line used when executing this application
  void parseCommandLine(int argv, char **argv);
  //! Displays the version string to stdout
  void showVersion();
  //! Displays usage information and help to stdout
  void showHelp();

  //! Handles signal events for the application
  static void signalHandler(int signal);

public:
#ifndef SWIG
  //! Openbox constructor.
  /*!
    \param argc Number of command line arguments, as received in main()
    \param argv The command line arguments, as received in main()
  */
  Openbox(int argc, char **argv);
  //! Openbox destructor.
  virtual ~Openbox();
#endif

  //! Returns the state of the window manager (starting, exiting, etc)
  inline RunState state() const { return _state; }

  //! Returns the otk::OBTimerQueueManager for the application
  /*!
    All otk::OBTimer objects used in the application should be made to use this
    otk::OBTimerQueueManager.
  */
  inline otk::OBTimerQueueManager *timerManager() { return &_timermanager; }

  //! Returns the otk::OBProperty instance for the window manager
  inline const otk::OBProperty *property() const { return _property; }

  //! Returns a managed screen
  inline OBScreen *screen(int num) {
    assert(num >= 0); assert(num < (signed)_screens.size());
    return _screens[num];
  }

  //! Returns the mouse cursors used throughout Openbox
  inline const Cursors &cursors() const { return _cursors; }

#ifndef SWIG
  //! The main function of the Openbox class
  /*!
    This function should be called after instantiating the Openbox class.
    It loops indefinately while handling all events for the application.
    The Openbox::shutdown method will cause this function to exit.
  */
  void eventLoop();
#endif

  //! Adds an OBClient to the client list for lookups
  void addClient(Window window, OBClient *client);

  //! Removes an OBClient from the client list for lookups
  void removeClient(Window window);

  //! Finds an OBClient based on its window id
  OBClient *findClient(Window window);

  //! Requests that the window manager exit
  /*!
    Causes the Openbox::eventLoop function to stop looping, so that the window
    manager can be destroyed.
  */
  inline void shutdown() { _doshutdown = true; }
};

}

#endif // __openbox_hh
