// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __backgroundwidget_hh
#define   __backgroundwidget_hh

#include "otk/widget.hh"
#include "widgetbase.hh"

namespace ob {

class BackgroundWidget : public otk::Widget, public WidgetBase
{
private:
  void setTextures();
  
public:
  BackgroundWidget(otk::Widget *parent, WidgetBase::WidgetType type);
  virtual ~BackgroundWidget();

  virtual void setStyle(otk::RenderStyle *style);

  virtual void adjust();

  virtual void focus();
  virtual void unfocus();
};

}

#endif // __backgroundwidget_hh
