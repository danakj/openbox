// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module openbox

%{
#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "openbox.hh"
#include "screen.hh"
#include "client.hh"
%}

%immutable ob::Openbox::instance;

%include "openbox.hh"
%include "screen.hh"
%include "client.hh"


%include stl.i
%include std_list.i

%{
class OBClient;
%}
%template(ClientList) std::list<OBClient*>;
