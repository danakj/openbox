// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module otk_renderstyle

%{
#include "config.h"
#include "renderstyle.hh"
%}

%include "std_string.i"
%include "otk_rendercolor.i"
%include "otk_rendertexture.i"
%include "otk_font.i"

namespace otk {

%ignore StyleNotify;

%ignore RenderStyle::initialize();
%ignore RenderStyle::destroy();
%ignore RenderStyle::registerNotify(int, StyleNotify*);
%ignore RenderStyle::unregisterNotify(int, StyleNotify *);
%ignore RenderStyle::~RenderStyle();

}

%include "renderstyle.hh"
