// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module otk_rendercolor

%{
#include "config.h"
#include "rendercolor.hh"
%}

namespace otk {

%ignore RenderColor::initialize();
%ignore RenderColor::destroy();

}

%include "rendercolor.hh"
