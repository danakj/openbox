// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// Basemenu.cc for Blackbox - an X11 Window manager
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
#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#ifdef HAVE_STRING_H
#  include <string.h>
#endif // HAVE_STRING_H
}

#include <algorithm>
using namespace std;

#include "i18n.hh"
#include "blackbox.hh"
#include "Basemenu.hh"
#include "GCCache.hh"
#include "Image.hh"
#include "Screen.hh"
#include "Util.hh"


static Basemenu *shown = (Basemenu *) 0;

Basemenu::Basemenu(BScreen *scrn) {
  screen = scrn;
  blackbox = screen->getBlackbox();
  image_ctrl = screen->getImageControl();
  display = blackbox->getXDisplay();
  parent = (Basemenu *) 0;
  alignment = AlignDontCare;

  title_vis =
    movable =
    hide_tree = True;

  shifted =
    internal_menu =
    moving =
    torn =
    visible = False;

  menu.x =
    menu.y =
    menu.x_shift =
    menu.y_shift =
    menu.x_move =
    menu.y_move = 0;

  which_sub =
    which_press =
    which_sbl = -1;

  menu.frame_pixmap =
    menu.title_pixmap =
    menu.hilite_pixmap =
    menu.sel_pixmap = None;

  menu.bevel_w = screen->getBevelWidth();

  if (i18n.multibyte())
    menu.width = menu.title_h = menu.item_w = menu.frame_h =
      screen->getMenuStyle()->t_fontset_extents->max_ink_extent.height +
      (menu.bevel_w  * 2);
  else
    menu.width = menu.title_h = menu.item_w = menu.frame_h =
      screen->getMenuStyle()->t_font->ascent +
      screen->getMenuStyle()->t_font->descent + (menu.bevel_w * 2);

  menu.sublevels =
    menu.persub =
    menu.minsub = 0;

  MenuStyle *style = screen->getMenuStyle();
  if (i18n.multibyte()) {
    menu.item_h = style->f_fontset_extents->max_ink_extent.height +
      (menu.bevel_w);
  } else {
    menu.item_h = style->f_font->ascent + style->f_font->descent +
      (menu.bevel_w);
  }

  menu.height = menu.title_h + screen->getBorderWidth() + menu.frame_h;

  unsigned long attrib_mask = CWBackPixmap | CWBackPixel | CWBorderPixel |
    CWColormap | CWOverrideRedirect | CWEventMask;
  XSetWindowAttributes attrib;
  attrib.background_pixmap = None;
  attrib.background_pixel = attrib.border_pixel =
    screen->getBorderColor()->pixel();
  attrib.colormap = screen->getColormap();
  attrib.override_redirect = True;
  attrib.event_mask = ButtonPressMask | ButtonReleaseMask |
    ButtonMotionMask | ExposureMask;

  menu.window =
    XCreateWindow(display, screen->getRootWindow(),
                  menu.x, menu.y, menu.width, menu.height,
                  screen->getBorderWidth(), screen->getDepth(),
                  InputOutput, screen->getVisual(), attrib_mask, &attrib);
  blackbox->saveMenuSearch(menu.window, this);

  attrib_mask = CWBackPixmap | CWBackPixel | CWBorderPixel | CWEventMask;
  attrib.background_pixel = screen->getBorderColor()->pixel();
  attrib.event_mask |= EnterWindowMask | LeaveWindowMask;

  menu.title =
    XCreateWindow(display, menu.window, 0, 0, menu.width, menu.height, 0,
                  screen->getDepth(), InputOutput, screen->getVisual(),
                  attrib_mask, &attrib);
  blackbox->saveMenuSearch(menu.title, this);

  attrib.event_mask |= PointerMotionMask;
  menu.frame = XCreateWindow(display, menu.window, 0,
                             menu.title_h + screen->getBorderWidth(),
                             menu.width, menu.frame_h, 0,
                             screen->getDepth(), InputOutput,
                             screen->getVisual(), attrib_mask, &attrib);
  blackbox->saveMenuSearch(menu.frame, this);

  // even though this is the end of the constructor the menu is still not
  // completely created.  items must be inserted and it must be update()'d
}

Basemenu::~Basemenu(void) {
  XUnmapWindow(display, menu.window);

  if (shown && shown->getWindowID() == getWindowID())
    shown = (Basemenu *) 0;

  MenuItems::const_iterator it = menuitems.begin();
  while (it != menuitems.end()) {
    BasemenuItem *item = *it;
    if ((! internal_menu)) {
      Basemenu *tmp = (Basemenu *) item->submenu();
      if (tmp) {
        if (! tmp->internal_menu) {
          delete tmp;
        } else {
          tmp->internal_hide();
        }
      }
    }
    ++it;
  }

  std::for_each(menuitems.begin(), menuitems.end(), PointerAssassin());

  if (menu.title_pixmap)
    image_ctrl->removeImage(menu.title_pixmap);

  if (menu.frame_pixmap)
    image_ctrl->removeImage(menu.frame_pixmap);

  if (menu.hilite_pixmap)
    image_ctrl->removeImage(menu.hilite_pixmap);

  if (menu.sel_pixmap)
    image_ctrl->removeImage(menu.sel_pixmap);

  blackbox->removeMenuSearch(menu.title);
  XDestroyWindow(display, menu.title);

  blackbox->removeMenuSearch(menu.frame);
  XDestroyWindow(display, menu.frame);

  blackbox->removeMenuSearch(menu.window);
  XDestroyWindow(display, menu.window);
}


BasemenuItem::~BasemenuItem(void) {}


BasemenuItem *Basemenu::find(int index) {
  if (index < 0 || index > static_cast<signed>(menuitems.size()))
    return (BasemenuItem*) 0;

  return *(menuitems.begin() + index);
}


int Basemenu::insert(BasemenuItem *item, int pos) {
  if (pos < 0) {
    menuitems.push_back(item);
  } else {
    assert(pos < static_cast<signed>(menuitems.size()));
    menuitems.insert((menuitems.begin() + pos), item);
  }
  return menuitems.size();
}


int Basemenu::insert(const string& label, int function,
                     const string& exec, int pos) {
  BasemenuItem *item = new BasemenuItem(label, function, exec);
  return insert(item, pos);
}


int Basemenu::insert(const string& label, Basemenu *submenu, int pos) {
  BasemenuItem *item = new BasemenuItem(label, submenu);
  submenu->parent = this;

  return insert(item, pos);
}


int Basemenu::remove(int index) {
  BasemenuItem *item = find(index);
  if (! item) return -1;

  if ((! internal_menu)) {
    Basemenu *tmp = (Basemenu *) item->submenu();
    if (tmp) {
      if (! tmp->internal_menu) {
        delete tmp;
      } else {
        tmp->internal_hide();
      }
    }
  }

  delete item;

  if (which_sub == index)
    which_sub = -1;
  else if (which_sub > index)
    which_sub--;

  menuitems.erase(menuitems.begin() + index);

  return menuitems.size();
}


void Basemenu::update(void) {
  MenuStyle *style = screen->getMenuStyle();
  if (i18n.multibyte()) {
    menu.item_h = style->f_fontset_extents->max_ink_extent.height +
      menu.bevel_w;
    menu.title_h = style->t_fontset_extents->max_ink_extent.height +
      (menu.bevel_w * 2);
  } else {
    menu.item_h = style->f_font->ascent + style->f_font->descent +
      menu.bevel_w;
    menu.title_h = style->t_font->ascent + style->t_font->descent +
      (menu.bevel_w * 2);
  }

  if (title_vis) {
    const char *s = getLabel();
    int l = strlen(s);

    if (i18n.multibyte()) {
      XRectangle ink, logical;
      XmbTextExtents(screen->getMenuStyle()->t_fontset, s, l, &ink, &logical);
      menu.item_w = logical.width;
    } else {
      menu.item_w = XTextWidth(screen->getMenuStyle()->t_font, s, l);
    }

    menu.item_w += (menu.bevel_w * 2);
  }  else {
    menu.item_w = 1;
  }

  unsigned int ii = 0;
  MenuItems::iterator it = menuitems.begin(), end = menuitems.end();
  for (; it != end; ++it) {
    BasemenuItem *tmp = *it;
    const char *s = tmp->l.c_str();
    int l = strlen(s);

    if (i18n.multibyte()) {
      XRectangle ink, logical;
      XmbTextExtents(screen->getMenuStyle()->f_fontset, s, l, &ink, &logical);
      ii = logical.width;
    } else
      ii = XTextWidth(screen->getMenuStyle()->f_font, s, l);

    ii += (menu.bevel_w * 2) + (menu.item_h * 2);

    menu.item_w = ((menu.item_w < ii) ? ii : menu.item_w);
  }

  if (! menuitems.empty()) {
    menu.sublevels = 1;

    unsigned int menu_size = menuitems.size();
    while (((menu.item_h * (menu_size + 1) / menu.sublevels)
            + menu.title_h + screen->getBorderWidth()) >
           screen->getHeight())
      menu.sublevels++;

    if (menu.sublevels < menu.minsub) menu.sublevels = menu.minsub;

    menu.persub = menu_size / menu.sublevels;
    if (menu_size % menu.sublevels) menu.persub++;
  } else {
    menu.sublevels = 0;
    menu.persub = 0;
  }

  menu.width = (menu.sublevels * (menu.item_w));
  if (! menu.width) menu.width = menu.item_w;

  menu.frame_h = (menu.item_h * menu.persub);
  menu.height = ((title_vis) ? menu.title_h + screen->getBorderWidth() : 0) +
    menu.frame_h;
  if (! menu.frame_h) menu.frame_h = 1;
  if (menu.height < 1) menu.height = 1;

  Pixmap tmp;
  BTexture *texture;
  if (title_vis) {
    tmp = menu.title_pixmap;
    texture = &(screen->getMenuStyle()->title);
    if (texture->texture() == (BTexture::Flat | BTexture::Solid)) {
      menu.title_pixmap = None;
      XSetWindowBackground(display, menu.title,
                           texture->color().pixel());
    } else {
      menu.title_pixmap =
        image_ctrl->renderImage(menu.width, menu.title_h, *texture);
      XSetWindowBackgroundPixmap(display, menu.title, menu.title_pixmap);
    }
    if (tmp) image_ctrl->removeImage(tmp);
    XClearWindow(display, menu.title);
  }

  tmp = menu.frame_pixmap;
  texture = &(screen->getMenuStyle()->frame);
  if (texture->texture() == (BTexture::Flat | BTexture::Solid)) {
    menu.frame_pixmap = None;
    XSetWindowBackground(display, menu.frame,
                         texture->color().pixel());
  } else {
    menu.frame_pixmap =
      image_ctrl->renderImage(menu.width, menu.frame_h, *texture);
    XSetWindowBackgroundPixmap(display, menu.frame, menu.frame_pixmap);
  }
  if (tmp) image_ctrl->removeImage(tmp);

  tmp = menu.hilite_pixmap;
  texture = &(screen->getMenuStyle()->hilite);
  if (texture->texture() == (BTexture::Flat | BTexture::Solid)) {
    menu.hilite_pixmap = None;
  } else {
    menu.hilite_pixmap =
      image_ctrl->renderImage(menu.item_w, menu.item_h, *texture);
  }
  if (tmp) image_ctrl->removeImage(tmp);

  tmp = menu.sel_pixmap;
  if (texture->texture() == (BTexture::Flat | BTexture::Solid)) {
    menu.sel_pixmap = None;
  } else {
    int hw = menu.item_h / 2;
    menu.sel_pixmap =
      image_ctrl->renderImage(hw, hw, *texture);
  }
  if (tmp) image_ctrl->removeImage(tmp);

  XResizeWindow(display, menu.window, menu.width, menu.height);

  if (title_vis)
    XResizeWindow(display, menu.title, menu.width, menu.title_h);

  XMoveResizeWindow(display, menu.frame, 0,
                    ((title_vis) ? menu.title_h +
                     screen->getBorderWidth() : 0), menu.width,
                    menu.frame_h);

  XClearWindow(display, menu.window);
  XClearWindow(display, menu.title);
  XClearWindow(display, menu.frame);

  if (title_vis && visible) redrawTitle();

  const int menu_size = menuitems.size();
  for (int i = 0; visible && i < menu_size; i++) {
    if (i == which_sub) {
      drawItem(i, True, 0);
      drawSubmenu(i);
    } else {
      drawItem(i, False, 0);
    }
  }

  if (parent && visible)
    parent->drawSubmenu(parent->which_sub);

  XMapSubwindows(display, menu.window);
}


void Basemenu::show(void) {
  XMapSubwindows(display, menu.window);
  XMapWindow(display, menu.window);
  visible = True;

  if (! parent) {
    if (shown && (! shown->torn))
      shown->hide();

    shown = this;
  }
}


void Basemenu::hide(void) {
  if ((! torn) && hide_tree && parent && parent->isVisible()) {
    Basemenu *p = parent;

    while (p->isVisible() && (! p->torn) && p->parent) p = p->parent;
    p->internal_hide();
  } else {
    internal_hide();
  }
}


void Basemenu::internal_hide(void) {
  BasemenuItem *tmp = find(which_sub);
  if (tmp)
    tmp->submenu()->internal_hide();

  if (parent && (! torn)) {
    parent->drawItem(parent->which_sub, False, True);

    parent->which_sub = -1;
  } else if (shown && shown->menu.window == menu.window) {
    shown = (Basemenu *) 0;
  }

  torn = visible = False;
  which_sub = which_press = which_sub = -1;

  XUnmapWindow(display, menu.window);
}


void Basemenu::move(int x, int y) {
  menu.x = x;
  menu.y = y;
  XMoveWindow(display, menu.window, x, y);
  if (which_sub != -1)
    drawSubmenu(which_sub);
}


void Basemenu::redrawTitle(void) {
  const char *text = (! menu.label.empty()) ? getLabel() :
    i18n(BasemenuSet, BasemenuBlackboxMenu, "Blackbox Menu");
  int dx = menu.bevel_w, len = strlen(text);
  unsigned int l;

  if (i18n.multibyte()) {
    XRectangle ink, logical;
    XmbTextExtents(screen->getMenuStyle()->t_fontset, text, len,
                   &ink, &logical);
    l = logical.width;
  } else {
    l = XTextWidth(screen->getMenuStyle()->t_font, text, len);
  }

  l +=  (menu.bevel_w * 2);

  switch (screen->getMenuStyle()->t_justify) {
  case RightJustify:
    dx += menu.width - l;
    break;

  case CenterJustify:
    dx += (menu.width - l) / 2;
    break;

  case LeftJustify:
  default:
    break;
  }

  MenuStyle *style = screen->getMenuStyle();
  BPen pen(style->t_text, style->t_font);
  if (i18n.multibyte())
    XmbDrawString(display, menu.title, style->t_fontset, pen.gc(), dx,
                  (menu.bevel_w - style->t_fontset_extents->max_ink_extent.y),
                  text, len);
  else
    XDrawString(display, menu.title, pen.gc(), dx,
                (style->t_font->ascent + menu.bevel_w), text, len);
}


void Basemenu::drawSubmenu(int index) {
  BasemenuItem *item = find(which_sub);
  if (item && item->submenu() && ! item->submenu()->isTorn() &&
      which_sub != index)
    item->submenu()->internal_hide();

  item = find(index);
  if (! item)
    return;
  Basemenu *submenu = item->submenu();

  if (submenu && visible && ! submenu->isTorn() && item->isEnabled()) {
    if (submenu->parent != this) submenu->parent = this;
    int sbl = index / menu.persub, i = index - (sbl * menu.persub),
      x = menu.x + ((menu.item_w * (sbl + 1)) + screen->getBorderWidth()), y;

    if (alignment == AlignTop) {
      y = (((shifted) ? menu.y_shift : menu.y) +
           ((title_vis) ? menu.title_h + screen->getBorderWidth() : 0) -
           ((submenu->title_vis) ?
            submenu->menu.title_h + screen->getBorderWidth() : 0));
    } else {
      y = (((shifted) ? menu.y_shift : menu.y) +
           (menu.item_h * i) +
           ((title_vis) ? menu.title_h + screen->getBorderWidth() : 0) -
           ((submenu->title_vis) ?
            submenu->menu.title_h + screen->getBorderWidth() : 0));
    }

    if (alignment == AlignBottom &&
        (y + submenu->menu.height) > ((shifted) ? menu.y_shift :
                                              menu.y) + menu.height)
      y = (((shifted) ? menu.y_shift : menu.y) +
           menu.height - submenu->menu.height);

    if ((x + submenu->getWidth()) > screen->getWidth())
      x = ((shifted) ? menu.x_shift : menu.x) -
        submenu->getWidth() - screen->getBorderWidth();

    if (x < 0) x = 0;

    if ((y + submenu->getHeight()) > screen->getHeight())
      y = screen->getHeight() - submenu->getHeight() -
        (screen->getBorderWidth() * 2);
    if (y < 0) y = 0;

    submenu->move(x, y);
    if (! moving) drawItem(index, True);

    if (! submenu->isVisible())
      submenu->show();
    submenu->moving = moving;
    which_sub = index;
  } else {
    which_sub = -1;
  }
}


bool Basemenu::hasSubmenu(int index) {
  BasemenuItem *item = find(index);
  if (item && item->submenu())
    return True;
  return False;
}


void Basemenu::drawItem(int index, bool highlight, bool clear,
                        int x, int y, unsigned int w, unsigned int h)
{
  BasemenuItem *item = find(index);
  if (! item) return;

  bool dotext = True, dohilite = True, dosel = True;
  const char *text = item->label();
  int sbl = index / menu.persub, i = index - (sbl * menu.persub);
  int item_x = (sbl * menu.item_w), item_y = (i * menu.item_h);
  int hilite_x = item_x, hilite_y = item_y, hoff_x = 0, hoff_y = 0;
  int text_x = 0, text_y = 0, len = strlen(text), sel_x = 0, sel_y = 0;
  unsigned int hilite_w = menu.item_w, hilite_h = menu.item_h, text_w = 0,
    text_h = 0;
  unsigned int half_w = menu.item_h / 2, quarter_w = menu.item_h / 4;

  if (text) {
    if (i18n.multibyte()) {
      XRectangle ink, logical;
      XmbTextExtents(screen->getMenuStyle()->f_fontset,
                     text, len, &ink, &logical);
      text_w = logical.width;
      text_y = item_y + (menu.bevel_w / 2) -
        screen->getMenuStyle()->f_fontset_extents->max_ink_extent.y;
    } else {
      text_w = XTextWidth(screen->getMenuStyle()->f_font, text, len);
      text_y =  item_y +
        screen->getMenuStyle()->f_font->ascent +
        (menu.bevel_w / 2);
    }

    switch(screen->getMenuStyle()->f_justify) {
    case LeftJustify:
      text_x = item_x + menu.bevel_w + menu.item_h + 1;
      break;

    case RightJustify:
      text_x = item_x + menu.item_w - (menu.item_h + menu.bevel_w + text_w);
      break;

    case CenterJustify:
      text_x = item_x + ((menu.item_w + 1 - text_w) / 2);
      break;
    }

    text_h = menu.item_h - menu.bevel_w;
  }

  MenuStyle *style = screen->getMenuStyle();
  BPen pen((highlight || item->isSelected()) ? style->h_text : style->f_text),
      textpen((highlight) ? style->h_text :
              item->isEnabled() ? style->f_text : style->d_text, style->f_font),
      hipen(style->hilite.color());


  sel_x = item_x;
  if (screen->getMenuStyle()->bullet_pos == Right)
    sel_x += (menu.item_w - menu.item_h - menu.bevel_w);
  sel_x += quarter_w;
  sel_y = item_y + quarter_w;

  if (clear) {
    XClearArea(display, menu.frame, item_x, item_y, menu.item_w, menu.item_h,
               False);
  } else if (! (x == y && y == -1 && w == h && h == 0)) {
    // calculate the which part of the hilite to redraw
    if (! (max(item_x, x) <= min<signed>(item_x + menu.item_w, x + w) &&
           max(item_y, y) <= min<signed>(item_y + menu.item_h, y + h))) {
      dohilite = False;
    } else {
      hilite_x = max(item_x, x);
      hilite_y = max(item_y, y);
      hilite_w = min(item_x + menu.item_w, x + w) - hilite_x;
      hilite_h = min(item_y + menu.item_h, y + h) - hilite_y;
      hoff_x = hilite_x % menu.item_w;
      hoff_y = hilite_y % menu.item_h;
    }

    // check if we need to redraw the text
    int text_ry = item_y + (menu.bevel_w / 2);
    if (! (max(text_x, x) <= min<signed>(text_x + text_w, x + w) &&
           max(text_ry, y) <= min<signed>(text_ry + text_h, y + h)))
      dotext = False;

    // check if we need to redraw the select pixmap/menu bullet
    if (! (max(sel_x, x) <= min<signed>(sel_x + half_w, x + w) &&
           max(sel_y, y) <= min<signed>(sel_y + half_w, y + h)))
      dosel = False;
  }

  if (dohilite && highlight && (menu.hilite_pixmap != ParentRelative)) {
    if (menu.hilite_pixmap)
      XCopyArea(display, menu.hilite_pixmap, menu.frame,
                hipen.gc(), hoff_x, hoff_y,
                hilite_w, hilite_h, hilite_x, hilite_y);
    else
      XFillRectangle(display, menu.frame, hipen.gc(),
                     hilite_x, hilite_y, hilite_w, hilite_h);
  } else if (dosel && item->isSelected() &&
             (menu.sel_pixmap != ParentRelative)) {
    if (menu.sel_pixmap)
      XCopyArea(display, menu.sel_pixmap, menu.frame, hipen.gc(), 0, 0,
                half_w, half_w, sel_x, sel_y);
    else
      XFillRectangle(display, menu.frame, hipen.gc(), sel_x, sel_y, half_w, half_w);
  }

  if (dotext && text) {
    if (i18n.multibyte())
      XmbDrawString(display, menu.frame, screen->getMenuStyle()->f_fontset,
                    textpen.gc(), text_x, text_y, text, len);
    else
      XDrawString(display, menu.frame, textpen.gc(), text_x, text_y, text, len);
  }

  if (dosel && item->submenu()) {
    switch (screen->getMenuStyle()->bullet) {
    case Square:
      XDrawRectangle(display, menu.frame, pen.gc(), sel_x, sel_y, half_w, half_w);
      break;

    case Triangle:
      XPoint tri[3];

      if (screen->getMenuStyle()->bullet_pos == Right) {
        tri[0].x = sel_x + quarter_w - 2;
        tri[0].y = sel_y + quarter_w - 2;
        tri[1].x = 4;
        tri[1].y = 2;
        tri[2].x = -4;
        tri[2].y = 2;
      } else {
        tri[0].x = sel_x + quarter_w - 2;
        tri[0].y = item_y + half_w;
        tri[1].x = 4;
        tri[1].y = 2;
        tri[2].x = 0;
        tri[2].y = -4;
      }

      XFillPolygon(display, menu.frame, pen.gc(), tri, 3, Convex,
                   CoordModePrevious);
      break;

    case Diamond:
      XPoint dia[4];

      dia[0].x = sel_x + quarter_w - 3;
      dia[0].y = item_y + half_w;
      dia[1].x = 3;
      dia[1].y = -3;
      dia[2].x = 3;
      dia[2].y = 3;
      dia[3].x = -3;
      dia[3].y = 3;

      XFillPolygon(display, menu.frame, pen.gc(), dia, 4, Convex,
                   CoordModePrevious);
      break;
    }
  }
}


void Basemenu::setLabel(const string& label) {
  menu.label = label;
}


void Basemenu::setItemSelected(int index, bool sel) {
  assert(index >= 0);
  BasemenuItem *item = find(index);
  if (! item) return;

  item->setSelected(sel);
  if (visible) drawItem(index, (index == which_sub), True);
}


bool Basemenu::isItemSelected(int index) {
  assert(index >= 0);
  BasemenuItem *item = find(index);
  if (! item) return False;

  return item->isSelected();
}


void Basemenu::setItemEnabled(int index, bool enable) {
  assert(index >= 0);
  BasemenuItem *item = find(index);
  if (! item) return;

  item->setEnabled(enable);
  if (visible) drawItem(index, (index == which_sub), True);
}


bool Basemenu::isItemEnabled(int index) {
  assert(index >= 0);
  BasemenuItem *item = find(index);
  if (! item) return False;

  return item->isEnabled();
}


void Basemenu::buttonPressEvent(XButtonEvent *be) {
  if (be->window == menu.frame) {
    int sbl = (be->x / menu.item_w), i = (be->y / menu.item_h);
    int w = (sbl * menu.persub) + i;

    BasemenuItem *item = find(w);
    if (item) {
      which_press = i;
      which_sbl = sbl;


      if (item->submenu())
        drawSubmenu(w);
      else
        drawItem(w, (item->isEnabled()), True);
    }
  } else {
    menu.x_move = be->x_root - menu.x;
    menu.y_move = be->y_root - menu.y;
  }
}


void Basemenu::buttonReleaseEvent(XButtonEvent *re) {
  if (re->window == menu.title) {
    if (moving) {
      moving = False;

      if (which_sub != -1)
        drawSubmenu(which_sub);
    }

    if (re->x >= 0 && re->x <= static_cast<signed>(menu.width) &&
        re->y >= 0 && re->y <= static_cast<signed>(menu.title_h))
      if (re->button == 3)
        hide();
  } else if (re->window == menu.frame &&
             re->x >= 0 && re->x < static_cast<signed>(menu.width) &&
             re->y >= 0 && re->y < static_cast<signed>(menu.frame_h)) {
    if (re->button == 3) {
      hide();
    } else {
      int sbl = (re->x / menu.item_w), i = (re->y / menu.item_h),
        ix = sbl * menu.item_w, iy = i * menu.item_h,
        w = (sbl * menu.persub) + i,
        p = (which_sbl * menu.persub) + which_press;

      if (w >= 0 && w < static_cast<signed>(menuitems.size())) {
        drawItem(p, (p == which_sub), True);

        if  (p == w && isItemEnabled(w)) {
          if (re->x > ix && re->x < static_cast<signed>(ix + menu.item_w) &&
              re->y > iy && re->y < static_cast<signed>(iy + menu.item_h)) {
            itemSelected(re->button, w);
          }
        }
      } else {
        drawItem(p, False, True);
      }
    }
  }
}


void Basemenu::motionNotifyEvent(XMotionEvent *me) {
  if (me->window == menu.title && (me->state & Button1Mask)) {
    if (movable) {
      if (! moving) {
        if (parent && (! torn)) {
          parent->drawItem(parent->which_sub, False, True);
          parent->which_sub = -1;
        }

        moving = torn = True;

        if (which_sub != -1)
          drawSubmenu(which_sub);
      } else {
        menu.x = me->x_root - menu.x_move,
          menu.y = me->y_root - menu.y_move;

        XMoveWindow(display, menu.window, menu.x, menu.y);

        if (which_sub != -1)
          drawSubmenu(which_sub);
      }
    }
  } else if ((! (me->state & Button1Mask)) && me->window == menu.frame &&
             me->x >= 0 && me->x < static_cast<signed>(menu.width) &&
             me->y >= 0 && me->y < static_cast<signed>(menu.frame_h)) {
    int sbl = (me->x / menu.item_w), i = (me->y / menu.item_h),
      w = (sbl * menu.persub) + i;

    if ((i != which_press || sbl != which_sbl) &&
        (w >= 0 && w < static_cast<signed>(menuitems.size()))) {
      if (which_press != -1 && which_sbl != -1) {
        int p = (which_sbl * menu.persub) + which_press;
        BasemenuItem *item = find(p);

        drawItem(p, False, True);
        if (item->submenu())
          if (item->submenu()->isVisible() &&
              (! item->submenu()->isTorn())) {
            item->submenu()->internal_hide();
            which_sub = -1;
          }
      }

      which_press = i;
      which_sbl = sbl;

      BasemenuItem *itmp = find(w);

      if (itmp->submenu())
        drawSubmenu(w);
      else
        drawItem(w, (itmp->isEnabled()), True);
    }
  }
}


void Basemenu::exposeEvent(XExposeEvent *ee) {
  if (ee->window == menu.title) {
    redrawTitle();
  } else if (ee->window == menu.frame) {
    // this is a compilicated algorithm... lets do it step by step...
    // first... we see in which sub level the expose starts... and how many
    // items down in that sublevel

    int sbl = (ee->x / menu.item_w), id = (ee->y / menu.item_h),
      // next... figure out how many sublevels over the redraw spans
      sbl_d = ((ee->x + ee->width) / menu.item_w),
      // then we see how many items down to redraw
      id_d = ((ee->y + ee->height) / menu.item_h);

    if (id_d > menu.persub) id_d = menu.persub;

    // draw the sublevels and the number of items the exposure spans
    MenuItems::iterator it,
      end = menuitems.end();
    int i, ii;
    for (i = sbl; i <= sbl_d; i++) {
      // set the iterator to the first item in the sublevel needing redrawing
      it = menuitems.begin() + (id + (i * menu.persub));
      for (ii = id; ii <= id_d && it != end; ++it, ii++) {
        int index = ii + (i * menu.persub);
        // redraw the item
        drawItem(index, (which_sub == index), False,
                 ee->x, ee->y, ee->width, ee->height);
      }
    }
  }
}


void Basemenu::enterNotifyEvent(XCrossingEvent *ce) {
  if (ce->window == menu.frame) {
    menu.x_shift = menu.x, menu.y_shift = menu.y;
    if (menu.x + menu.width > screen->getWidth()) {
      menu.x_shift = screen->getWidth() - menu.width -
        screen->getBorderWidth();
      shifted = True;
    } else if (menu.x < 0) {
      menu.x_shift = -screen->getBorderWidth();
      shifted = True;
    }

    if (menu.y + menu.height > screen->getHeight()) {
      menu.y_shift = screen->getHeight() - menu.height -
        screen->getBorderWidth();
      shifted = True;
    } else if (menu.y + static_cast<signed>(menu.title_h) < 0) {
      menu.y_shift = -screen->getBorderWidth();
      shifted = True;
    }

    if (shifted)
      XMoveWindow(display, menu.window, menu.x_shift, menu.y_shift);

    if (which_sub != -1) {
      BasemenuItem *tmp = find(which_sub);
      if (tmp->submenu()->isVisible()) {
        int sbl = (ce->x / menu.item_w), i = (ce->y / menu.item_h),
          w = (sbl * menu.persub) + i;

        if (w != which_sub && (! tmp->submenu()->isTorn())) {
          tmp->submenu()->internal_hide();

          drawItem(which_sub, False, True);
          which_sub = -1;
        }
      }
    }
  }
}


void Basemenu::leaveNotifyEvent(XCrossingEvent *ce) {
  if (ce->window == menu.frame) {
    if (which_press != -1 && which_sbl != -1 && menuitems.size() > 0) {
      int p = (which_sbl * menu.persub) + which_press;

      drawItem(p, (p == which_sub), True);

      which_sbl = which_press = -1;
    }

    if (shifted) {
      XMoveWindow(display, menu.window, menu.x, menu.y);
      shifted = False;

      if (which_sub != -1) drawSubmenu(which_sub);
    }
  }
}


void Basemenu::reconfigure(void) {
  XSetWindowBackground(display, menu.window,
                       screen->getBorderColor()->pixel());
  XSetWindowBorder(display, menu.window,
                   screen->getBorderColor()->pixel());
  XSetWindowBorderWidth(display, menu.window, screen->getBorderWidth());

  menu.bevel_w = screen->getBevelWidth();
  update();
}


void Basemenu::changeItemLabel(unsigned int index, const string& label) {
  BasemenuItem *item = find(index);
  assert(item);
  item->newLabel(label);
}
