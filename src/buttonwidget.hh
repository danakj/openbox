// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __obbuttonwidget_hh
#define   __obbuttonwidget_hh

#include "otk/widget.hh"
#include "widget.hh"

namespace ob {

class OBButtonWidget : public otk::OtkWidget, public OBWidget
{
private:
  void setTextures();
  bool _pressed;
  unsigned int _button;
  
public:
  OBButtonWidget(otk::OtkWidget *parent, OBWidget::WidgetType type);
  virtual ~OBButtonWidget();

  virtual void setStyle(otk::Style *style);

  virtual void adjust();

  virtual void update();
  
  virtual void focus();
  virtual void unfocus();

  virtual void buttonPressHandler(const XButtonEvent &e);
  virtual void buttonReleaseHandler(const XButtonEvent &e);
};

}

#endif // __obbuttonwidget_hh
