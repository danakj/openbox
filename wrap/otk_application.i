// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module otk_application

%{
#include "config.h"
#include "application.hh"
%}

%include "otk_widget.i"

%import "../otk/eventdispatcher.hh"
%include "application.hh"
