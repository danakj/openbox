// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module otk_appwidget

%{
#include "config.h"
#include "appwidget.hh"
%}

%include "otk_widget.i"
%include "otk_application.i"

namespace otk {

%ignore AppWidget::clientMessageHandler(const XClientMessageEvent &);


}

%include "appwidget.hh"
