#ifndef __widget_hh
#define __widget_hh

#include "basewidget.hh"

extern "C" {
#include <assert.h>
}

namespace otk {

class OtkWidget : public OtkBaseWidget {

public:

  enum Direction { Horizontal, Vertical };

  OtkWidget(OtkWidget *parent, Direction = Horizontal);
  OtkWidget(OtkEventDispatcher *event_dispatcher, Style *style,
            Direction direction = Horizontal, Cursor cursor = 0,
            int bevel_width = 1);

  virtual ~OtkWidget();

  virtual void update(void);

  inline bool isStretchableHorz(void) const { return _stretchable_horz; }
  void setStretchableHorz(bool s_horz = true) { _stretchable_horz = s_horz; }

  inline bool isStretchableVert(void) const { return _stretchable_vert; }
  void setStretchableVert(bool s_vert = true)  { _stretchable_vert = s_vert; }

  inline Direction getDirection(void) const { return _direction; }
  void setDirection(Direction dir) { _direction = dir; }

  inline OtkEventDispatcher *getEventDispatcher(void)
  { return _event_dispatcher; }
  void setEventDispatcher(OtkEventDispatcher *disp);

private:

  void adjust(void);
  void adjustHorz(void);
  void adjustVert(void);
  void internalResize(int width, int height);

  Direction _direction;

  bool _stretchable_vert;
  bool _stretchable_horz;

  OtkEventDispatcher *_event_dispatcher;
};

}

#endif // __widget_hh
