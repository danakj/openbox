#ifndef __application_hh
#define __application_hh

#include "eventdispatcher.hh"
#include "display.hh"
#include "configuration.hh"
#include "timerqueuemanager.hh"
#include "image.hh"
#include "style.hh"

namespace otk {

class OtkAppWidget;

class OtkApplication : public OtkEventDispatcher {

public:

  OtkApplication(int argc, char **argv);
  virtual ~OtkApplication();

  virtual void run(void);
  // more bummy cool functionality

  void setDockable(bool dockable) { _dockable = dockable; }
  inline bool isDockable(void) const { return _dockable; }

  inline otk::Style *getStyle(void) const { return _style; }
  // more accessors

private:
  void loadStyle(void);

  OBTimerQueueManager *_timer_manager;
  BImageControl *_img_ctrl;
  Configuration *_style_conf;
  Style *_style;
  bool _dockable;

  int _appwidget_count;

  friend class OtkAppWidget;
};

}

#endif
