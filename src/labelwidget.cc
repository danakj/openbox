// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "labelwidget.hh"

namespace ob {

OBLabelWidget::OBLabelWidget(otk::OtkWidget *parent, OBWidget::WidgetType type)
  : otk::OtkFocusLabel(parent),
    OBWidget(type)
{
}


OBLabelWidget::~OBLabelWidget()
{
}


void OBLabelWidget::setStyle(otk::Style *style)
{
  setTexture(style->getLabelFocus());
  setUnfocusTexture(style->getLabelUnfocus());

  otk::OtkFocusLabel::setStyle(style);
}


void OBLabelWidget::adjust()
{
  otk::OtkFocusLabel::adjust();

  // XXX: adjust shit
}


}
