// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "display.hh"
#include "screeninfo.hh"
#include "gccache.hh"
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

#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

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

#include "gettext.h"
#define _(str) gettext(str)
}

namespace otk {


::Display *Display::display = (::Display*) 0;
bool Display::_xkb = false;
int  Display::_xkb_event_basep = 0;
bool Display::_shape = false;
int  Display::_shape_event_basep = 0;
bool Display::_xinerama = false;
int  Display::_xinerama_event_basep = 0;
unsigned int Display::_mask_list[8];
unsigned int Display::_scrollLockMask = 0;
unsigned int Display::_numLockMask = 0;
Display::ScreenInfoList Display::_screenInfoList;
GCCache *Display::_gccache = (GCCache*) 0;
int Display::_grab_count = 0;


static int xerrorHandler(::Display *d, XErrorEvent *e)
{
#ifdef DEBUG
  char errtxt[128];

  //if (e->error_code != BadWindow)
  {
    XGetErrorText(d, e->error_code, errtxt, 128);
    printf("X Error: %s\n", errtxt);
    if (e->error_code != BadWindow)
      abort();
  }
#else
  (void)d;
  (void)e;
#endif

  return false;
}


void Display::initialize(char *name)
{
  int junk;
  (void)junk;

  // Open the X display
  if (!(display = XOpenDisplay(name))) {
    printf(_("Unable to open connection to the X server. Please set the \n\
DISPLAY environment variable approriately, or use the '-display' command \n\
line argument.\n\n"));
    ::exit(1);
  }
  if (fcntl(ConnectionNumber(display), F_SETFD, 1) == -1) {
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
  // XXX rm -> std::string dtmp = "DISPLAY=" + DisplayString(display);
  putenv(std::string("DISPLAY=") + DisplayString(display));
  
  // find the availability of X extensions we like to use
#ifdef XKB
  _xkb = XkbQueryExtension(display, &junk, &_xkb_event_basep, &junk, NULL, 
                           NULL);
#endif

#ifdef SHAPE
  _shape = XShapeQueryExtension(display, &_shape_event_basep, &junk);
#endif

#ifdef XINERAMA
  _xinerama = XineramaQueryExtension(display, &_xinerama_event_basep, &junk);
#endif // XINERAMA

  // get lock masks that are defined by the display (not constant)
  XModifierKeymap *modmap;

  modmap = XGetModifierMapping(display);
  if (modmap && modmap->max_keypermod > 0) {
    const int mask_table[] = {
      ShiftMask, LockMask, ControlMask, Mod1Mask,
      Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
    };
    const size_t size = (sizeof(mask_table) / sizeof(mask_table[0])) *
      modmap->max_keypermod;
    // get the values of the keyboard lock modifiers
    // Note: Caps lock is not retrieved the same way as Scroll and Num lock
    // since it doesn't need to be.
    const KeyCode num_lock = XKeysymToKeycode(display, XK_Num_Lock);
    const KeyCode scroll_lock = XKeysymToKeycode(display, XK_Scroll_Lock);

    for (size_t cnt = 0; cnt < size; ++cnt) {
      if (! modmap->modifiermap[cnt]) continue;

      if (num_lock == modmap->modifiermap[cnt])
        _numLockMask = mask_table[cnt / modmap->max_keypermod];
      if (scroll_lock == modmap->modifiermap[cnt])
        _scrollLockMask = mask_table[cnt / modmap->max_keypermod];
    }
  }

  if (modmap) XFreeModifiermap(modmap);

  _mask_list[0] = 0;
  _mask_list[1] = LockMask;
  _mask_list[2] = _numLockMask;
  _mask_list[3] = LockMask | _numLockMask;
  _mask_list[4] = _scrollLockMask;
  _mask_list[5] = _scrollLockMask | LockMask;
  _mask_list[6] = _scrollLockMask | _numLockMask;
  _mask_list[7] = _scrollLockMask | LockMask | _numLockMask;

  // Get information on all the screens which are available.
  _screenInfoList.reserve(ScreenCount(display));
  for (int i = 0; i < ScreenCount(display); ++i)
    _screenInfoList.push_back(ScreenInfo(i));

  _gccache = new GCCache(_screenInfoList.size());
}


void Display::destroy()
{
  delete _gccache;
  while (_grab_count > 0)
    ungrab();
  XCloseDisplay(display);
}


const ScreenInfo* Display::screenInfo(int snum) {
  assert(snum >= 0);
  assert(snum < static_cast<int>(_screenInfoList.size()));
  return &_screenInfoList[snum];
}


const ScreenInfo* Display::findScreen(Window root)
{
  ScreenInfoList::iterator it, end = _screenInfoList.end();
  for (it = _screenInfoList.begin(); it != end; ++it)
    if (it->rootWindow() == root)
      return &(*it);
  return 0;
}


void Display::grab()
{
  if (_grab_count == 0)
    XGrabServer(display);
  _grab_count++;
}


void Display::ungrab()
{
  if (_grab_count == 0) return;
  _grab_count--;
  if (_grab_count == 0)
    XUngrabServer(display);
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
                         Cursor cursor, bool allow_scroll_lock) {
  unsigned int length = (allow_scroll_lock) ? 8 / 2:
                                              8;
  for (size_t cnt = 0; cnt < length; ++cnt)
    XGrabButton(Display::display, button, modifiers | _mask_list[cnt],
                grab_window, owner_events, event_mask, pointer_mode,
                keyboard_mode, confine_to, cursor);
}


/*
 * Releases the grab on a button, and ungrabs all possible combinations of the
 * keyboard lock keys.
 */
void Display::ungrabButton(unsigned int button, unsigned int modifiers,
                           Window grab_window) {
  for (size_t cnt = 0; cnt < 8; ++cnt)
    XUngrabButton(Display::display, button, modifiers | _mask_list[cnt],
                  grab_window);
}

void Display::grabKey(unsigned int keycode, unsigned int modifiers,
                        Window grab_window, bool owner_events,
                        int pointer_mode, int keyboard_mode,
                        bool allow_scroll_lock)
{
  unsigned int length = (allow_scroll_lock) ? 8 / 2:
                                              8;
  for (size_t cnt = 0; cnt < length; ++cnt)
    XGrabKey(Display::display, keycode, modifiers | _mask_list[cnt],
                grab_window, owner_events, pointer_mode, keyboard_mode);
}

void Display::ungrabKey(unsigned int keycode, unsigned int modifiers,
                          Window grab_window)
{
  for (size_t cnt = 0; cnt < 8; ++cnt)
    XUngrabKey(Display::display, keycode, modifiers | _mask_list[cnt],
               grab_window);
}

}
