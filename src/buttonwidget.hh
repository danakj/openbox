// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __obbuttonwidget_hh
#define   __obbuttonwidget_hh

#include "otk/button.hh"
#include "widget.hh"

namespace ob {

class OBButtonWidget : public otk::OtkButton, public OBWidget
{
private:
  
public:
  OBButtonWidget(otk::OtkWidget *parent, OBWidget::WidgetType type);
  virtual ~OBButtonWidget();

  virtual void setStyle(otk::Style *style);

  virtual void adjust();
};

}

#endif // __obbuttonwidget_hh
