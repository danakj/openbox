// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module ob_openbox

%{
#include "config.h"
#include "openbox.hh"
%}

%include "ob_screen.i"
%include "ob_client.i"
%include "std_string.i"

namespace ob {

%ignore Cursors;

%immutable openbox;

%ignore Openbox::Openbox(int, char **);
%ignore Openbox::~Openbox();
%ignore Openbox::actions() const;
%ignore Openbox::bindings() const;
%ignore Openbox::cursors() const;
%ignore Openbox::eventLoop();
%ignore Openbox::addClient(Window, Client*);
%ignore Openbox::removeClient(Window);
%ignore Openbox::setFocusedClient(Client*);
%ignore Openbox::doRestart() const;
%ignore Openbox::restartProgram() const;

}

%import "../otk/eventdispatcher.hh"
%include "openbox.hh"
