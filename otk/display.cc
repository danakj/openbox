// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "display.hh"
#include "screeninfo.hh"
#include "rendercontrol.hh"
#include "util.hh"

extern "C" {
#include <X11/keysym.h>

#ifdef    XKB
#include <X11/XKBlib.h>
#endif // XKB

#ifdef    SHAPE
#include <X11/extensions/shape.h>
#endif // SHAPE

#ifdef    XINERAMA
#include <X11/extensions/Xinerama.h>
#endif // XINERAMA

#ifdef    HAVE_SIGNAL_H
#  include <signal.h>
#endif // HAVE_SIGNAL_H

#ifdef    HAVE_FCNTL_H
#  include <fcntl.h>
#endif // HAVE_FCNTL_H

#ifdef    HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif // HAVE_UNISTD_H

#include "../src/gettext.h"
#define _(str) gettext(str)
}

#include <cstdio>

namespace otk {


Display *display = (Display*) 0;

static int xerrorHandler(::Display *d, XErrorEvent *e)
{
  if (!display->ignoreErrors()) {
#ifdef DEBUG
    char errtxt[128];

    //if (e->error_code != BadWindow)
    {
      XGetErrorText(d, e->error_code, errtxt, 127);
      printf("X Error: %s\n", errtxt);
      if (e->error_code != BadWindow)
        abort();
    }
#else
    (void)d;
    (void)e;
#endif
  }
  return false;
}


Display::Display()
  : _display(0),
    _xkb(false),
    _xkb_event_basep(0),
    _shape(false),
    _shape_event_basep(0),
    _xinerama(false),
    _xinerama_event_basep(0),
    _mask_list(),
    _num_lock_mask(0),
    _scroll_lock_mask(0),
    _grab_count(0),
    _screeninfo_list(0),
    _rendercontrol_list(0)
{
  int junk;
  (void)junk;

  display = this;
  
  // Open the X display
  if (!(_display = XOpenDisplay(NULL))) {
    printf(_("Unable to open connection to the X server. Please set the \n\
DISPLAY environment variable approriately.\n\n"));
    ::exit(1);
  }
  if (fcntl(ConnectionNumber(_display), F_SETFD, 1) == -1) {
    printf(_("Couldn't mark display connection as close-on-exec.\n\n"));
    ::exit(1);
  }
  if (!XSupportsLocale())
    printf(_("X server does not support locale.\n"));
  if (!XSetLocaleModifiers(""))
    printf(_("Cannot set locale modifiers for the X server.\n"));
  
  // set our error handler for X errors
  XSetErrorHandler(xerrorHandler);

  // set the DISPLAY environment variable for any lauched children, to the
  // display we're using, so they open in the right place.
  putenv(std::string("DISPLAY=") + DisplayString(_display));
  
  // find the availability of X extensions we like to use
#ifdef XKB
  _xkb = XkbQueryExtension(_display, &junk, &_xkb_event_basep, &junk, NULL, 
                           NULL);
#endif

#ifdef SHAPE
  _shape = XShapeQueryExtension(_display, &_shape_event_basep, &junk);
#endif

#ifdef XINERAMA
  _xinerama = XineramaQueryExtension(_display, &_xinerama_event_basep, &junk);
#endif // XINERAMA

  // get lock masks that are defined by the display (not constant)
  _modmap = XGetModifierMapping(_display);
  assert(_modmap);
  if (_modmap && _modmap->max_keypermod > 0) {
    const int mask_table[] = {
      ShiftMask, LockMask, ControlMask, Mod1Mask,
      Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
    };
    const size_t size = (sizeof(mask_table) / sizeof(mask_table[0])) *
      _modmap->max_keypermod;
    // get the values of the keyboard lock modifiers
    // Note: Caps lock is not retrieved the same way as Scroll and Num lock
    // since it doesn't need to be.
    const KeyCode num_lock = XKeysymToKeycode(_display, XK_Num_Lock);
    const KeyCode scroll_lock = XKeysymToKeycode(_display, XK_Scroll_Lock);

    for (size_t cnt = 0; cnt < size; ++cnt) {
      if (! _modmap->modifiermap[cnt]) continue;

      if (num_lock == _modmap->modifiermap[cnt])
        _num_lock_mask = mask_table[cnt / _modmap->max_keypermod];
      if (scroll_lock == _modmap->modifiermap[cnt])
        _scroll_lock_mask = mask_table[cnt / _modmap->max_keypermod];
    }
  }

  _mask_list[0] = 0;
  _mask_list[1] = LockMask;
  _mask_list[2] = _num_lock_mask;
  _mask_list[3] = LockMask | _num_lock_mask;
  _mask_list[4] = _scroll_lock_mask;
  _mask_list[5] = _scroll_lock_mask | LockMask;
  _mask_list[6] = _scroll_lock_mask | _num_lock_mask;
  _mask_list[7] = _scroll_lock_mask | LockMask | _num_lock_mask;

  // Get information on all the screens which are available, and create their
  // RenderControl
  _screeninfo_list = new ScreenInfo*[ScreenCount(_display)];
  _rendercontrol_list = new RenderControl*[ScreenCount(_display)];
  for (int i = 0; i < ScreenCount(_display); ++i) {
    _screeninfo_list[i] = new ScreenInfo(i);
    _rendercontrol_list[i] = RenderControl::getRenderControl(i);
  }
}


Display::~Display()
{
  while (_grab_count > 0)
    ungrab();

  XFreeModifiermap(_modmap);
  
  for (int i = 0; i < ScreenCount(_display); ++i) {
    delete _rendercontrol_list[i];
    delete _screeninfo_list[i];
  }
  delete [] _rendercontrol_list;
  delete [] _screeninfo_list;
  
  XCloseDisplay(_display);
}


const ScreenInfo* Display::screenInfo(int snum) const
{
  assert(snum >= 0);
  assert(snum < (signed) ScreenCount(_display));
  return _screeninfo_list[snum];
}


const ScreenInfo* Display::findScreen(Window root) const
{
  for (int i = 0; i < ScreenCount(_display); ++i)
    if (_screeninfo_list[i]->rootWindow() == root)
      return _screeninfo_list[i];
  return 0;
}


const RenderControl *Display::renderControl(int snum) const
{
  assert(snum >= 0);
  assert(snum < (signed) ScreenCount(_display));
  return _rendercontrol_list[snum];
}


void Display::setIgnoreErrors(bool t)
{
  _ignore_errors = t;
  // sync up so that anything already sent is/isn't ignored!
  XSync(_display, false);
}

void Display::grab()
{
  if (_grab_count == 0) {
    XGrabServer(_display);
    XSync(_display, false); // make sure it kicks in
  }
  _grab_count++;
}


void Display::ungrab()
{
  if (_grab_count == 0) return;
  _grab_count--;
  if (_grab_count == 0) {
    XUngrabServer(_display);
    XFlush(_display); // ungrab as soon as possible
  }
}







/*
 * Grabs a button, but also grabs the button in every possible combination
 * with the keyboard lock keys, so that they do not cancel out the event.

 * if allow_scroll_lock is true then only the top half of the lock mask
 * table is used and scroll lock is ignored.  This value defaults to false.
 */
void Display::grabButton(unsigned int button, unsigned int modifiers,
                         Window grab_window, bool owner_events,
                         unsigned int event_mask, int pointer_mode,
                         int keyboard_mode, Window confine_to,
                         Cursor cursor, bool allow_scroll_lock) const
{
  unsigned int length = (allow_scroll_lock) ? 8 / 2:
                                              8;
  for (size_t cnt = 0; cnt < length; ++cnt)
    XGrabButton(_display, button, modifiers | _mask_list[cnt],
                grab_window, owner_events, event_mask, pointer_mode,
                keyboard_mode, confine_to, cursor);
}


/*
 * Releases the grab on a button, and ungrabs all possible combinations of the
 * keyboard lock keys.
 */
void Display::ungrabButton(unsigned int button, unsigned int modifiers,
                           Window grab_window) const
{
  for (size_t cnt = 0; cnt < 8; ++cnt)
    XUngrabButton(_display, button, modifiers | _mask_list[cnt],
                  grab_window);
}

void Display::grabKey(unsigned int keycode, unsigned int modifiers,
                        Window grab_window, bool owner_events,
                        int pointer_mode, int keyboard_mode,
                        bool allow_scroll_lock) const
{
  unsigned int length = (allow_scroll_lock) ? 8 / 2:
                                              8;
  for (size_t cnt = 0; cnt < length; ++cnt)
    XGrabKey(_display, keycode, modifiers | _mask_list[cnt],
                grab_window, owner_events, pointer_mode, keyboard_mode);
}

void Display::ungrabKey(unsigned int keycode, unsigned int modifiers,
                          Window grab_window) const
{
  for (size_t cnt = 0; cnt < 8; ++cnt)
    XUngrabKey(_display, keycode, modifiers | _mask_list[cnt],
               grab_window);
}

void Display::ungrabAllKeys(Window grab_window) const
{
  XUngrabKey(_display, AnyKey, AnyModifier, grab_window);
}

}
