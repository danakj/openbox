// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module ob_client

%{
#include "config.h"
#include "client.hh"
%}

%include "otk_size.i"
%include "otk_ustring.i"
%include "std_string.i"

namespace ob {

%immutable Client::frame;

%ignore Client::event_mask;
%ignore Client::no_propagate_mask;
%ignore Client::ignore_unmaps;
%ignore Client::Client(int, Window);
%ignore Client::~Client();
%ignore Client::icon(const otk::Size &) const;
%ignore Client::pixmapIcon() const;
%ignore Client::pixmapIconMask() const;
%ignore Client::showhide();
%ignore Client::focus();
%ignore Client::unfocus() const;
%ignore Client::validate() const;
%ignore Client::installColormap(bool) const;
%ignore Client::focusHandler(const XFocusChangeEvent &);
%ignore Client::unfocusHandler(const XFocusChangeEvent &);
%ignore Client::propertyHandler(const XPropertyEvent &);
%ignore Client::clientMessageHandler(const XClientMessageEvent &);
%ignore Client::configureRequestHandler(const XConfigureRequestEvent &);
%ignore Client::unmapHandler(const XUnmapEvent &);
%ignore Client::destroyHandler(const XDestroyWindowEvent &);
%ignore Client::reparentHandler(const XReparentEvent &);
%ignore Client::mapRequestHandler(const XMapRequestEvent &);
%ignore Client::shapeHandler(const XShapeEvent &);


%extend Client {
  void focus(bool unshade = false, bool raise = false) {
    Window root = otk::display->screenInfo(self->screen())->rootWindow();
    send_client_msg(root, otk::Property::atoms.openbox_active_window,
                    self->window(), unshade ? 1 : 0, raise ? 1 : 0);
  }
  
  bool __cmp__(const Client *o) { return self->window() != o->window(); }

  void raiseWindow() {
    Window root = RootWindow(**otk::display, self->screen());
    send_client_msg(root, otk::Property::atoms.openbox_restack_window,
                    self->window(), 0);
  }
  void lowerWindow() {
    Window root = RootWindow(**otk::display, self->screen());
    send_client_msg(root, otk::Property::atoms.openbox_restack_window,
                    self->window(), 1);
  }

  void setDesktop(unsigned int desktop) {
    Window root = RootWindow(**otk::display, self->screen());
    send_client_msg(root, otk::Property::atoms.net_wm_desktop,
                    self->window(), desktop);
  }

  void iconify(bool icon = true) {
    Window root = RootWindow(**otk::display, self->screen());
    send_client_msg(root, otk::Property::atoms.wm_change_state,
                    self->window(), icon ? IconicState : NormalState);
  }

  void close() {
    Window root = RootWindow(**otk::display, self->screen());
    send_client_msg(root, otk::Property::atoms.net_close_window,
                    self->window(), 0);
  }

  void shade(bool shade = true) {
    Window root = RootWindow(**otk::display, self->screen());
    send_client_msg(root, otk::Property::atoms.net_wm_state,
                    self->window(),
                    (shade ? otk::Property::atoms.net_wm_state_add :
                             otk::Property::atoms.net_wm_state_remove),
                    otk::Property::atoms.net_wm_state_shaded);
  }

  void maximize(bool max = true) { 
    Window root = RootWindow(**otk::display, self->screen());
    send_client_msg(root, otk::Property::atoms.net_wm_state,
                    self->window(),
                    (max ? otk::Property::atoms.net_wm_state_add :
                           otk::Property::atoms.net_wm_state_remove),
                    otk::Property::atoms.net_wm_state_maximized_horz,
                    otk::Property::atoms.net_wm_state_maximized_vert);
 }

  void maximizeHorizontal(bool max = true) {
    Window root = RootWindow(**otk::display, self->screen());
    send_client_msg(root, otk::Property::atoms.net_wm_state,
                    self->window(),
                    (max ? otk::Property::atoms.net_wm_state_add :
                           otk::Property::atoms.net_wm_state_remove),
                    otk::Property::atoms.net_wm_state_maximized_horz);
  }

  void maximizeVertical(bool max = true) {
    Window root = RootWindow(**otk::display, self->screen());
    send_client_msg(root, otk::Property::atoms.net_wm_state,
                    self->window(),
                    (max ? otk::Property::atoms.net_wm_state_add :
                           otk::Property::atoms.net_wm_state_remove),
                    otk::Property::atoms.net_wm_state_maximized_vert);
  }

  void setSkipTaskbar(bool skip = true) {
    Window root = RootWindow(**otk::display, self->screen());
    send_client_msg(root, otk::Property::atoms.net_wm_state,
                    self->window(),
                    (skip ? otk::Property::atoms.net_wm_state_add :
                            otk::Property::atoms.net_wm_state_remove),
                    otk::Property::atoms.net_wm_state_skip_taskbar);
  }

  void setSkipPager(bool skip = true) {
    Window root = RootWindow(**otk::display, self->screen());
    send_client_msg(root, otk::Property::atoms.net_wm_state,
                    self->window(),
                    (skip ? otk::Property::atoms.net_wm_state_add :
                            otk::Property::atoms.net_wm_state_remove),
                    otk::Property::atoms.net_wm_state_skip_pager);
  }

  void setAbove(bool above = true) {
    Window root = RootWindow(**otk::display, self->screen());
    send_client_msg(root, otk::Property::atoms.net_wm_state,
                    self->window(),
                    (above ? otk::Property::atoms.net_wm_state_add :
                            otk::Property::atoms.net_wm_state_remove),
                    otk::Property::atoms.net_wm_state_above);
  }

  void setBelow(bool below = true) {
    Window root = RootWindow(**otk::display, self->screen());
    send_client_msg(root, otk::Property::atoms.net_wm_state,
                    self->window(),
                    (below ? otk::Property::atoms.net_wm_state_add :
                            otk::Property::atoms.net_wm_state_remove),
                    otk::Property::atoms.net_wm_state_below);
  }
};

}

%import "../otk/eventhandler.hh"
%include "client.hh"
