// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module ob_python

%{
#include "config.h"
#include "python.hh"
%}

namespace ob {

%ignore python_init(char*);
%ignore python_destroy();
%ignore python_exec(const std::string &);

}

%include "python.hh"
