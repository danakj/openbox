// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "rootwindow.hh"
#include "openbox.hh"
#include "screen.hh"
#include "otk/display.hh"

namespace ob {

OBRootWindow::OBRootWindow(int screen)
  : OBWidget(OBWidget::Type_Root),
    _info(otk::OBDisplay::screenInfo(screen))
{
  updateDesktopNames();

  Openbox::instance->registerHandler(_info->rootWindow(), this);
}


OBRootWindow::~OBRootWindow()
{
}


void OBRootWindow::updateDesktopNames()
{
  const int numWorkspaces = 1; // XXX: change this to the number of workspaces!

  const otk::OBProperty *property = Openbox::instance->property();

  unsigned long num = (unsigned) -1;
  
  if (!property->get(_info->rootWindow(),
                     otk::OBProperty::net_desktop_names,
                     otk::OBProperty::utf8, &num, &_names))
    _names.clear();
  while ((signed)_names.size() < numWorkspaces)
    _names.push_back("Unnamed");
}


void OBRootWindow::propertyHandler(const XPropertyEvent &e)
{
  otk::OtkEventHandler::propertyHandler(e);

  const otk::OBProperty *property = Openbox::instance->property();

  // compress changes to a single property into a single change
  XEvent ce;
  while (XCheckTypedEvent(otk::OBDisplay::display, e.type, &ce)) {
    // XXX: it would be nice to compress ALL changes to a property, not just
    //      changes in a row without other props between.
    if (ce.xproperty.atom != e.atom) {
      XPutBackEvent(otk::OBDisplay::display, &ce);
      break;
    }
  }

  if (e.atom == property->atom(otk::OBProperty::net_desktop_names)) 
    updateDesktopNames();
}


void OBRootWindow::clientMessageHandler(const XClientMessageEvent &e)
{
  otk::OtkEventHandler::clientMessageHandler(e);

  if (e.format != 32) return;

  //const otk::OBProperty *property = Openbox::instance->property();
  
  // XXX: so many client messages to handle here! ..or not.. they go to clients
}


void OBRootWindow::setDesktopNames(const otk::OBProperty::StringVect &names)
{
  _names = names;
  const otk::OBProperty *property = Openbox::instance->property();
  property->set(_info->rootWindow(), otk::OBProperty::net_desktop_names,
                otk::OBProperty::utf8, names);
}

void OBRootWindow::setDesktopName(int i, const std::string &name)
{
  const int numWorkspaces = 1; // XXX: change this to the number of workspaces!
  assert(i >= 0);
  assert(i < numWorkspaces); 

  const otk::OBProperty *property = Openbox::instance->property();
  
  otk::OBProperty::StringVect newnames = _names;
  newnames[i] = name;
  property->set(_info->rootWindow(), otk::OBProperty::net_desktop_names,
                otk::OBProperty::utf8, newnames);
}


void OBRootWindow::mapRequestHandler(const XMapRequestEvent &e)
{
#ifdef    DEBUG
  printf("MapRequest for 0x%lx\n", e.window);
#endif // DEBUG

  OBClient *client = Openbox::instance->findClient(e.window);

  if (client) {
    // XXX: uniconify and/or unshade the window
  } else {
    Openbox::instance->screen(_info->screen())->manageWindow(e.window);
  }
}

}
