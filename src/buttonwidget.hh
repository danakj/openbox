// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __buttonwidget_hh
#define   __buttonwidget_hh

#include "widgetbase.hh"
#include "otk/widget.hh"

namespace ob {

class ButtonWidget : public otk::Widget, public WidgetBase
{
private:
  void setTextures();
  bool _pressed;
  unsigned int _button;
  
public:
  ButtonWidget(otk::Widget *parent, WidgetBase::WidgetType type);
  virtual ~ButtonWidget();

  virtual void setStyle(otk::Style *style);

  virtual void adjust();

  virtual void update();
  
  virtual void focus();
  virtual void unfocus();

  virtual void buttonPressHandler(const XButtonEvent &e);
  virtual void buttonReleaseHandler(const XButtonEvent &e);
};

}

#endif // __buttonwidget_hh
