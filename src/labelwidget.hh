// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __oblabelwidget_hh
#define   __oblabelwidget_hh

#include "otk/focuslabel.hh"
#include "widget.hh"

namespace ob {

class OBLabelWidget : public otk::OtkFocusLabel, public OBWidget
{
private:
  
public:
  OBLabelWidget(otk::OtkWidget *parent, OBWidget::WidgetType type);
  virtual ~OBLabelWidget();

  virtual void setStyle(otk::Style *style);

  virtual void adjust();
};

}

#endif // __oblabelwidget_hh
