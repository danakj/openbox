// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module ob_python

%{
#include "config.h"
#include "python.hh"
%}

namespace ob {

%ignore MouseContext::NUM_MOUSE_CONTEXT;
%ignore MouseAction::NUM_MOUSE_ACTION;
%ignore KeyContext::NUM_KEY_CONTEXT;
%ignore KeyAction::NUM_KEY_ACTION;
%ignore EventAction::NUM_EVENT_ACTION;

%ignore python_init(char*);
%ignore python_destroy();
%ignore python_exec(const std::string &);

}

%include "python.hh"
