// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __obbackgroundwidget_hh
#define   __obbackgroundwidget_hh

#include "otk/focuswidget.hh"
#include "widget.hh"

namespace ob {

class OBBackgroundWidget : public otk::OtkFocusWidget, public OBWidget
{
private:
  
public:
  OBBackgroundWidget(otk::OtkWidget *parent, OBWidget::WidgetType type);
  virtual ~OBBackgroundWidget();

  virtual void setStyle(otk::Style *style);

  virtual void adjust();
};

}

#endif // __obbackgroundwidget_hh
