// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "client.hh"
#include "frame.hh"
#include "screen.hh"
#include "openbox.hh"
#include "otk/display.hh"
#include "otk/property.hh"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <assert.h>

#include "gettext.h"
#define _(str) gettext(str)
}

namespace ob {

Client::Client(int screen, Window window)
  : otk::EventHandler(),
    WidgetBase(WidgetBase::Type_Client),
    frame(0), _screen(screen), _window(window)
{
  assert(screen >= 0);
  assert(window);

  ignore_unmaps = 0;
  
  // update EVERYTHING the first time!!

  // the state is kinda assumed to be normal. is this right? XXX
  _wmstate = NormalState; _iconic = false;
  // no default decors or functions, each has to be enabled
  _decorations = _functions = 0;
  // start unfocused
  _focused = false;
  // not a transient by default of course
  _transient_for = 0;
  // pick a layer to start from
  _layer = Layer_Normal;
  
  getGravity();
  getArea();
  getDesktop();

  updateTransientFor();
  getType();
  getMwmHints();

  setupDecorAndFunctions();
  
  getState();
  getShaped();

  updateProtocols();
  updateWMHints();
  updateTitle();
  updateIconTitle();
  updateClass();
  updateStrut();

  changeState();
}


Client::~Client()
{
  // clean up childrens' references
  while (!_transients.empty()) {
    _transients.front()->_transient_for = 0;
    _transients.pop_front();
  }
  
  // clean up parents reference to this
  if (_transient_for)
    _transient_for->_transients.remove(this); // remove from old parent
  
  if (openbox->state() != Openbox::State_Exiting) {
    // these values should not be persisted across a window unmapping/mapping
    otk::Property::erase(_window, otk::Property::atoms.net_wm_desktop);
    otk::Property::erase(_window, otk::Property::atoms.net_wm_state);
  }
}


void Client::getGravity()
{
  XSizeHints size;
  XWindowAttributes wattrib;
  Status ret;
  long junk;

  if (XGetWMNormalHints(**otk::display, _window, &size, &junk) &&
      size.flags & PWinGravity)
    // first try the normal hints
    _gravity = size.win_gravity;
  else {
    // then fall back to the attribute
    ret = XGetWindowAttributes(**otk::display, _window, &wattrib);
    assert(ret != BadWindow);
    _gravity = wattrib.win_gravity;
  }
}


void Client::getDesktop()
{
  // defaults to the current desktop
  _desktop = openbox->screen(_screen)->desktop();

  if (!otk::Property::get(_window, otk::Property::atoms.net_wm_desktop,
                          otk::Property::atoms.cardinal,
                          (long unsigned*)&_desktop)) {
    // make sure the hint exists
    otk::Property::set(_window, otk::Property::atoms.net_wm_desktop,
                       otk::Property::atoms.cardinal, (unsigned)_desktop);
  }
}


void Client::getType()
{
  _type = (WindowType) -1;
  
  unsigned long *val;
  unsigned long num = (unsigned) -1;
  if (otk::Property::get(_window, otk::Property::atoms.net_wm_window_type,
                         otk::Property::atoms.atom, &num, &val)) {
    // use the first value that we know about in the array
    for (unsigned long i = 0; i < num; ++i) {
      if (val[i] == otk::Property::atoms.net_wm_window_type_desktop)
        _type = Type_Desktop;
      else if (val[i] == otk::Property::atoms.net_wm_window_type_dock)
        _type = Type_Dock;
      else if (val[i] == otk::Property::atoms.net_wm_window_type_toolbar)
        _type = Type_Toolbar;
      else if (val[i] == otk::Property::atoms.net_wm_window_type_menu)
        _type = Type_Menu;
      else if (val[i] == otk::Property::atoms.net_wm_window_type_utility)
        _type = Type_Utility;
      else if (val[i] == otk::Property::atoms.net_wm_window_type_splash)
        _type = Type_Splash;
      else if (val[i] == otk::Property::atoms.net_wm_window_type_dialog)
        _type = Type_Dialog;
      else if (val[i] == otk::Property::atoms.net_wm_window_type_normal)
        _type = Type_Normal;
//    XXX: make this work again
//    else if (val[i] == otk::Property::atoms.kde_net_wm_window_type_override)
//      mwm_decorations = 0; // prevent this window from getting any decor
      if (_type != (WindowType) -1)
        break; // grab the first known type
    }
    delete val;
  }
    
  if (_type == (WindowType) -1) {
    /*
     * the window type hint was not set, which means we either classify ourself
     * as a normal window or a dialog, depending on if we are a transient.
     */
    if (_transient_for)
      _type = Type_Dialog;
    else
      _type = Type_Normal;
  }
}


void Client::setupDecorAndFunctions()
{
  // start with everything
  _decorations = Decor_Titlebar | Decor_Handle | Decor_Border |
    Decor_Iconify | Decor_Maximize;
  _functions = Func_Resize | Func_Move | Func_Iconify | Func_Maximize;
  
  switch (_type) {
  case Type_Normal:
    // normal windows retain all of the possible decorations and
    // functionality

  case Type_Dialog:
    // dialogs cannot be maximized
    _decorations &= ~Decor_Maximize;
    _functions &= ~Func_Maximize;
    break;

  case Type_Menu:
  case Type_Toolbar:
  case Type_Utility:
    // these windows get less functionality
    _decorations &= ~(Decor_Iconify | Decor_Handle);
    _functions &= ~(Func_Iconify | Func_Resize);
    break;

  case Type_Desktop:
  case Type_Dock:
  case Type_Splash:
    // none of these windows are manipulated by the window manager
    _decorations = 0;
    _functions = 0;
    break;
  }

  // Mwm Hints are applied subtractively to what has already been chosen for
  // decor and functionality
  if (_mwmhints.flags & MwmFlag_Decorations) {
    if (! (_mwmhints.decorations & MwmDecor_All)) {
      if (! (_mwmhints.decorations & MwmDecor_Border))
        _decorations &= ~Decor_Border;
      if (! (_mwmhints.decorations & MwmDecor_Handle))
        _decorations &= ~Decor_Handle;
      if (! (_mwmhints.decorations & MwmDecor_Title))
        _decorations &= ~Decor_Titlebar;
      if (! (_mwmhints.decorations & MwmDecor_Iconify))
        _decorations &= ~Decor_Iconify;
      if (! (_mwmhints.decorations & MwmDecor_Maximize))
        _decorations &= ~Decor_Maximize;
    }
  }

  if (_mwmhints.flags & MwmFlag_Functions) {
    if (! (_mwmhints.functions & MwmFunc_All)) {
      if (! (_mwmhints.functions & MwmFunc_Resize))
        _functions &= ~Func_Resize;
      if (! (_mwmhints.functions & MwmFunc_Move))
        _functions &= ~Func_Move;
      if (! (_mwmhints.functions & MwmFunc_Iconify))
        _functions &= ~Func_Iconify;
      if (! (_mwmhints.functions & MwmFunc_Maximize))
        _functions &= ~Func_Maximize;
      // dont let mwm hints kill the close button
      //if (! (_mwmhints.functions & MwmFunc_Close))
      //  _functions &= ~Func_Close;
    }
  }

  // XXX: changeAllowedActions();
}


void Client::getMwmHints()
{
  unsigned long num = MwmHints::elements;
  unsigned long *hints;

  _mwmhints.flags = 0; // default to none
  
  if (!otk::Property::get(_window, otk::Property::atoms.motif_wm_hints,
                          otk::Property::atoms.motif_wm_hints, &num,
                          (unsigned long **)&hints))
    return;
  
  if (num >= MwmHints::elements) {
    // retrieved the hints
    _mwmhints.flags = hints[0];
    _mwmhints.functions = hints[1];
    _mwmhints.decorations = hints[2];
  }

  delete [] hints;
}


void Client::getArea()
{
  XWindowAttributes wattrib;
  Status ret;
  
  ret = XGetWindowAttributes(**otk::display, _window, &wattrib);
  assert(ret != BadWindow);

  _area.setRect(wattrib.x, wattrib.y, wattrib.width, wattrib.height);
  _border_width = wattrib.border_width;
}


void Client::getState()
{
  _modal = _shaded = _max_horz = _max_vert = _fullscreen = _above = _below =
    _skip_taskbar = _skip_pager = false;
  
  unsigned long *state;
  unsigned long num = (unsigned) -1;
  
  if (otk::Property::get(_window, otk::Property::atoms.net_wm_state,
                         otk::Property::atoms.atom, &num, &state)) {
    for (unsigned long i = 0; i < num; ++i) {
      if (state[i] == otk::Property::atoms.net_wm_state_modal)
        _modal = true;
      else if (state[i] == otk::Property::atoms.net_wm_state_shaded) {
        _shaded = true;
        _wmstate = IconicState;
      } else if (state[i] == otk::Property::atoms.net_wm_state_skip_taskbar)
        _skip_taskbar = true;
      else if (state[i] == otk::Property::atoms.net_wm_state_skip_pager)
        _skip_pager = true;
      else if (state[i] == otk::Property::atoms.net_wm_state_fullscreen)
        _fullscreen = true;
      else if (state[i] == otk::Property::atoms.net_wm_state_maximized_vert)
        _max_vert = true;
      else if (state[i] == otk::Property::atoms.net_wm_state_maximized_horz)
        _max_horz = true;
      else if (state[i] == otk::Property::atoms.net_wm_state_above)
        _above = true;
      else if (state[i] == otk::Property::atoms.net_wm_state_below)
        _below = true;
    }

    delete [] state;
  }
}


void Client::getShaped()
{
  _shaped = false;
#ifdef   SHAPE
  if (otk::display->shape()) {
    int foo;
    unsigned int ufoo;
    int s;

    XShapeSelectInput(**otk::display, _window, ShapeNotifyMask);

    XShapeQueryExtents(**otk::display, _window, &s, &foo,
                       &foo, &ufoo, &ufoo, &foo, &foo, &foo, &ufoo, &ufoo);
    _shaped = (s != 0);
  }
#endif // SHAPE
}


void Client::calcLayer() {
  StackLayer l;

  if (_iconic) l = Layer_Icon;
  else if (_fullscreen) l = Layer_Fullscreen;
  else if (_type == Type_Desktop) l = Layer_Desktop;
  else if (_type == Type_Dock) {
    if (!_below) l = Layer_Top;
    else l = Layer_Normal;
  }
  else if (_above) l = Layer_Above;
  else if (_below) l = Layer_Below;
  else l = Layer_Normal;

  if (l != _layer) {
    _layer = l;
    if (frame) {
      /*
        if we don't have a frame, then we aren't mapped yet (and this would
        SIGSEGV :)
      */
      openbox->screen(_screen)->raiseWindow(this);
    }
  }
}


void Client::updateProtocols()
{
  Atom *proto;
  int num_return = 0;

  _focus_notify = false;
  _decorations &= ~Decor_Close;
  _functions &= ~Func_Close;

  if (XGetWMProtocols(**otk::display, _window, &proto, &num_return)) {
    for (int i = 0; i < num_return; ++i) {
      if (proto[i] == otk::Property::atoms.wm_delete_window) {
        _decorations |= Decor_Close;
        _functions |= Func_Close;
        if (frame)
          frame->adjustSize(); // update the decorations
      } else if (proto[i] == otk::Property::atoms.wm_take_focus)
        // if this protocol is requested, then the window will be notified
        // by the window manager whenever it receives focus
        _focus_notify = true;
    }
    XFree(proto);
  }
}


void Client::updateNormalHints()
{
  XSizeHints size;
  long ret;
  int oldgravity = _gravity;

  // defaults
  _size_inc.setPoint(1, 1);
  _base_size.setPoint(0, 0);
  _min_size.setPoint(0, 0);
  _max_size.setPoint(INT_MAX, INT_MAX);

  // XXX: might want to cancel any interactive resizing of the window at this
  // point..

  // get the hints from the window
  if (XGetWMNormalHints(**otk::display, _window, &size, &ret)) {
    _positioned = (size.flags & (PPosition|USPosition));

    if (size.flags & PWinGravity) {
      _gravity = size.win_gravity;
      
      // if the client has a frame, i.e. has already been mapped and is
      // changing its gravity
      if (_gravity != oldgravity) {
        // move our idea of the client's position based on its new gravity
        int x, y;
        frame->frameGravity(x, y);
        _area.setPos(x, y);
      }
    }

    if (size.flags & PMinSize)
      _min_size.setPoint(size.min_width, size.min_height);
    
    if (size.flags & PMaxSize)
      _max_size.setPoint(size.max_width, size.max_height);
    
    if (size.flags & PBaseSize)
      _base_size.setPoint(size.base_width, size.base_height);
    
    if (size.flags & PResizeInc)
      _size_inc.setPoint(size.width_inc, size.height_inc);
  }
}


void Client::updateWMHints()
{
  XWMHints *hints;

  // assume a window takes input if it doesnt specify
  _can_focus = true;
  _urgent = false;
  
  if ((hints = XGetWMHints(**otk::display, _window)) != NULL) {
    if (hints->flags & InputHint)
      _can_focus = hints->input;

    if (hints->flags & XUrgencyHint)
      _urgent = true;

    if (hints->flags & WindowGroupHint) {
      if (hints->window_group != _group) {
        // XXX: remove from the old group if there was one
        _group = hints->window_group;
        // XXX: do stuff with the group
      }
    } else // no group!
      _group = None;

    XFree(hints);
  }
}


void Client::updateTitle()
{
  _title = "";
  
  // try netwm
  if (!otk::Property::get(_window, otk::Property::atoms.net_wm_name,
                          otk::Property::utf8, &_title)) {
    // try old x stuff
    otk::Property::get(_window, otk::Property::atoms.wm_name,
                       otk::Property::ascii, &_title);
  }

  if (_title.empty())
    _title = _("Unnamed Window");

  if (frame)
    frame->setTitle(_title);
}


void Client::updateIconTitle()
{
  _icon_title = "";
  
  // try netwm
  if (!otk::Property::get(_window, otk::Property::atoms.net_wm_icon_name,
                          otk::Property::utf8, &_icon_title)) {
    // try old x stuff
    otk::Property::get(_window, otk::Property::atoms.wm_icon_name,
                       otk::Property::ascii, &_icon_title);
  }

  if (_title.empty())
    _icon_title = _("Unnamed Window");
}


void Client::updateClass()
{
  // set the defaults
  _app_name = _app_class = _role = "";

  otk::Property::StringVect v;
  unsigned long num = 2;

  if (otk::Property::get(_window, otk::Property::atoms.wm_class,
                         otk::Property::ascii, &num, &v)) {
    if (num > 0) _app_name = v[0].c_str();
    if (num > 1) _app_class = v[1].c_str();
  }

  v.clear();
  num = 1;
  if (otk::Property::get(_window, otk::Property::atoms.wm_window_role,
                         otk::Property::ascii, &num, &v)) {
    if (num > 0) _role = v[0].c_str();
  }
}


void Client::updateStrut()
{
  unsigned long num = 4;
  unsigned long *data;
  if (!otk::Property::get(_window, otk::Property::atoms.net_wm_strut,
                          otk::Property::atoms.cardinal, &num, &data))
    return;

  if (num == 4) {
    _strut.left = data[0];
    _strut.right = data[1];
    _strut.top = data[2];
    _strut.bottom = data[3];
    
    openbox->screen(_screen)->updateStrut();
  }

  delete [] data;
}


void Client::updateTransientFor()
{
  Window t = 0;
  Client *c = 0;

  if (XGetTransientForHint(**otk::display, _window, &t) &&
      t != _window) { // cant be transient to itself!
    c = openbox->findClient(t);
    assert(c != this); // if this happens then we need to check for it

    if (!c /*XXX: && _group*/) {
      // not transient to a client, see if it is transient for a group
      if (//t == _group->leader() ||
        t == None ||
        t == otk::display->screenInfo(_screen)->rootWindow()) {
        // window is a transient for its group!
        // XXX: for now this is treated as non-transient.
        //      this needs to be fixed!
      }
    }
  }

  // if anything has changed...
  if (c != _transient_for) {
    if (_transient_for)
      _transient_for->_transients.remove(this); // remove from old parent
    _transient_for = c;
    if (_transient_for)
      _transient_for->_transients.push_back(this); // add to new parent

    // XXX: change decor status?
  }
}


void Client::propertyHandler(const XPropertyEvent &e)
{
  otk::EventHandler::propertyHandler(e);
  
  // compress changes to a single property into a single change
  XEvent ce;
  while (XCheckTypedEvent(**otk::display, e.type, &ce)) {
    // XXX: it would be nice to compress ALL changes to a property, not just
    //      changes in a row without other props between.
    if (ce.xproperty.atom != e.atom) {
      XPutBackEvent(**otk::display, &ce);
      break;
    }
  }

  if (e.atom == XA_WM_NORMAL_HINTS)
    updateNormalHints();
  else if (e.atom == XA_WM_HINTS)
    updateWMHints();
  else if (e.atom == XA_WM_TRANSIENT_FOR) {
    updateTransientFor();
    getType();
    calcLayer(); // type may have changed, so update the layer
    setupDecorAndFunctions();
    frame->adjustSize(); // this updates the frame for any new decor settings
  }
  else if (e.atom == otk::Property::atoms.net_wm_name ||
           e.atom == otk::Property::atoms.wm_name)
    updateTitle();
  else if (e.atom == otk::Property::atoms.net_wm_icon_name ||
           e.atom == otk::Property::atoms.wm_icon_name)
    updateIconTitle();
  else if (e.atom == otk::Property::atoms.wm_class)
    updateClass();
  else if (e.atom == otk::Property::atoms.wm_protocols)
    updateProtocols();
  else if (e.atom == otk::Property::atoms.net_wm_strut)
    updateStrut();
}


void Client::setWMState(long state)
{
  if (state == _wmstate) return; // no change
  
  _wmstate = state;
  switch (_wmstate) {
  case IconicState:
    // XXX: cause it to iconify
    break;
  case NormalState:
    // XXX: cause it to uniconify
    break;
  }
}


void Client::setDesktop(long target)
{
  if (target == _desktop) return;
  
  printf("Setting desktop %ld\n", target);

  if (!(target >= 0 || target == (signed)0xffffffff)) return;
  
  _desktop = target;

  otk::Property::set(_window, otk::Property::atoms.net_wm_desktop,
                     otk::Property::atoms.cardinal, (unsigned)_desktop);
  
  // 'move' the window to the new desktop
  if (_desktop == openbox->screen(_screen)->desktop() ||
      _desktop == (signed)0xffffffff)
    frame->show();
  else
    frame->hide();
}


void Client::setState(StateAction action, long data1, long data2)
{
  bool shadestate = _shaded;

  if (!(action == State_Add || action == State_Remove ||
        action == State_Toggle))
    return; // an invalid action was passed to the client message, ignore it

  for (int i = 0; i < 2; ++i) {
    Atom state = i == 0 ? data1 : data2;
    
    if (! state) continue;

    // if toggling, then pick whether we're adding or removing
    if (action == State_Toggle) {
      if (state == otk::Property::atoms.net_wm_state_modal)
        action = _modal ? State_Remove : State_Add;
      else if (state == otk::Property::atoms.net_wm_state_maximized_vert)
        action = _max_vert ? State_Remove : State_Add;
      else if (state == otk::Property::atoms.net_wm_state_maximized_horz)
        action = _max_horz ? State_Remove : State_Add;
      else if (state == otk::Property::atoms.net_wm_state_shaded)
        action = _shaded ? State_Remove : State_Add;
      else if (state == otk::Property::atoms.net_wm_state_skip_taskbar)
        action = _skip_taskbar ? State_Remove : State_Add;
      else if (state == otk::Property::atoms.net_wm_state_skip_pager)
        action = _skip_pager ? State_Remove : State_Add;
      else if (state == otk::Property::atoms.net_wm_state_fullscreen)
        action = _fullscreen ? State_Remove : State_Add;
      else if (state == otk::Property::atoms.net_wm_state_above)
        action = _above ? State_Remove : State_Add;
      else if (state == otk::Property::atoms.net_wm_state_below)
        action = _below ? State_Remove : State_Add;
    }
    
    if (action == State_Add) {
      if (state == otk::Property::atoms.net_wm_state_modal) {
        if (_modal) continue;
        _modal = true;
        // XXX: give it focus if another window has focus that shouldnt now
      } else if (state == otk::Property::atoms.net_wm_state_maximized_vert) {
        if (_max_vert) continue;
        _max_vert = true;
        // XXX: resize the window etc
      } else if (state == otk::Property::atoms.net_wm_state_maximized_horz) {
        if (_max_horz) continue;
        _max_horz = true;
        // XXX: resize the window etc
      } else if (state == otk::Property::atoms.net_wm_state_shaded) {
        if (_shaded) continue;
        // shade when we're all thru here
        shadestate = true;
      } else if (state == otk::Property::atoms.net_wm_state_skip_taskbar) {
        _skip_taskbar = true;
      } else if (state == otk::Property::atoms.net_wm_state_skip_pager) {
        _skip_pager = true;
      } else if (state == otk::Property::atoms.net_wm_state_fullscreen) {
        if (_fullscreen) continue;
        _fullscreen = true;
      } else if (state == otk::Property::atoms.net_wm_state_above) {
        if (_above) continue;
        _above = true;
      } else if (state == otk::Property::atoms.net_wm_state_below) {
        if (_below) continue;
        _below = true;
      }

    } else { // action == State_Remove
      if (state == otk::Property::atoms.net_wm_state_modal) {
        if (!_modal) continue;
        _modal = false;
      } else if (state == otk::Property::atoms.net_wm_state_maximized_vert) {
        if (!_max_vert) continue;
        _max_vert = false;
        // XXX: resize the window etc
      } else if (state == otk::Property::atoms.net_wm_state_maximized_horz) {
        if (!_max_horz) continue;
        _max_horz = false;
        // XXX: resize the window etc
      } else if (state == otk::Property::atoms.net_wm_state_shaded) {
        if (!_shaded) continue;
        // unshade when we're all thru here
        shadestate = false;
      } else if (state == otk::Property::atoms.net_wm_state_skip_taskbar) {
        _skip_taskbar = false;
      } else if (state == otk::Property::atoms.net_wm_state_skip_pager) {
        _skip_pager = false;
      } else if (state == otk::Property::atoms.net_wm_state_fullscreen) {
        if (!_fullscreen) continue;
        _fullscreen = false;
      } else if (state == otk::Property::atoms.net_wm_state_above) {
        if (!_above) continue;
        _above = false;
      } else if (state == otk::Property::atoms.net_wm_state_below) {
        if (!_below) continue;
        _below = false;
      }
    }
  }
  if (shadestate != _shaded)
    shade(shadestate);
  calcLayer();
}


void Client::toggleClientBorder(bool addborder)
{
  // adjust our idea of where the client is, based on its border. When the
  // border is removed, the client should now be considered to be in a
  // different position.
  // when re-adding the border to the client, the same operation needs to be
  // reversed.
  int x = _area.x(), y = _area.y();
  switch(_gravity) {
  default:
  case NorthWestGravity:
  case WestGravity:
  case SouthWestGravity:
  case NorthGravity:
  case CenterGravity:
  case SouthGravity:
    break;
  case NorthEastGravity:
  case EastGravity:
  case SouthEastGravity:
    if (addborder) x -= _border_width * 2;
    else           x += _border_width * 2;
    break;
  case ForgetGravity:
  case StaticGravity:
    if (addborder) x -= _border_width;
    else           x += _border_width;
    break;
  }
  switch(_gravity) {
  default:
  case NorthWestGravity:
  case WestGravity:
  case NorthGravity:
  case CenterGravity:
  case NorthEastGravity:
  case EastGravity:
    break;
  case SouthWestGravity:
  case SouthGravity:
  case SouthEastGravity:
    if (addborder) y -= _border_width * 2;
    else           y += _border_width * 2;
    break;
  case ForgetGravity:
  case StaticGravity:
    if (addborder) y -= _border_width;
    else           y += _border_width;
    break;
  }
  _area.setPos(x, y);

  if (addborder) {
    XSetWindowBorderWidth(**otk::display, _window, _border_width);

    // move the client so it is back it the right spot _with_ its border!
    XMoveWindow(**otk::display, _window, x, y);
  } else
    XSetWindowBorderWidth(**otk::display, _window, 0);
}


void Client::clientMessageHandler(const XClientMessageEvent &e)
{
  otk::EventHandler::clientMessageHandler(e);
  
  if (e.format != 32) return;

  if (e.message_type == otk::Property::atoms.wm_change_state) {
    // compress changes into a single change
    bool compress = false;
    XEvent ce;
    while (XCheckTypedEvent(**otk::display, e.type, &ce)) {
      // XXX: it would be nice to compress ALL messages of a type, not just
      //      messages in a row without other message types between.
      if (ce.xclient.message_type != e.message_type) {
        XPutBackEvent(**otk::display, &ce);
        break;
      }
      compress = true;
    }
    if (compress)
      setWMState(ce.xclient.data.l[0]); // use the found event
    else
      setWMState(e.data.l[0]); // use the original event
  } else if (e.message_type == otk::Property::atoms.net_wm_desktop) {
    // compress changes into a single change 
    bool compress = false;
    XEvent ce;
    while (XCheckTypedEvent(**otk::display, e.type, &ce)) {
      // XXX: it would be nice to compress ALL messages of a type, not just
      //      messages in a row without other message types between.
      if (ce.xclient.message_type != e.message_type) {
        XPutBackEvent(**otk::display, &ce);
        break;
      }
      compress = true;
    }
    if (compress)
      setDesktop(e.data.l[0]); // use the found event
    else
      setDesktop(e.data.l[0]); // use the original event
  } else if (e.message_type == otk::Property::atoms.net_wm_state) {
    // can't compress these
#ifdef DEBUG
    printf("net_wm_state %s %ld %ld for 0x%lx\n",
           (e.data.l[0] == 0 ? "Remove" : e.data.l[0] == 1 ? "Add" :
            e.data.l[0] == 2 ? "Toggle" : "INVALID"),
           e.data.l[1], e.data.l[2], _window);
#endif
    setState((StateAction)e.data.l[0], e.data.l[1], e.data.l[2]);
  } else if (e.message_type == otk::Property::atoms.net_close_window) {
#ifdef DEBUG
    printf("net_close_window for 0x%lx\n", _window);
#endif
    close();
  } else if (e.message_type == otk::Property::atoms.net_active_window) {
#ifdef DEBUG
    printf("net_active_window for 0x%lx\n", _window);
#endif
    if (_shaded)
      shade(false);
    // XXX: deiconify
    focus();
    openbox->screen(_screen)->raiseWindow(this);
  }
}


#if defined(SHAPE)
void Client::shapeHandler(const XShapeEvent &e)
{
  otk::EventHandler::shapeHandler(e);

  if (e.kind == ShapeBounding) {
    _shaped = e.shaped;
    frame->adjustShape();
  }
}
#endif


void Client::resize(Corner anchor, int w, int h, int x, int y)
{
  w -= _base_size.x(); 
  h -= _base_size.y();

  // for interactive resizing. have to move half an increment in each
  // direction.
  w += _size_inc.x() / 2;
  h += _size_inc.y() / 2;

  // is the window resizable? if it is not, then don't check its sizes, the
  // client can do what it wants and the user can't change it anyhow
  if (_min_size.x() <= _max_size.x() && _min_size.y() <= _max_size.y()) {
    // smaller than min size or bigger than max size?
    if (w < _min_size.x()) w = _min_size.x();
    else if (w > _max_size.x()) w = _max_size.x();
    if (h < _min_size.y()) h = _min_size.y();
    else if (h > _max_size.y()) h = _max_size.y();
  }

  // keep to the increments
  w /= _size_inc.x();
  h /= _size_inc.y();

  // you cannot resize to nothing
  if (w < 1) w = 1;
  if (h < 1) h = 1;
  
  // store the logical size
  _logical_size.setPoint(w, h);

  w *= _size_inc.x();
  h *= _size_inc.y();

  w += _base_size.x();
  h += _base_size.y();

  if (x == INT_MIN || y == INT_MIN) {
    x = _area.x();
    y = _area.y();
    switch (anchor) {
    case TopLeft:
      break;
    case TopRight:
      x -= w - _area.width();
      break;
    case BottomLeft:
      y -= h - _area.height();
      break;
    case BottomRight:
      x -= w - _area.width();
      y -= h - _area.height();
      break;
    }
  }

  _area.setSize(w, h);

  XResizeWindow(**otk::display, _window, w, h);

  // resize the frame to match the request
  frame->adjustSize();
  move(x, y);
}


void Client::move(int x, int y)
{
  _area.setPos(x, y);

  // move the frame to be in the requested position
  if (frame) { // this can be called while mapping, before frame exists
    frame->adjustPosition();

    // send synthetic configure notify (we don't need to if we aren't mapped
    // yet)
    XEvent event;
    event.type = ConfigureNotify;
    event.xconfigure.display = **otk::display;
    event.xconfigure.event = _window;
    event.xconfigure.window = _window;
    event.xconfigure.x = x;
    event.xconfigure.y = y;
    event.xconfigure.width = _area.width();
    event.xconfigure.height = _area.height();
    event.xconfigure.border_width = _border_width;
    event.xconfigure.above = frame->window();
    event.xconfigure.override_redirect = False;
    XSendEvent(event.xconfigure.display, event.xconfigure.window, False,
               StructureNotifyMask, &event);
  }
}


void Client::close()
{
  XEvent ce;

  if (!(_functions & Func_Close)) return;

  // XXX: itd be cool to do timeouts and shit here for killing the client's
  //      process off
  // like... if the window is around after 5 seconds, then the close button
  // turns a nice red, and if this function is called again, the client is
  // explicitly killed.

  ce.xclient.type = ClientMessage;
  ce.xclient.message_type =  otk::Property::atoms.wm_protocols;
  ce.xclient.display = **otk::display;
  ce.xclient.window = _window;
  ce.xclient.format = 32;
  ce.xclient.data.l[0] = otk::Property::atoms.wm_delete_window;
  ce.xclient.data.l[1] = CurrentTime;
  ce.xclient.data.l[2] = 0l;
  ce.xclient.data.l[3] = 0l;
  ce.xclient.data.l[4] = 0l;
  XSendEvent(**otk::display, _window, false, NoEventMask, &ce);
}


void Client::changeState()
{
  unsigned long state[2];
  state[0] = _wmstate;
  state[1] = None;
  otk::Property::set(_window, otk::Property::atoms.wm_state,
                     otk::Property::atoms.wm_state, state, 2);
  
  Atom netstate[10];
  int num = 0;
  if (_modal)
    netstate[num++] = otk::Property::atoms.net_wm_state_modal;
  if (_shaded)
    netstate[num++] = otk::Property::atoms.net_wm_state_shaded;
  if (_iconic)
    netstate[num++] = otk::Property::atoms.net_wm_state_hidden;
  if (_skip_taskbar)
    netstate[num++] = otk::Property::atoms.net_wm_state_skip_taskbar;
  if (_skip_pager)
    netstate[num++] = otk::Property::atoms.net_wm_state_skip_pager;
  if (_fullscreen)
    netstate[num++] = otk::Property::atoms.net_wm_state_fullscreen;
  if (_max_vert)
    netstate[num++] = otk::Property::atoms.net_wm_state_maximized_vert;
  if (_max_horz)
    netstate[num++] = otk::Property::atoms.net_wm_state_maximized_horz;
  if (_above)
    netstate[num++] = otk::Property::atoms.net_wm_state_above;
  if (_below)
    netstate[num++] = otk::Property::atoms.net_wm_state_below;
  otk::Property::set(_window, otk::Property::atoms.net_wm_state,
                     otk::Property::atoms.atom, netstate, num);

  calcLayer();
}


void Client::shade(bool shade)
{
  if (shade == _shaded) return; // already done

  _wmstate = shade ? IconicState : NormalState;
  _shaded = shade;
  changeState();
  frame->adjustSize();
}


bool Client::focus() const
{
  // won't try focus if the client doesn't want it, or if the window isn't
  // visible on the screen
  if (!(frame->isVisible() && (_can_focus || _focus_notify))) return false;

  if (_focused) return true;

  if (_can_focus)
    XSetInputFocus(**otk::display, _window,
                   RevertToNone, CurrentTime);

  if (_focus_notify) {
    XEvent ce;
    ce.xclient.type = ClientMessage;
    ce.xclient.message_type = otk::Property::atoms.wm_protocols;
    ce.xclient.display = **otk::display;
    ce.xclient.window = _window;
    ce.xclient.format = 32;
    ce.xclient.data.l[0] = otk::Property::atoms.wm_take_focus;
    ce.xclient.data.l[1] = openbox->lastTime();
    ce.xclient.data.l[2] = 0l;
    ce.xclient.data.l[3] = 0l;
    ce.xclient.data.l[4] = 0l;
    XSendEvent(**otk::display, _window, False, NoEventMask, &ce);
  }

  return true;
}


void Client::unfocus() const
{
  if (!_focused) return;

  assert(openbox->focusedClient() == this);
  openbox->setFocusedClient(0);
}


void Client::focusHandler(const XFocusChangeEvent &e)
{
#ifdef    DEBUG
//  printf("FocusIn for 0x%lx\n", e.window);
#endif // DEBUG
  
  otk::EventHandler::focusHandler(e);

  frame->focus();
  _focused = true;

  openbox->setFocusedClient(this);
}


void Client::unfocusHandler(const XFocusChangeEvent &e)
{
#ifdef    DEBUG
//  printf("FocusOut for 0x%lx\n", e.window);
#endif // DEBUG
  
  otk::EventHandler::unfocusHandler(e);

  frame->unfocus();
  _focused = false;

  if (openbox->focusedClient() == this)
    openbox->setFocusedClient(0);
}


void Client::configureRequestHandler(const XConfigureRequestEvent &e)
{
#ifdef    DEBUG
  printf("ConfigureRequest for 0x%lx\n", e.window);
#endif // DEBUG
  
  otk::EventHandler::configureRequestHandler(e);

  // XXX: if we are iconic (or shaded? (fvwm does that)) ignore the event

  if (e.value_mask & CWBorderWidth)
    _border_width = e.border_width;

  // resize, then move, as specified in the EWMH section 7.7
  if (e.value_mask & (CWWidth | CWHeight)) {
    int w = (e.value_mask & CWWidth) ? e.width : _area.width();
    int h = (e.value_mask & CWHeight) ? e.height : _area.height();

    Corner corner;
    switch (_gravity) {
    case NorthEastGravity:
    case EastGravity:
      corner = TopRight;
      break;
    case SouthWestGravity:
    case SouthGravity:
      corner = BottomLeft;
      break;
    case SouthEastGravity:
      corner = BottomRight;
      break;
    default:     // NorthWest, Static, etc
      corner = TopLeft;
    }

    // if moving AND resizing ...
    if (e.value_mask & (CWX | CWY)) {
      int x = (e.value_mask & CWX) ? e.x : _area.x();
      int y = (e.value_mask & CWY) ? e.y : _area.y();
      resize(corner, w, h, x, y);
    } else // if JUST resizing...
      resize(corner, w, h);
  } else if (e.value_mask & (CWX | CWY)) { // if JUST moving...
    int x = (e.value_mask & CWX) ? e.x : _area.x();
    int y = (e.value_mask & CWY) ? e.y : _area.y();
    move(x, y);
  }

  if (e.value_mask & CWStackMode) {
    switch (e.detail) {
    case Below:
    case BottomIf:
      openbox->screen(_screen)->lowerWindow(this);
      break;

    case Above:
    case TopIf:
    default:
      openbox->screen(_screen)->raiseWindow(this);
      break;
    }
  }
}


void Client::unmapHandler(const XUnmapEvent &e)
{
  if (ignore_unmaps) {
#ifdef    DEBUG
    printf("Ignored UnmapNotify for 0x%lx (event 0x%lx)\n", e.window, e.event);
#endif // DEBUG
    ignore_unmaps--;
    return;
  }
  
#ifdef    DEBUG
  printf("UnmapNotify for 0x%lx\n", e.window);
#endif // DEBUG

  otk::EventHandler::unmapHandler(e);

  // this deletes us etc
  openbox->screen(_screen)->unmanageWindow(this);
}


void Client::destroyHandler(const XDestroyWindowEvent &e)
{
#ifdef    DEBUG
  printf("DestroyNotify for 0x%lx\n", e.window);
#endif // DEBUG

  otk::EventHandler::destroyHandler(e);

  // this deletes us etc
  openbox->screen(_screen)->unmanageWindow(this);
}


void Client::reparentHandler(const XReparentEvent &e)
{
  // this is when the client is first taken captive in the frame
  if (e.parent == frame->plate()) return;

#ifdef    DEBUG
  printf("ReparentNotify for 0x%lx\n", e.window);
#endif // DEBUG

  otk::EventHandler::reparentHandler(e);

  /*
    This event is quite rare and is usually handled in unmapHandler.
    However, if the window is unmapped when the reparent event occurs,
    the window manager never sees it because an unmap event is not sent
    to an already unmapped window.
  */

  // we don't want the reparent event, put it back on the stack for the X
  // server to deal with after we unmanage the window
  XEvent ev;
  ev.xreparent = e;
  XPutBackEvent(**otk::display, &ev);
  
  // this deletes us etc
  openbox->screen(_screen)->unmanageWindow(this);
}

}
