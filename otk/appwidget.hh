#ifndef __appwidget_hh
#define __appwidget_hh

#include "widget.hh"

namespace otk {

class OtkApplication;

class OtkAppWidget : public OtkWidget {

public:
  OtkAppWidget(OtkApplication *app, Direction direction = Horizontal,
               Cursor cursor = 0, int bevel_width = 1);
  virtual ~OtkAppWidget();

  virtual void show(void);
  virtual void hide(void);

  virtual void clientMessageHandler(const XClientMessageEvent &e);
  
private:

  OtkApplication *_application;
  Atom _wm_protocols;
  Atom _wm_delete;
};

}

#endif // __appwidget_hh
