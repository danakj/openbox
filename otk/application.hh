#ifndef __application_hh
#define __application_hh

#include "eventdispatcher.hh"
#include "display.hh"
#include "configuration.hh"
#include "timerqueuemanager.hh"
#include "image.hh"
#include "style.hh"

namespace otk {

class OtkWidget;

class OtkApplication : public OtkEventDispatcher {

public:

  OtkApplication(int argc, char **argv);
  virtual ~OtkApplication();

  virtual void exec(void);
  // more bummy cool functionality

  void setDockable(bool dockable) { _dockable = dockable; }
  inline bool isDockable(void) const { return _dockable; }

  inline Style *getStyle(void) const { return _style; }
  // more accessors

protected:
  bool setMainWidget(const OtkWidget *main_widget);

private:
  void loadStyle(void);

  const OtkWidget *_main_widget;
  OBTimerQueueManager *_timer_manager;
  BImageControl *_img_ctrl;
  Configuration *_style_conf;
  Style *_style;
  bool _dockable;

  friend class OtkWidget; // for access to setMainWidget
};

}

#endif
