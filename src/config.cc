// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

/*
  python_get_stringlist("DESKTOP_NAMES", &desktop_names);

  python_get_string("THEME", &theme);
  // initialize the screen's style
  otk::RenderStyle::setStyle(_screen, theme);
  // draw the root window
  otk::bexec("obsetroot " + otk::RenderStyle::style(_screen)->rootArgs(),
             info->displayString());


  if (!python_get_string("TITLEBAR_LAYOUT", &titlebar_layout)) {
    fprintf(stderr, _("Unable to load config.%s\n"), "TITLEBAR_LAYOUT");
    ::exit(1);
  }

  if (!python_get_long("DOUBLE_CLICK_DELAY", &double_click_delay)) {
    fprintf(stderr, _("Unable to load config.%s\n"), "DOUBLE_CLICK_DELAY");
    ::exit(1);
  }
  if (!python_get_long("DRAG_THRESHOLD", &drag_threshold)) {
    fprintf(stderr, _("Unable to load config.%s\n"), "DRAG_THRESHOLD");
    ::exit(1);
  }
  if (!python_get_long("NUMBER_OF_DESKTOPS", (long*)&num_desktops)) {
    fprintf(stderr, _("Unable to load config.%s\n"), "NUMBER_OF_DESKTOPS");
    ::exit(1);
  }

  // Set the net_desktop_names property
  otk::Property::set(root,
                     otk::Property::atoms.net_desktop_names,
                     otk::Property::utf8, desktop_names);
  // the above set() will cause screen::updateDesktopNames to fire right away
  // so we have a list of desktop names

  XEvent ce;
  ce.xclient.type = ClientMessage;
  ce.xclient.message_type = otk::Property::atoms.net_number_of_desktops;
  ce.xclient.display = **otk::display;
  ce.xclient.window = root;
  ce.xclient.format = 32;
  ce.xclient.data.l[0] = num_desktops;
  XSendEvent(**otk::display, root, False,
	     SubstructureNotifyMask | SubstructureRedirectMask, &ce);
}

Config::Config(int screen)
  : _screen(screen)
{
}

Config::~Config()
{
}

*/
