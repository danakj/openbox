// -*- mode: C++; indent-tabs-mode: nil; -*-
#ifndef   __BaseDisplay_hh
#define   __BaseDisplay_hh

#include "screeninfo.hh"
#include "timer.hh"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xatom.h>
}

#include <vector>

// forward declaration
class BaseDisplay;
class BGCCache;

class BaseDisplay: public TimerQueueManager {
private:
  struct BShape {
    bool extensions;
    int event_basep, error_basep;
  };
  BShape shape;

#ifdef    XINERAMA
  struct BXinerama {
    bool extensions;
    int event_basep, error_basep;
    int major, minor; // version
  };
  BXinerama xinerama;
#endif // XINERAMA

  unsigned int MaskList[8];
  size_t MaskListLength;

  enum RunState { STARTUP, RUNNING, SHUTDOWN };
  RunState run_state;

  Display *display;
  mutable BGCCache *gccache;

  typedef std::vector<ScreenInfo> ScreenInfoList;
  ScreenInfoList screenInfoList;
  TimerQueue timerList;

  const char *display_name, *application_name;

  // no copying!
  BaseDisplay(const BaseDisplay &);
  BaseDisplay& operator=(const BaseDisplay&);

protected:
  // pure virtual function... you must override this
  virtual void process_event(XEvent *e) = 0;

  // the masks of the modifiers which are ignored in button events.
  int NumLockMask, ScrollLockMask;


public:
  BaseDisplay(const char *app_name, const char *dpy_name = 0);
  virtual ~BaseDisplay(void);

  const ScreenInfo* getScreenInfo(const unsigned int s) const;

  BGCCache *gcCache(void) const;

  inline bool hasShapeExtensions(void) const
    { return shape.extensions; }
#ifdef    XINERAMA
  inline bool hasXineramaExtensions(void) const
    { return xinerama.extensions; }
#endif // XINERAMA
  inline bool doShutdown(void) const
    { return run_state == SHUTDOWN; }
  inline bool isStartup(void) const
    { return run_state == STARTUP; }

  inline Display *getXDisplay(void) const { return display; }

  inline const char *getXDisplayName(void) const
    { return display_name; }
  inline const char *getApplicationName(void) const
    { return application_name; }

  inline unsigned int getNumberOfScreens(void) const
    { return screenInfoList.size(); }
  inline int getShapeEventBase(void) const
    { return shape.event_basep; }
#ifdef    XINERAMA
  inline int getXineramaMajorVersion(void) const
    { return xinerama.major; }
#endif // XINERAMA

  inline void shutdown(void) { run_state = SHUTDOWN; }
  inline void run(void) { run_state = RUNNING; }

  void grabButton(unsigned int button, unsigned int modifiers,
                  Window grab_window, bool owner_events,
                  unsigned int event_mask, int pointer_mode,
                  int keyboard_mode, Window confine_to, Cursor cursor,
                  bool allow_scroll_lock) const;
  void ungrabButton(unsigned int button, unsigned int modifiers,
                    Window grab_window) const;

  void eventLoop(void);

  // from TimerQueueManager interface
  virtual void addTimer(BTimer *timer);
  virtual void removeTimer(BTimer *timer);

  // another pure virtual... this is used to handle signals that BaseDisplay
  // doesn't understand itself
  virtual bool handleSignal(int sig) = 0;
};


#endif // __BaseDisplay_hh
