// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "rootwindow.hh"
#include "openbox.hh"
#include "otk/display.hh"

namespace ob {

OBRootWindow::OBRootWindow(int screen)
  : _info(otk::OBDisplay::screenInfo(screen))
{
  updateDesktopNames();

  Openbox::instance->registerHandler(_info->getRootWindow(), this);
}


OBRootWindow::~OBRootWindow()
{
}


void OBRootWindow::updateDesktopNames()
{
  const int numWorkspaces = 1; // XXX: change this to the number of workspaces!

  const otk::OBProperty *property = Openbox::instance->property();

  unsigned long num = (unsigned) -1;
  
  if (!property->get(_info->getRootWindow(),
                     otk::OBProperty::net_desktop_names,
                     otk::OBProperty::utf8, &num, &_names))
    _names.clear();
  for (int i = 0; i < numWorkspaces; ++i)
    if (i <= static_cast<int>(_names.size()))
      _names.push_back("Unnamed workspace");
}


void OBRootWindow::propertyHandler(const XPropertyEvent &e)
{
  otk::OtkEventHandler::propertyHandler(e);

  const otk::OBProperty *property = Openbox::instance->property();

  if (e.atom == property->atom(otk::OBProperty::net_desktop_names))
    updateDesktopNames();
}


void OBRootWindow::clientMessageHandler(const XClientMessageEvent &e)
{
  otk::OtkEventHandler::clientMessageHandler(e);

  if (e.format != 32) return;

  //const otk::OBProperty *property = Openbox::instance->property();
  
  // XXX: so many client messages to handle here!
}


void OBRootWindow::setDesktopName(int i, const std::string &name)
{
  const int numWorkspaces = 1; // XXX: change this to the number of workspaces!
  assert(i >= 0);
  assert(i < numWorkspaces); 

  const otk::OBProperty *property = Openbox::instance->property();
  
  otk::OBProperty::StringVect newnames = _names;
  newnames[i] = name;
  property->set(_info->getRootWindow(), otk::OBProperty::net_desktop_names,
                otk::OBProperty::utf8, newnames);
}


}
