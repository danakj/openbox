// -*- mode: C++; indent-tabs-mode: nil; -*-
// Toolbar.cc for Blackbox - an X11 Window manager
// Copyright (c) 2001 - 2002 Sean 'Shaleh' Perry <shaleh@debian.org>
// Copyright (c) 1997 - 2000 Brad Hughes (bhughes@tcac.net)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#include <X11/keysym.h>

#ifdef HAVE_STRING_H
#  include <string.h>
#endif // HAVE_STRING_H

#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef    TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else // !TIME_WITH_SYS_TIME
# ifdef    HAVE_SYS_TIME_H
#  include <sys/time.h>
# else // !HAVE_SYS_TIME_H
#  include <time.h>
# endif // HAVE_SYS_TIME_H
#endif // TIME_WITH_SYS_TIME
}

#include <string>
using std::string;

#include "i18n.hh"
#include "blackbox.hh"
#include "Font.hh"
#include "GCCache.hh"
#include "Image.hh"
#include "Screen.hh"
#include "Toolbar.hh"
#include "Window.hh"
#include "Workspace.hh"
#include "Clientmenu.hh"
#include "Workspacemenu.hh"
#include "Slit.hh"


static long aMinuteFromNow(void) {
  timeval now;
  gettimeofday(&now, 0);
  return ((60 - (now.tv_sec % 60)) * 1000);
}


Toolbar::Toolbar(BScreen *scrn) {
  screen = scrn;
  blackbox = screen->getBlackbox();
  toolbarstr = "session.screen" + itostring(screen->getScreenNumber()) +
    ".toolbar.";
  config = blackbox->getConfig();

  load_rc();

  // get the clock updating every minute
  clock_timer = new BTimer(blackbox, this);
  clock_timer->setTimeout(aMinuteFromNow());
  clock_timer->recurring(True);
  clock_timer->start();
  frame.minute = frame.hour = -1;

  hide_handler.toolbar = this;
  hide_timer = new BTimer(blackbox, &hide_handler);
  hide_timer->setTimeout(blackbox->getAutoRaiseDelay());

  editing = False;
  new_name_pos = 0;

  toolbarmenu = new Toolbarmenu(this);

  display = blackbox->getXDisplay();
  XSetWindowAttributes attrib;
  unsigned long create_mask = CWBackPixmap | CWBackPixel | CWBorderPixel |
                              CWColormap | CWOverrideRedirect | CWEventMask;
  attrib.background_pixmap = None;
  attrib.background_pixel = attrib.border_pixel =
    screen->getBorderColor()->pixel();
  attrib.colormap = screen->getColormap();
  attrib.override_redirect = True;
  attrib.event_mask = ButtonPressMask | ButtonReleaseMask |
                      EnterWindowMask | LeaveWindowMask;

  frame.window =
    XCreateWindow(display, screen->getRootWindow(), 0, 0, 1, 1, 0,
                  screen->getDepth(), InputOutput, screen->getVisual(),
                  create_mask, &attrib);
  blackbox->saveToolbarSearch(frame.window, this);

  attrib.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask |
                      KeyPressMask | EnterWindowMask;

  frame.workspace_label =
    XCreateWindow(display, frame.window, 0, 0, 1, 1, 0, screen->getDepth(),
                  InputOutput, screen->getVisual(), create_mask, &attrib);
  blackbox->saveToolbarSearch(frame.workspace_label, this);

  frame.window_label =
    XCreateWindow(display, frame.window, 0, 0, 1, 1, 0, screen->getDepth(),
                  InputOutput, screen->getVisual(), create_mask, &attrib);
  blackbox->saveToolbarSearch(frame.window_label, this);

  frame.clock =
    XCreateWindow(display, frame.window, 0, 0, 1, 1, 0, screen->getDepth(),
                  InputOutput, screen->getVisual(), create_mask, &attrib);
  blackbox->saveToolbarSearch(frame.clock, this);

  frame.psbutton =
    XCreateWindow(display ,frame.window, 0, 0, 1, 1, 0, screen->getDepth(),
                  InputOutput, screen->getVisual(), create_mask, &attrib);
  blackbox->saveToolbarSearch(frame.psbutton, this);

  frame.nsbutton =
    XCreateWindow(display ,frame.window, 0, 0, 1, 1, 0, screen->getDepth(),
                  InputOutput, screen->getVisual(), create_mask, &attrib);
  blackbox->saveToolbarSearch(frame.nsbutton, this);

  frame.pwbutton =
    XCreateWindow(display ,frame.window, 0, 0, 1, 1, 0, screen->getDepth(),
                  InputOutput, screen->getVisual(), create_mask, &attrib);
  blackbox->saveToolbarSearch(frame.pwbutton, this);

  frame.nwbutton =
    XCreateWindow(display ,frame.window, 0, 0, 1, 1, 0, screen->getDepth(),
                  InputOutput, screen->getVisual(), create_mask, &attrib);
  blackbox->saveToolbarSearch(frame.nwbutton, this);

  frame.base = frame.label = frame.wlabel = frame.clk = frame.button =
    frame.pbutton = None;

  reconfigure();
  mapToolbar();
}


Toolbar::~Toolbar(void) {
  unmapToolbar();

  if (frame.base) screen->getImageControl()->removeImage(frame.base);
  if (frame.label) screen->getImageControl()->removeImage(frame.label);
  if (frame.wlabel) screen->getImageControl()->removeImage(frame.wlabel);
  if (frame.clk) screen->getImageControl()->removeImage(frame.clk);
  if (frame.button) screen->getImageControl()->removeImage(frame.button);
  if (frame.pbutton) screen->getImageControl()->removeImage(frame.pbutton);

  blackbox->removeToolbarSearch(frame.window);
  blackbox->removeToolbarSearch(frame.workspace_label);
  blackbox->removeToolbarSearch(frame.window_label);
  blackbox->removeToolbarSearch(frame.clock);
  blackbox->removeToolbarSearch(frame.psbutton);
  blackbox->removeToolbarSearch(frame.nsbutton);
  blackbox->removeToolbarSearch(frame.pwbutton);
  blackbox->removeToolbarSearch(frame.nwbutton);

  XDestroyWindow(display, frame.workspace_label);
  XDestroyWindow(display, frame.window_label);
  XDestroyWindow(display, frame.clock);

  XDestroyWindow(display, frame.window);

  delete hide_timer;
  delete clock_timer;
  delete toolbarmenu;
}


void Toolbar::mapToolbar() {
  if (!screen->doHideToolbar()) {
    //not hidden, so windows should not maximize over the toolbar
    XMapSubwindows(display, frame.window);
    XMapWindow(display, frame.window);
  }
  screen->addStrut(&strut);
  updateStrut();
}


void Toolbar::unmapToolbar() {
  if (toolbarmenu->isVisible())
    toolbarmenu->hide();
  //hidden so we can maximize over the toolbar
  screen->removeStrut(&strut);
  screen->updateAvailableArea();

  XUnmapWindow(display, frame.window);
  updateStrut();
}


void Toolbar::saveOnTop(bool b) {
  on_top = b;
  config->setValue(toolbarstr + "onTop", on_top);
}


void Toolbar::saveAutoHide(bool b) {
  do_auto_hide = b;
  config->setValue(toolbarstr + "autoHide", do_auto_hide);
}


void Toolbar::saveWidthPercent(unsigned int w) {
  width_percent = w;
  config->setValue(toolbarstr + "widthPercent", width_percent);
}


void Toolbar::savePlacement(int p) {
  placement = p;
  const char *pname;
  switch (placement) {
  case TopLeft: pname = "TopLeft"; break;
  case BottomLeft: pname = "BottomLeft"; break;
  case TopCenter: pname = "TopCenter"; break;
  case TopRight: pname = "TopRight"; break;
  case BottomRight: pname = "BottomRight"; break;
  case BottomCenter: default: pname = "BottomCenter"; break;
  }
  config->setValue(toolbarstr + "placement", pname);
}


void Toolbar::save_rc(void) {
  saveOnTop(on_top);
  saveAutoHide(do_auto_hide);
  saveWidthPercent(width_percent);
  savePlacement(placement);
}


void Toolbar::load_rc(void) {
  string s;
  
  if (! config->getValue(toolbarstr + "onTop", on_top))
    on_top = false;

  if (! config->getValue(toolbarstr + "autoHide", do_auto_hide))
    do_auto_hide = false;
  hidden = do_auto_hide;

  if (! config->getValue(toolbarstr + "widthPercent", width_percent) ||
      width_percent == 0 || width_percent > 100)
    width_percent = 66;

  if (config->getValue(toolbarstr + "placement", s)) {
    if (s == "TopLeft")
      placement = TopLeft;
    else if (s == "BottomLeft")
      placement = BottomLeft;
    else if (s == "TopCenter")
      placement = TopCenter;
    else if (s == "TopRight")
      placement = TopRight;
    else if (s == "BottomRight")
      placement = BottomRight;
    else //if (s == "BottomCenter")
      placement = BottomCenter;
  } else
    placement = BottomCenter;
}


void Toolbar::reconfigure(void) {
  unsigned int width, height;
 
  width = (screen->getWidth() * width_percent) / 100;
  height = screen->getToolbarStyle()->font->height();

  frame.bevel_w = screen->getBevelWidth();
  frame.button_w = height;
  height += 2;
  frame.label_h = height;
  height += (frame.bevel_w * 2);

  frame.rect.setSize(width, height);

  int x, y;
  switch (placement) {
  case TopLeft:
  case TopRight:
  case TopCenter:
    if (placement == TopLeft)
      x = 0;
    else if (placement == TopRight)
      x = screen->getWidth() - frame.rect.width()
        - (screen->getBorderWidth() * 2);
    else
      x = (screen->getWidth() - frame.rect.width()) / 2;

    y = 0;

    frame.x_hidden = x;
    frame.y_hidden = screen->getBevelWidth() - screen->getBorderWidth()
                     - frame.rect.height();
    break;

  case BottomLeft:
  case BottomRight:
  case BottomCenter:
  default:
    if (placement == BottomLeft)
      x = 0;
    else if (placement == BottomRight)
      x = screen->getWidth() - frame.rect.width()
        - (screen->getBorderWidth() * 2);
    else
      x = (screen->getWidth() - frame.rect.width()) / 2;

    y = screen->getHeight() - frame.rect.height()
      - (screen->getBorderWidth() * 2);

    frame.x_hidden = x;
    frame.y_hidden = screen->getHeight() - screen->getBevelWidth()
                     - screen->getBorderWidth();
    break;
  }

  frame.rect.setPos(x, y);

  updateStrut();

#ifdef    HAVE_STRFTIME
  time_t ttmp = time(NULL);

  frame.clock_w = 0;
  if (ttmp != -1) {
    struct tm *tt = localtime(&ttmp);
    if (tt) {
      char t[1024];
      int len = strftime(t, 1024, screen->getStrftimeFormat(), tt);
      if (len == 0) { // invalid time format found
        screen->saveStrftimeFormat("%I:%M %p"); // so use the default
        strftime(t, 1024, screen->getStrftimeFormat(), tt);
      }
      // find the length of the rendered string and add room for two extra
      // characters to it.  This allows for variable width output of the fonts
      BFont *font = screen->getToolbarStyle()->font;
      frame.clock_w = font->measureString(t) + font->maxCharWidth() * 2;
    }
  }
#else // !HAVE_STRFTIME
  {
    string s = i18n(ToolbarSet, ToolbarNoStrftimeLength, "00:00000");
    frame.clock_w = screen->getToolbarStyle()->font->measureString(s);
  }
#endif // HAVE_STRFTIME

  frame.workspace_label_w = 0;

  for (unsigned int i = 0; i < screen->getWorkspaceCount(); i++) {
    const string& workspace_name = screen->getWorkspace(i)->getName();
    width = screen->getToolbarStyle()->font->measureString(workspace_name);
    if (width > frame.workspace_label_w) frame.workspace_label_w = width;
  }

  frame.workspace_label_w = frame.clock_w =
    std::max(frame.workspace_label_w, frame.clock_w) + (frame.bevel_w * 4);

  // XXX: where'd the +6 come from?
  frame.window_label_w =
    (frame.rect.width() - (frame.clock_w + (frame.button_w * 4) +
                           frame.workspace_label_w + (frame.bevel_w * 8) + 6));

  if (hidden) {
    XMoveResizeWindow(display, frame.window, frame.x_hidden, frame.y_hidden,
                      frame.rect.width(), frame.rect.height());
  } else {
    XMoveResizeWindow(display, frame.window, frame.rect.x(), frame.rect.y(),
                      frame.rect.width(), frame.rect.height());
  }

  XMoveResizeWindow(display, frame.workspace_label, frame.bevel_w,
                    frame.bevel_w, frame.workspace_label_w,
                    frame.label_h);
  XMoveResizeWindow(display, frame.psbutton,
                    ((frame.bevel_w * 2) + frame.workspace_label_w + 1),
                    frame.bevel_w + 1, frame.button_w, frame.button_w);
  XMoveResizeWindow(display, frame.nsbutton,
                    ((frame.bevel_w * 3) + frame.workspace_label_w +
                     frame.button_w + 2), frame.bevel_w + 1, frame.button_w,
                    frame.button_w);
  XMoveResizeWindow(display, frame.window_label,
                    ((frame.bevel_w * 4) + (frame.button_w * 2) +
                     frame.workspace_label_w + 3), frame.bevel_w,
                    frame.window_label_w, frame.label_h);
  XMoveResizeWindow(display, frame.pwbutton,
                    ((frame.bevel_w * 5) + (frame.button_w * 2) +
                     frame.workspace_label_w + frame.window_label_w + 4),
                    frame.bevel_w + 1, frame.button_w, frame.button_w);
  XMoveResizeWindow(display, frame.nwbutton,
                    ((frame.bevel_w * 6) + (frame.button_w * 3) +
                     frame.workspace_label_w + frame.window_label_w + 5),
                    frame.bevel_w + 1, frame.button_w, frame.button_w);
  XMoveResizeWindow(display, frame.clock,
                    frame.rect.width() - frame.clock_w - (frame.bevel_w * 2),
                    frame.bevel_w, frame.clock_w, frame.label_h);

  ToolbarStyle *style = screen->getToolbarStyle();
  frame.base = style->toolbar.render(frame.rect.width(), frame.rect.height(),
                                     frame.base);
  if (! frame.base)
    XSetWindowBackground(display, frame.window,
                         style->toolbar.color().pixel());
  else
    XSetWindowBackgroundPixmap(display, frame.window, frame.base);

  frame.label = style->window.render(frame.window_label_w, frame.label_h,
                                     frame.label);
  if (! frame.label)
    XSetWindowBackground(display, frame.window_label,
                         style->window.color().pixel());
  else
    XSetWindowBackgroundPixmap(display, frame.window_label, frame.label);

  frame.wlabel = style->label.render(frame.workspace_label_w, frame.label_h,
                                frame.wlabel);
  if (! frame.wlabel)
    XSetWindowBackground(display, frame.workspace_label,
                         style->label.color().pixel());
  else
    XSetWindowBackgroundPixmap(display, frame.workspace_label, frame.wlabel);

  frame.clk = style->clock.render(frame.clock_w, frame.label_h, frame.clk);
  if (! frame.clk)
    XSetWindowBackground(display, frame.clock, style->clock.color().pixel());
  else
    XSetWindowBackgroundPixmap(display, frame.clock, frame.clk);

  frame.button = style->button.render(frame.button_w, frame.button_w,
                                frame.button);
  if (! frame.button) {
    frame.button_pixel = style->button.color().pixel();
    XSetWindowBackground(display, frame.psbutton, frame.button_pixel);
    XSetWindowBackground(display, frame.nsbutton, frame.button_pixel);
    XSetWindowBackground(display, frame.pwbutton, frame.button_pixel);
    XSetWindowBackground(display, frame.nwbutton, frame.button_pixel);
  } else {
    XSetWindowBackgroundPixmap(display, frame.psbutton, frame.button);
    XSetWindowBackgroundPixmap(display, frame.nsbutton, frame.button);
    XSetWindowBackgroundPixmap(display, frame.pwbutton, frame.button);
    XSetWindowBackgroundPixmap(display, frame.nwbutton, frame.button);
  }

  frame.pbutton = style->pressed.render(frame.button_w, frame.button_w,
                                        frame.pbutton);
  if (! frame.pbutton)
    frame.pbutton_pixel = style->pressed.color().pixel();

  XSetWindowBorder(display, frame.window,
                   screen->getBorderColor()->pixel());
  XSetWindowBorderWidth(display, frame.window, screen->getBorderWidth());

  XClearWindow(display, frame.window);
  XClearWindow(display, frame.workspace_label);
  XClearWindow(display, frame.window_label);
  XClearWindow(display, frame.clock);
  XClearWindow(display, frame.psbutton);
  XClearWindow(display, frame.nsbutton);
  XClearWindow(display, frame.pwbutton);
  XClearWindow(display, frame.nwbutton);

  redrawWindowLabel();
  redrawWorkspaceLabel();
  redrawPrevWorkspaceButton();
  redrawNextWorkspaceButton();
  redrawPrevWindowButton();
  redrawNextWindowButton();
  checkClock(True);

  toolbarmenu->reconfigure();
}


void Toolbar::updateStrut(void) {
  // left and right are always 0
  strut.top = strut.bottom = 0;

  // when hidden only one border is visible
  unsigned int border_width = screen->getBorderWidth();
  if (! do_auto_hide)
    border_width *= 2;

  if (! screen->doHideToolbar()) {
    switch(placement) {
    case TopLeft:
    case TopCenter:
    case TopRight:
      strut.top = getExposedHeight() + border_width;
      break;
    default:
      strut.bottom = getExposedHeight() + border_width;
    }
  }

  screen->updateAvailableArea();
}


#ifdef    HAVE_STRFTIME
void Toolbar::checkClock(bool redraw) {
#else // !HAVE_STRFTIME
void Toolbar::checkClock(bool redraw, bool date) {
#endif // HAVE_STRFTIME
  time_t tmp = 0;
  struct tm *tt = 0;

  if ((tmp = time(NULL)) != -1) {
    if (! (tt = localtime(&tmp))) return;
    if (tt->tm_min != frame.minute || tt->tm_hour != frame.hour) {
      frame.hour = tt->tm_hour;
      frame.minute = tt->tm_min;
      XClearWindow(display, frame.clock);
      redraw = True;
    }
  }

  if (redraw) {
#ifdef    HAVE_STRFTIME
    char t[1024];
    if (! strftime(t, 1024, screen->getStrftimeFormat(), tt))
      return;
#else // !HAVE_STRFTIME
    char t[9];
    if (date) {
      // format the date... with special consideration for y2k ;)
      if (screen->getDateFormat() == Blackbox::B_EuropeanDate)
        sprintf(t, i18n(ToolbarSet, ToolbarNoStrftimeDateFormatEu,
                       "%02d.%02d.%02d"),
                tt->tm_mday, tt->tm_mon + 1,
                (tt->tm_year >= 100) ? tt->tm_year - 100 : tt->tm_year);
      else
        sprintf(t, i18n(ToolbarSet, ToolbarNoStrftimeDateFormat,
                        "%02d/%02d/%02d"),
                tt->tm_mon + 1, tt->tm_mday,
                (tt->tm_year >= 100) ? tt->tm_year - 100 : tt->tm_year);
    } else {
      if (screen->isClock24Hour())
        sprintf(t, i18n(ToolbarSet, ToolbarNoStrftimeTimeFormat24,
                        "  %02d:%02d "),
                frame.hour, frame.minute);
      else
        sprintf(t, i18n(ToolbarSet, ToolbarNoStrftimeTimeFormat12,
                        "%02d:%02d %sm"),
                ((frame.hour > 12) ? frame.hour - 12 :
                 ((frame.hour == 0) ? 12 : frame.hour)), frame.minute,
                ((frame.hour >= 12) ?
                 i18n(ToolbarSet, ToolbarNoStrftimeTimeFormatP, "p") :
                 i18n(ToolbarSet, ToolbarNoStrftimeTimeFormatA, "a")));
    }
#endif // HAVE_STRFTIME

    ToolbarStyle *style = screen->getToolbarStyle();

    int pos = frame.bevel_w * 2; // this is modified by doJustify()
    style->doJustify(t, pos, frame.clock_w, frame.bevel_w * 4);

#ifdef    XFT
    XClearWindow(display, frame.clock);
#endif // XFT

    style->font->drawString(frame.clock, pos, 1, style->c_text, t);
  }
}


void Toolbar::redrawWindowLabel(bool redraw) {
  BlackboxWindow *foc = screen->getBlackbox()->getFocusedWindow();
  if (! foc) {
    XClearWindow(display, frame.window_label);
    return;
  }

#ifdef    XFT
  redraw = true;
#endif // XFT

  if (redraw)
    XClearWindow(display, frame.window_label);

  if (foc->getScreen() != screen) return;

  const char *title = foc->getTitle();
  ToolbarStyle *style = screen->getToolbarStyle();

  int pos = frame.bevel_w * 2; // modified by doJustify()
  style->doJustify(title, pos, frame.window_label_w, frame.bevel_w * 4);
  style->font->drawString(frame.window_label, pos, 1, style->w_text, title);
}


void Toolbar::redrawWorkspaceLabel(bool redraw) {
  const string& name = screen->getCurrentWorkspace()->getName();

#ifdef    XFT
  redraw = true;
#endif // XFT

  if (redraw)
    XClearWindow(display, frame.workspace_label);

  ToolbarStyle *style = screen->getToolbarStyle();

  int pos = frame.bevel_w * 2;
  style->doJustify(name.c_str(), pos, frame.workspace_label_w,
                   frame.bevel_w * 4);
  style->font->drawString(frame.workspace_label, pos, 1, style->l_text, name);
}


void Toolbar::drawArrow(Drawable surface, bool left) const {
  ToolbarStyle *style = screen->getToolbarStyle();

  BPen pen(style->b_pic);

  int hh = frame.button_w / 2, hw = frame.button_w / 2;
  XPoint pts[3];
  const int bullet_size = 3;


  if (left) {
#ifdef    BITMAPBUTTONS
    if (style->left_button.mask != None) {
      XSetClipMask(blackbox->getXDisplay(), pen.gc(), style->left_button.mask);
      XSetClipOrigin(blackbox->getXDisplay(), pen.gc(),
                     (frame.button_w - style->left_button.w)/2,
                     (frame.button_w - style->left_button.h)/2);

      XFillRectangle(blackbox->getXDisplay(), surface, pen.gc(),
                     (frame.button_w - style->left_button.w)/2,
                     (frame.button_w - style->left_button.h)/2,
                     style->left_button.w, style->left_button.h);

      XSetClipMask(blackbox->getXDisplay(), pen.gc(), None);
      XSetClipOrigin(blackbox->getXDisplay(), pen.gc(), 0, 0);
    } else {
#endif // BITMAPBUTTONS
      pts[0].x = hw - bullet_size;
      pts[0].y = hh;
      pts[1].x = 2 * bullet_size;
      pts[1].y = bullet_size;
      pts[2].x = 0;
      pts[2].y = -(2 * bullet_size);
      XFillPolygon(display, surface, pen.gc(), pts, 3, Convex,
                   CoordModePrevious);
#ifdef    BITMAPBUTTONS      
    }
#endif // BITMAPBUTTONS
  } else {
#ifdef    BITMAPBUTTONS
    if (style->right_button.mask != None) {
      XSetClipMask(blackbox->getXDisplay(), pen.gc(),
                   style->right_button.mask);
      XSetClipOrigin(blackbox->getXDisplay(), pen.gc(),
                     (frame.button_w - style->right_button.w)/2,
                     (frame.button_w - style->right_button.h)/2);

      XFillRectangle(blackbox->getXDisplay(), surface, pen.gc(),
                     (frame.button_w - style->right_button.w)/2,
                     (frame.button_w - style->right_button.h)/2,
                     (frame.button_w + style->right_button.w)/2,
                     (frame.button_w + style->right_button.h)/2);

      XSetClipMask(blackbox->getXDisplay(), pen.gc(), None);
      XSetClipOrigin(blackbox->getXDisplay(), pen.gc(), 0, 0);
    } else {
#endif // BITMAPBUTTONS
      pts[0].x = hw - bullet_size;
      pts[0].y = hh - bullet_size;
      pts[1].x = (2 * bullet_size);
      pts[1].y =  bullet_size;
      pts[2].x = -(2 * bullet_size);
      pts[2].y = bullet_size;
      XFillPolygon(display, surface, pen.gc(), pts, 3, Convex,
                   CoordModePrevious);
#ifdef    BITMAPBUTTONS
    }
#endif
  }
}


void Toolbar::redrawPrevWorkspaceButton(bool pressed, bool redraw) {
  if (redraw) {
    if (pressed) {
      if (frame.pbutton)
        XSetWindowBackgroundPixmap(display, frame.psbutton, frame.pbutton);
      else
        XSetWindowBackground(display, frame.psbutton, frame.pbutton_pixel);
    } else {
      if (frame.button)
        XSetWindowBackgroundPixmap(display, frame.psbutton, frame.button);
      else
        XSetWindowBackground(display, frame.psbutton, frame.button_pixel);
    }
    XClearWindow(display, frame.psbutton);
  }

  drawArrow(frame.psbutton, True);
}


void Toolbar::redrawNextWorkspaceButton(bool pressed, bool redraw) {
  if (redraw) {
    if (pressed) {
      if (frame.pbutton)
        XSetWindowBackgroundPixmap(display, frame.nsbutton, frame.pbutton);
      else
        XSetWindowBackground(display, frame.nsbutton, frame.pbutton_pixel);
    } else {
      if (frame.button)
        XSetWindowBackgroundPixmap(display, frame.nsbutton, frame.button);
      else
        XSetWindowBackground(display, frame.nsbutton, frame.button_pixel);
    }
    XClearWindow(display, frame.nsbutton);
  }

  drawArrow(frame.nsbutton, False);
}


void Toolbar::redrawPrevWindowButton(bool pressed, bool redraw) {
  if (redraw) {
    if (pressed) {
      if (frame.pbutton)
        XSetWindowBackgroundPixmap(display, frame.pwbutton, frame.pbutton);
      else
        XSetWindowBackground(display, frame.pwbutton, frame.pbutton_pixel);
    } else {
      if (frame.button)
        XSetWindowBackgroundPixmap(display, frame.pwbutton, frame.button);
      else
        XSetWindowBackground(display, frame.pwbutton, frame.button_pixel);
    }
    XClearWindow(display, frame.pwbutton);
  }

  drawArrow(frame.pwbutton, True);
}


void Toolbar::redrawNextWindowButton(bool pressed, bool redraw) {
  if (redraw) {
    if (pressed) {
      if (frame.pbutton)
        XSetWindowBackgroundPixmap(display, frame.nwbutton, frame.pbutton);
      else
        XSetWindowBackground(display, frame.nwbutton, frame.pbutton_pixel);
    } else {
      if (frame.button)
        XSetWindowBackgroundPixmap(display, frame.nwbutton, frame.button);
      else
        XSetWindowBackground(display, frame.nwbutton, frame.button_pixel);
    }
    XClearWindow(display, frame.nwbutton);
  }

  drawArrow(frame.nwbutton, False);
}


void Toolbar::edit(void) {
  Window window;
  int foo;

  editing = True;
  XGetInputFocus(display, &window, &foo);
  if (window == frame.workspace_label)
    return;

  XSetInputFocus(display, frame.workspace_label,
                 RevertToPointerRoot, CurrentTime);
  XClearWindow(display, frame.workspace_label);

  blackbox->setNoFocus(True);
  if (blackbox->getFocusedWindow())
    blackbox->getFocusedWindow()->setFocusFlag(False);

  ToolbarStyle *style = screen->getToolbarStyle();
  BPen pen(style->l_text);
  XDrawRectangle(display, frame.workspace_label, pen.gc(),
                 frame.workspace_label_w / 2, 0, 1,
                 frame.label_h - 1);
  // change the background of the window to that of an active window label
  BTexture *texture = &(screen->getWindowStyle()->l_focus);
  frame.wlabel = texture->render(frame.workspace_label_w, frame.label_h,
                                 frame.wlabel);
  if (! frame.wlabel)
    XSetWindowBackground(display, frame.workspace_label,
                         texture->color().pixel());
  else
    XSetWindowBackgroundPixmap(display, frame.workspace_label, frame.wlabel);
}


void Toolbar::buttonPressEvent(const XButtonEvent *be) {
  if (be->button == 1) {
    if (be->window == frame.psbutton)
      redrawPrevWorkspaceButton(True, True);
    else if (be->window == frame.nsbutton)
      redrawNextWorkspaceButton(True, True);
    else if (be->window == frame.pwbutton)
      redrawPrevWindowButton(True, True);
    else if (be->window == frame.nwbutton)
      redrawNextWindowButton(True, True);
#ifndef   HAVE_STRFTIME
    else if (be->window == frame.clock) {
      XClearWindow(display, frame.clock);
      checkClock(True, True);
    }
#endif // HAVE_STRFTIME
    else if (! on_top) {
      Window w[1] = { frame.window };
      screen->raiseWindows(w, 1);
    }
  } else if (be->button == 2 && (! on_top)) {
    XLowerWindow(display, frame.window);
  } else if (be->button == 3) {
    if (toolbarmenu->isVisible()) {
      toolbarmenu->hide();
    } else {
      int x, y;

      x = be->x_root - (toolbarmenu->getWidth() / 2);
      y = be->y_root - (toolbarmenu->getHeight() / 2);

      if (x < 0)
        x = 0;
      else if (x + toolbarmenu->getWidth() > screen->getWidth())
        x = screen->getWidth() - toolbarmenu->getWidth();

      if (y < 0)
        y = 0;
      else if (y + toolbarmenu->getHeight() > screen->getHeight())
        y = screen->getHeight() - toolbarmenu->getHeight();

      toolbarmenu->move(x, y);
      toolbarmenu->show();
    }
  }
}



void Toolbar::buttonReleaseEvent(const XButtonEvent *re) {
  if (re->button == 1) {
    if (re->window == frame.psbutton) {
      redrawPrevWorkspaceButton(False, True);

      if (re->x >= 0 && re->x < static_cast<signed>(frame.button_w) &&
          re->y >= 0 && re->y < static_cast<signed>(frame.button_w))
       if (screen->getCurrentWorkspace()->getID() > 0)
          screen->changeWorkspaceID(screen->getCurrentWorkspace()->
                                    getID() - 1);
        else
          screen->changeWorkspaceID(screen->getWorkspaceCount() - 1);
    } else if (re->window == frame.nsbutton) {
      redrawNextWorkspaceButton(False, True);

      if (re->x >= 0 && re->x < static_cast<signed>(frame.button_w) &&
          re->y >= 0 && re->y < static_cast<signed>(frame.button_w))
        if (screen->getCurrentWorkspace()->getID() <
            (screen->getWorkspaceCount() - 1))
          screen->changeWorkspaceID(screen->getCurrentWorkspace()->
                                    getID() + 1);
        else
          screen->changeWorkspaceID(0);
    } else if (re->window == frame.pwbutton) {
      redrawPrevWindowButton(False, True);

      if (re->x >= 0 && re->x < static_cast<signed>(frame.button_w) &&
          re->y >= 0 && re->y < static_cast<signed>(frame.button_w))
        screen->prevFocus();
    } else if (re->window == frame.nwbutton) {
      redrawNextWindowButton(False, True);

      if (re->x >= 0 && re->x < static_cast<signed>(frame.button_w) &&
          re->y >= 0 && re->y < static_cast<signed>(frame.button_w))
        screen->nextFocus();
    } else if (re->window == frame.window_label)
      screen->raiseFocus();
#ifndef   HAVE_STRFTIME
    else if (re->window == frame.clock) {
      XClearWindow(display, frame.clock);
      checkClock(True);
    }
#endif // HAVE_STRFTIME
  }
}


void Toolbar::enterNotifyEvent(const XCrossingEvent *) {
  if (! do_auto_hide)
    return;

  if (hidden) {
    if (! hide_timer->isTiming()) hide_timer->start();
  } else {
    if (hide_timer->isTiming()) hide_timer->stop();
  }
}

void Toolbar::leaveNotifyEvent(const XCrossingEvent *) {
  if (! do_auto_hide)
    return;

  if (hidden) {
    if (hide_timer->isTiming()) hide_timer->stop();
  } else if (! toolbarmenu->isVisible()) {
    if (! hide_timer->isTiming()) hide_timer->start();
  }
}


void Toolbar::exposeEvent(const XExposeEvent *ee) {
  if (ee->window == frame.clock) checkClock(True);
  else if (ee->window == frame.workspace_label && (! editing))
    redrawWorkspaceLabel();
  else if (ee->window == frame.window_label) redrawWindowLabel();
  else if (ee->window == frame.psbutton) redrawPrevWorkspaceButton();
  else if (ee->window == frame.nsbutton) redrawNextWorkspaceButton();
  else if (ee->window == frame.pwbutton) redrawPrevWindowButton();
  else if (ee->window == frame.nwbutton) redrawNextWindowButton();
}


void Toolbar::keyPressEvent(const XKeyEvent *ke) {
  if (ke->window == frame.workspace_label && editing) {
    if (new_workspace_name.empty()) {
      new_name_pos = 0;
    }

    KeySym ks;
    char keychar[1];
    XLookupString(const_cast<XKeyEvent*>(ke), keychar, 1, &ks, 0);

    // either we are told to end with a return or we hit 127 chars
    if (ks == XK_Return || new_name_pos == 127) {
      editing = False;

      blackbox->setNoFocus(False);
      if (blackbox->getFocusedWindow())
        blackbox->getFocusedWindow()->setInputFocus();
      else
        blackbox->setFocusedWindow(0);

      // the toolbar will be reconfigured when the change to the workspace name
      // gets caught in the PropertyNotify event handler
      screen->getCurrentWorkspace()->setName(new_workspace_name);

      new_workspace_name.erase();
      new_name_pos = 0;

      // reset the background to that of the workspace label (its normal
      // setting)
      BTexture *texture = &(screen->getToolbarStyle()->label);
      frame.wlabel = texture->render(frame.workspace_label_w, frame.label_h,
                                     frame.wlabel);
      if (! frame.wlabel)
        XSetWindowBackground(display, frame.workspace_label,
                             texture->color().pixel());
      else
        XSetWindowBackgroundPixmap(display, frame.workspace_label,
                                   frame.wlabel);
    } else if (! (ks == XK_Shift_L || ks == XK_Shift_R ||
                  ks == XK_Control_L || ks == XK_Control_R ||
                  ks == XK_Caps_Lock || ks == XK_Shift_Lock ||
                  ks == XK_Meta_L || ks == XK_Meta_R ||
                  ks == XK_Alt_L || ks == XK_Alt_R ||
                  ks == XK_Super_L || ks == XK_Super_R ||
                  ks == XK_Hyper_L || ks == XK_Hyper_R)) {
      if (ks == XK_BackSpace) {
        if (new_name_pos > 0) {
          --new_name_pos;
          new_workspace_name.erase(new_name_pos);
        } else {
          new_workspace_name.resize(0);
        }
      } else {
        new_workspace_name += (*keychar);
        ++new_name_pos;
      }

      XClearWindow(display, frame.workspace_label);
      unsigned int tw, x;

      tw = screen->getToolbarStyle()->font->measureString(new_workspace_name);
      x = (frame.workspace_label_w - tw) / 2;

      if (x < frame.bevel_w) x = frame.bevel_w;

      ToolbarStyle *style = screen->getToolbarStyle();
      style->font->drawString(frame.workspace_label, x, 1, style->l_text,
                              new_workspace_name);
      BPen pen(style->l_text);
      XDrawRectangle(display, frame.workspace_label, pen.gc(), x + tw, 0, 1,
                     frame.label_h - 1);
    }
  }
}


void Toolbar::timeout(void) {
  checkClock(True);

  clock_timer->setTimeout(aMinuteFromNow());
}


void Toolbar::HideHandler::timeout(void) {
  toolbar->hidden = ! toolbar->hidden;
  if (toolbar->hidden)
    XMoveWindow(toolbar->display, toolbar->frame.window,
                toolbar->frame.x_hidden, toolbar->frame.y_hidden);
  else
    XMoveWindow(toolbar->display, toolbar->frame.window,
                toolbar->frame.rect.x(), toolbar->frame.rect.y());
}


void Toolbar::toggleAutoHide(void) {
  saveAutoHide(! doAutoHide());

  updateStrut();
  screen->getSlit()->reposition();

  if (do_auto_hide == False && hidden) {
    // force the slit to be visible
    if (hide_timer->isTiming()) hide_timer->stop();
    hide_handler.timeout();
  }
}


Toolbarmenu::Toolbarmenu(Toolbar *tb) : Basemenu(tb->screen) {
  toolbar = tb;

  setLabel(i18n(ToolbarSet, ToolbarToolbarTitle, "Toolbar"));
  setInternalMenu();

  placementmenu = new Placementmenu(this);

  insert(i18n(CommonSet, CommonPlacementTitle, "Placement"),
         placementmenu);
  insert(i18n(CommonSet, CommonAlwaysOnTop, "Always on top"), 1);
  insert(i18n(CommonSet, CommonAutoHide, "Auto hide"), 2);
  insert(i18n(ToolbarSet, ToolbarEditWkspcName,
              "Edit current workspace name"), 3);

  update();
  setValues();
}


void Toolbarmenu::setValues() {
  setItemSelected(1, toolbar->isOnTop());
  setItemSelected(2, toolbar->doAutoHide());
}


Toolbarmenu::~Toolbarmenu(void) {
  delete placementmenu;
}


void Toolbarmenu::itemSelected(int button, unsigned int index) {
  if (button != 1)
    return;

  BasemenuItem *item = find(index);
  if (! item) return;

  switch (item->function()) {
  case 1: { // always on top
    toolbar->saveOnTop(! toolbar->isOnTop());
    setItemSelected(1, toolbar->isOnTop());

    if (toolbar->isOnTop()) getScreen()->raiseWindows((Window *) 0, 0);
    break;
  }

  case 2: { // auto hide
    toolbar->toggleAutoHide();
    setItemSelected(2, toolbar->doAutoHide());

    break;
  }

  case 3: { // edit current workspace name
    toolbar->edit();
    hide();

    break;
  }
  } // switch
}


void Toolbarmenu::internal_hide(void) {
  Basemenu::internal_hide();
  if (toolbar->doAutoHide() && ! toolbar->isEditing())
    toolbar->hide_handler.timeout();
}


void Toolbarmenu::reconfigure(void) {
  setValues();
  placementmenu->reconfigure();

  Basemenu::reconfigure();
}


Toolbarmenu::Placementmenu::Placementmenu(Toolbarmenu *tm)
  : Basemenu(tm->toolbar->screen), toolbar(tm->toolbar) {
  setLabel(i18n(ToolbarSet, ToolbarToolbarPlacement, "Toolbar Placement"));
  setInternalMenu();
  setMinimumSublevels(3);

  insert(i18n(CommonSet, CommonPlacementTopLeft, "Top Left"),
         Toolbar::TopLeft);
  insert(i18n(CommonSet, CommonPlacementBottomLeft, "Bottom Left"),
         Toolbar::BottomLeft);
  insert(i18n(CommonSet, CommonPlacementTopCenter, "Top Center"),
         Toolbar::TopCenter);
  insert(i18n(CommonSet, CommonPlacementBottomCenter, "Bottom Center"),
         Toolbar::BottomCenter);
  insert(i18n(CommonSet, CommonPlacementTopRight, "Top Right"),
         Toolbar::TopRight);
  insert(i18n(CommonSet, CommonPlacementBottomRight, "Bottom Right"),
         Toolbar::BottomRight);
  update();
  setValues();
}


void Toolbarmenu::Placementmenu::setValues(void) {
  int place = 0;
  switch (toolbar->getPlacement()) {
  case Toolbar::BottomRight:
    place++;
  case Toolbar::TopRight:
    place++;
  case Toolbar::BottomCenter:
    place++;
  case Toolbar::TopCenter:
    place++;
  case Toolbar::BottomLeft:
    place++;
  case Toolbar::TopLeft:
    break;
  }
  setItemSelected(0, 0 == place);
  setItemSelected(1, 1 == place);
  setItemSelected(2, 2 == place);
  setItemSelected(3, 3 == place);
  setItemSelected(4, 4 == place);
  setItemSelected(5, 5 == place);
}


void Toolbarmenu::Placementmenu::reconfigure(void) {
  setValues();
  Basemenu::reconfigure();
}


void Toolbarmenu::Placementmenu::itemSelected(int button, unsigned int index) {
  if (button != 1)
    return;

  BasemenuItem *item = find(index);
  if (! item) return;

  toolbar->savePlacement(item->function());
  hide();
  toolbar->reconfigure();

  // reposition the slit as well to make sure it doesn't intersect the
  // toolbar
  getScreen()->getSlit()->reposition();
}


void ToolbarStyle::doJustify(const std::string &text, int &start_pos,
                             unsigned int max_length,
                             unsigned int modifier) const {
  size_t text_len = text.size();
  unsigned int length;

  do {
    length = font->measureString(string(text, 0, text_len)) + modifier;
  } while (length > max_length && text_len-- > 0);

  switch (justify) {
  case RightJustify:
    start_pos += max_length - length;
    break;

  case CenterJustify:
    start_pos += (max_length - length) / 2;
    break;

  case LeftJustify:
  default:
    break;
  }
}
