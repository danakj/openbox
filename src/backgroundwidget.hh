// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __obbackgroundwidget_hh
#define   __obbackgroundwidget_hh

#include "otk/widget.hh"
#include "widget.hh"

namespace ob {

class OBBackgroundWidget : public otk::OtkWidget, public OBWidget
{
private:
  void setTextures();
  
public:
  OBBackgroundWidget(otk::OtkWidget *parent, OBWidget::WidgetType type);
  virtual ~OBBackgroundWidget();

  virtual void setStyle(otk::Style *style);

  virtual void adjust();

  virtual void focus();
  virtual void unfocus();
};

}

#endif // __obbackgroundwidget_hh
