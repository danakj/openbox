// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "renderstyle.hh"
#include "display.hh"
#include "screeninfo.hh"

#include <cassert>

namespace otk {

RenderStyle **RenderStyle::_styles = 0;
std::list<StyleNotify*> *RenderStyle::_notifies = 0;

void RenderStyle::initialize()
{
  int screens = ScreenCount(**display);
  _styles = new RenderStyle*[screens];
  for (int i = 0; i < screens; ++i) {
    _styles[i] = new RenderStyle();
    defaultStyle(_styles[i], i);
    }
  _notifies = new std::list<StyleNotify*>[screens];
}

void RenderStyle::destroy()
{
  int screens = ScreenCount(**display);
  for (int i = 0; i < screens; ++i)
    delete _styles[i];
  delete [] _styles;
  delete [] _notifies;
}

void RenderStyle::registerNotify(int screen, StyleNotify *n)
{
  assert(screen >= 0 && screen < ScreenCount(**display));
  _notifies[screen].push_back(n);
}

void RenderStyle::unregisterNotify(int screen, StyleNotify *n)
{
  assert(screen >= 0 && screen < ScreenCount(**display));
  _notifies[screen].remove(n);
}

RenderStyle *RenderStyle::style(int screen)
{
  assert(screen >= 0 && screen < ScreenCount(**display));
  return _styles[screen];
}

bool RenderStyle::setStyle(int screen, const ustring &stylefile)
{
  RenderStyle *s = new RenderStyle();
  if (!loadStyle(s, screen, stylefile)) {
    delete s;
    return false;
  }
  delete _styles[screen];
  _styles[screen] = s;

  std::list<StyleNotify*>::iterator it, end = _notifies[screen].end();
  for (it = _notifies[screen].begin(); it != end; ++it)
    (*it)->styleChanged(*s);
  return true;
}

bool RenderStyle::loadStyle(RenderStyle *s, int screen,
                            const ustring &stylefile)
{
  s->_screen = screen;
  s->_file = stylefile;
// pick one..
//#define FIERON
#define MERRY

#ifdef FIERON
  s->_root_color = new RenderColor(screen, 0x272a2f);
  
  s->_text_color_focus = new RenderColor(screen, 0x272a2f);
  s->_text_color_unfocus = new RenderColor(screen, 0x676869);

  s->_button_color_focus = new RenderColor(screen, 0x96ba86);
  s->_button_color_unfocus = new RenderColor(screen, 0x676869);

  s->_frame_border_color = new RenderColor(screen, 0x181f24);
  s->_frame_border_width = 1;

  s->_client_border_color_focus = new RenderColor(screen, 0x858687);
  s->_client_border_color_unfocus = new RenderColor(screen, 0x555657);
  s->_client_border_width = 1;

  s->_titlebar_focus = new RenderTexture(screen,
                                         false,
                                         RenderTexture::Flat,
                                         RenderTexture::Bevel1,
                                         false,
                                         RenderTexture::Vertical,
                                         false,
                                         0x858687,
                                         0x373a3f,
                                         0x0,
                                         0x0);
  s->_titlebar_unfocus = new RenderTexture(screen,
                                           false,
                                           RenderTexture::Flat,
                                           RenderTexture::Bevel1,
                                           false,
                                           RenderTexture::Vertical,
                                        false,
                                           0x555657,
                                           0x171a1f,
                                           0x0,
                                           0x0);

  s->_label_focus = new RenderTexture(screen,
                                      false,
                                      RenderTexture::Flat,
                                      RenderTexture::Bevel1,
                                      true,
                                      RenderTexture::Vertical,
                                      false,
                                      0x96ba86,
                                      0x5a724c,
                                      0x181f24,
                                      0x0);
  s->_label_unfocus = new RenderTexture(screen,
                                        false,
                                        RenderTexture::Sunken,
                                        RenderTexture::Bevel1,
                                        false,
                                        RenderTexture::CrossDiagonal,
                                        false,
                                        0x555657,
                                        0x272a2f,
                                        0x0,
                                        0x0);

  s->_handle_focus = new RenderTexture(screen,
                                       false,
                                       RenderTexture::Flat,
                                       RenderTexture::Bevel1,
                                       false,
                                       RenderTexture::Vertical,
                                       false,
                                       0x858687,
                                       0x373a3f,
                                       0x0,
                                       0x0);
  s->_handle_unfocus = new RenderTexture(screen,
                                         false,
                                         RenderTexture::Flat,
                                         RenderTexture::Bevel1,
                                         false,
                                         RenderTexture::Vertical,
                                         false,
                                         0x555657,
                                         0x171a1f,
                                         0x0,
                                         0x0);
  
  s->_button_unpress_focus = new RenderTexture(screen,
                                               false,
                                               RenderTexture::Raised,
                                               RenderTexture::Bevel2,
                                               false,
                                               RenderTexture::CrossDiagonal,
                                               false,
                                               0x858687,
                                               0x272a2f,
                                               0x0,
                                               0x0);
  s->_button_unpress_unfocus = new RenderTexture(screen,
                                                 false,
                                                 RenderTexture::Raised,
                                                 RenderTexture::Bevel2,
                                                 false,
                                                 RenderTexture::CrossDiagonal,
                                                 false,
                                                 0x555657,
                                                 0x171a1f,
                                                 0x0,
                                                 0x0);

  s->_button_press_focus = new RenderTexture(screen,
                                             false,
                                             RenderTexture::Sunken,
                                             RenderTexture::Bevel2,
                                             false,
                                             RenderTexture::CrossDiagonal,
                                             false,
                                             0x96ba86,
                                             0x5a724c,
                                             0x0,
                                             0x0);
  s->_button_press_unfocus = new RenderTexture(screen,
                                               false,
                                               RenderTexture::Sunken,
                                               RenderTexture::Bevel2,
                                               false,
                                               RenderTexture::CrossDiagonal,
                                               false,
                                               0x555657,
                                               0x171a1f,
                                               0x0,
                                               0x0);
  
  s->_grip_focus = new RenderTexture(screen,
                                     false,
                                     RenderTexture::Flat,
                                     RenderTexture::Bevel1,
                                     false,
                                     RenderTexture::Vertical,
                                     false,
                                     0x96ba86,
                                     0x5a724c,
                                     0x0,
                                     0x0);
  s->_grip_unfocus = new RenderTexture(screen,
                                       false,
                                       RenderTexture::Flat,
                                       RenderTexture::Bevel1,
                                       false,
                                       RenderTexture::Vertical,
                                       false,
                                       0x555657,
                                       0x171a1f,
                                       0x0,
                                       0x0);

  s->_label_font = new Font(screen, "Arial,Sans-9:bold", true, 1, 0x40);
  s->_label_justify = RightBottomJustify;

  s->_max_mask = new PixmapMask();
  s->_max_mask->w = s->_max_mask->h = 8;
  {
    //char data[] = { 0x7e, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0x7e };
    char data []  = {0x00, 0x00, 0x18, 0x3c, 0x66, 0x42, 0x00, 0x00 };
    s->_max_mask->mask =
      XCreateBitmapFromData(**display,
                            display->screenInfo(screen)->rootWindow(),
                            data, 8, 8);
  }

  s->_icon_mask = new PixmapMask();
  s->_icon_mask->w = s->_icon_mask->h = 8;
  {
    //char data[] = { 0x00, 0x00, 0xc3, 0xe7, 0x7e, 0x3c, 0x18, 0x00 };
    char data[] = { 0x00, 0x00, 0x42, 0x66, 0x3c, 0x18, 0x00, 0x00 };
    s->_icon_mask->mask =
      XCreateBitmapFromData(**display,
                            display->screenInfo(screen)->rootWindow(),
                            data, 8, 8);
  }
  
  s->_alldesk_mask = new PixmapMask();
  s->_alldesk_mask->w = s->_alldesk_mask->h = 8;
  {
    //char data[] = { 0x00, 0x00, 0x18, 0x3c, 0x3c, 0x18, 0x00, 0x00 };
    char data[] = { 0x00, 0x66, 0x66, 0x00, 0x00, 0x66, 0x66, 0x00 };
    s->_alldesk_mask->mask =
      XCreateBitmapFromData(**display,
                            display->screenInfo(screen)->rootWindow(),
                            data, 8, 8);
  }
  
  s->_close_mask = new PixmapMask();
  s->_close_mask->w = s->_close_mask->h = 8;
  {
    //char data[] = { 0xc3, 0xe7, 0x7e, 0x3c, 0x3c, 0x7e, 0xe7, 0xc3 };
    char data[] = { 0x00, 0xc3, 0x66, 0x3c, 0x3c, 0x66, 0xc3, 0x00 };
    s->_close_mask->mask =
      XCreateBitmapFromData(**display,
                            display->screenInfo(screen)->rootWindow(),
                            data, 8, 8);
  }

  s->_bevel_width = 1;
  s->_handle_width = 4;
#else
#  ifdef MERRY
  s->_root_color = new RenderColor(screen, 0x7b756a);
  
  s->_text_color_focus = new RenderColor(screen, 0xffffff);
  s->_text_color_unfocus = new RenderColor(screen, 0xffffff);

  s->_button_color_focus = new RenderColor(screen, 0x222222);
  s->_button_color_unfocus = new RenderColor(screen, 0x333333);

  s->_frame_border_color = new RenderColor(screen, 0x222222);
  s->_frame_border_width = 1;

  s->_client_border_color_focus = new RenderColor(screen, 0x858687);
  s->_client_border_color_unfocus = new RenderColor(screen, 0x555657);
  s->_client_border_width = 0;

  s->_titlebar_focus = new RenderTexture(screen,
                                         false,
                                         RenderTexture::Flat,
                                         RenderTexture::Bevel1,
                                         false,
                                         RenderTexture::Solid,
                                         false,
                                         0xe6e6e6,
                                         0xe6e6e6,  
                                         0x0,
                                         0x0);
  s->_titlebar_unfocus = new RenderTexture(screen,
                                           false,
                                           RenderTexture::Flat,
                                           RenderTexture::Bevel1,
                                           false,
                                           RenderTexture::Solid,
                                           false,
                                           0xe6e6e6,
                                           0xd9d9d9,
                                           0x0,
                                           0x0);

  s->_label_focus = new RenderTexture(screen,
                                      false,
                                      RenderTexture::Flat,
                                      RenderTexture::Bevel1,
                                      true,
                                      RenderTexture::Vertical,
                                      false,
                                      //0x6a6973,
                                      //0x6a6973,
                                      0x4c59a6,
                                      0x5a6dbd,
                                      0x222222,
                                      0x0);
  //urg this ain't so hot
  s->_label_unfocus = new RenderTexture(screen,
                                        false,
                                        RenderTexture::Flat,
                                        RenderTexture::Bevel1,
                                        true,
                                        RenderTexture::Vertical,
                                        false,
                                        0xb4b2ad,
                                        0xc3c1bc,
                                        0x6a696a,
                                        0x0);


  s->_handle_focus = new RenderTexture(screen,
                                       false,
                                       RenderTexture::Flat,
                                       RenderTexture::Bevel1,
                                       false,
                                       RenderTexture::Vertical,
                                       false,
                                       0xe6e6e6,
                                       0xd9d9d9,
                                       0x0,
                                       0x0);
  s->_handle_unfocus = new RenderTexture(screen,
                                         false,
                                         RenderTexture::Flat,
                                         RenderTexture::Bevel1,
                                         false,
                                         RenderTexture::Solid,
                                         false,
                                         0xe6e6e6,
                                         0xe6e6e6,
                                         0x0,
                                         0x0);

  
  s->_button_unpress_focus = new RenderTexture(screen,
                                               false,
                                               RenderTexture::Flat,
                                               RenderTexture::Bevel1,
                                               false,
                                               RenderTexture::Solid,
                                               false,
                                               0xe6e6e6,
                                               0xe6e6e6,
                                               0x0,
                                               0x0);
  s->_button_unpress_unfocus = new RenderTexture(screen,
                                                 false,
                                                 RenderTexture::Flat,
                                                 RenderTexture::Bevel1,
                                                 false,
                                                 RenderTexture::Solid,
                                                 false,
                                                 0xe6e6e6,
                                                 0xe6e6e6,
                                                 0x0,
                                                 0x0);

  s->_button_press_focus = new RenderTexture(screen,
                                             false,
                                             RenderTexture::Sunken,
                                             RenderTexture::Bevel2,
                                             false,
                                             RenderTexture::Vertical,
                                             false,
                                             0xe6e6e6,
                                             0xe6e6e6,
                                             0x0,
                                             0x0);
  s->_button_press_unfocus = new RenderTexture(screen,
                                               false,
                                               RenderTexture::Sunken,
                                               RenderTexture::Bevel2,
                                               false,
                                               RenderTexture::Vertical,
                                               false,
                                               0xe6e6e6,
                                               0xe6e6e6,
                                               0x0,
                                               0x0);

  s->_grip_focus = new RenderTexture(screen,
                                     false,
                                     RenderTexture::Flat,
                                     RenderTexture::Bevel1,
                                     false,
                                     RenderTexture::Vertical,
                                     false,
                                     0xe6e6e6,
                                     0xd9d9d9,
                                     0x0,
                                     0x0);
  s->_grip_unfocus = new RenderTexture(screen,
                                       false,
                                       RenderTexture::Flat,
                                       RenderTexture::Bevel1,
                                       false,
                                       RenderTexture::Solid,
                                       false,
                                       0xe6e6e6,
                                       0xe6e6e6,
                                       0x0,
                                       0x0);

  s->_label_font = new Font(screen, "Arial,Sans-8", true, 1, 0x3e);
  s->_label_justify = CenterJustify;

  s->_max_mask = new PixmapMask();
  s->_max_mask->w = s->_max_mask->h = 7;
  {
    char data []  = {0x7c, 0x44, 0x47, 0x47, 0x7f, 0x1f, 0x1f  };
    s->_max_mask->mask =
      XCreateBitmapFromData(**display,
                            display->screenInfo(screen)->rootWindow(),
                            data, 7, 7);
  }

  s->_icon_mask = new PixmapMask();
  s->_icon_mask->w = s->_icon_mask->h = 7;
  {
    char data[] = {0x00, 0x00, 0x00, 0x00, 0x3e, 0x3e, 0x3e };
    s->_icon_mask->mask =
      XCreateBitmapFromData(**display,
                            display->screenInfo(screen)->rootWindow(),
                            data, 7, 7);
  }
  
  s->_alldesk_mask = new PixmapMask();
  s->_alldesk_mask->w = s->_alldesk_mask->h = 7;
  {
    char data[] = {0x00, 0x36, 0x36, 0x00, 0x36, 0x36, 0x00 };
    s->_alldesk_mask->mask =
      XCreateBitmapFromData(**display,
                            display->screenInfo(screen)->rootWindow(),
                            data, 7, 7);
  }
  
  s->_close_mask = new PixmapMask();
  s->_close_mask->w = s->_close_mask->h = 7;
  {
    char data[] = {  0x22, 0x77, 0x3e, 0x1c, 0x3e, 0x77, 0x22 };
    s->_close_mask->mask =
      XCreateBitmapFromData(**display,
                            display->screenInfo(screen)->rootWindow(),
                            data, 7, 7);
  }

  s->_bevel_width = 1;
  s->_handle_width = 3;
#  else
#    error 1
#  endif
#endif

  return true;
}

void RenderStyle::defaultStyle(RenderStyle *s, int screen)
{
  s->_screen = screen;
  s->_file = "";

  s->_root_color = new RenderColor(screen, 0);
  s->_text_color_focus = new RenderColor(screen, 0xffffff);
  s->_text_color_unfocus = new RenderColor(screen, 0xffffff);
  s->_button_color_focus = new RenderColor(screen, 0);
  s->_button_color_unfocus = new RenderColor(screen, 0);
  s->_frame_border_color = new RenderColor(screen, 0);
  s->_frame_border_width = 1;
  s->_client_border_color_focus = new RenderColor(screen, 0);
  s->_client_border_color_unfocus = new RenderColor(screen, 0);
  s->_client_border_width = 1;
  s->_titlebar_focus = new RenderTexture(screen,
                                         false,
                                         RenderTexture::Flat,
                                         RenderTexture::Bevel1,
                                         false,
                                         RenderTexture::Solid,
                                         false,
                                         0, 0, 0, 0);
  s->_titlebar_unfocus = new RenderTexture(screen,
                                           false,
                                           RenderTexture::Flat,
                                           RenderTexture::Bevel1,
                                           false,
                                           RenderTexture::Solid,
                                           false,
                                           0, 0, 0, 0);

  s->_label_focus = new RenderTexture(screen,
                                      false,
                                      RenderTexture::Flat,
                                      RenderTexture::Bevel1,
                                      true,
                                      RenderTexture::Solid,
                                      false,
                                      0, 0, 0, 0);
  s->_label_unfocus = new RenderTexture(screen,
                                        false,
                                        RenderTexture::Flat,
                                        RenderTexture::Bevel1,
                                        false,
                                        RenderTexture::Solid,
                                        false,
                                        0, 0, 0, 0);

  s->_handle_focus = new RenderTexture(screen,
                                       false,
                                       RenderTexture::Flat,
                                       RenderTexture::Bevel1,
                                       false,
                                       RenderTexture::Solid,
                                       false,
                                       0, 0, 0, 0);
  s->_handle_unfocus = new RenderTexture(screen,
                                         false,
                                         RenderTexture::Flat,
                                         RenderTexture::Bevel1,
                                         false,
                                         RenderTexture::Solid,
                                         false,
                                         0, 0, 0, 0);
  
  s->_button_unpress_focus = new RenderTexture(screen,
                                               false,
                                               RenderTexture::Flat,
                                               RenderTexture::Bevel1,
                                               false,
                                               RenderTexture::Solid,
                                               false,
                                               0, 0, 0, 0);
  s->_button_unpress_unfocus = new RenderTexture(screen,
                                                 false,
                                                 RenderTexture::Flat,
                                                 RenderTexture::Bevel1,
                                                 false,
                                                 RenderTexture::Solid,
                                                 false,
                                                 0, 0, 0, 0);

  s->_button_press_focus = new RenderTexture(screen,
                                             false,
                                             RenderTexture::Flat,
                                             RenderTexture::Bevel1,
                                             false,
                                             RenderTexture::Solid,
                                             false,
                                             0, 0, 0, 0);
  s->_button_press_unfocus = new RenderTexture(screen,
                                               false,
                                               RenderTexture::Flat,
                                               RenderTexture::Bevel1,
                                               false,
                                               RenderTexture::Solid,
                                               false,
                                               0, 0, 0, 0);
  
  s->_grip_focus = new RenderTexture(screen,
                                     false,
                                     RenderTexture::Flat,
                                     RenderTexture::Bevel1,
                                     false,
                                     RenderTexture::Solid,
                                     false,
                                     0, 0, 0, 0);
  s->_grip_unfocus = new RenderTexture(screen,
                                       false,
                                       RenderTexture::Flat,
                                       RenderTexture::Bevel1,
                                       false,
                                       RenderTexture::Solid,
                                       false,
                                       0, 0, 0, 0);

  s->_label_font = new Font(screen, "Sans-9", false, 0, 0);
  s->_label_justify = LeftTopJustify;

  s->_max_mask = new PixmapMask();
  s->_max_mask->w = s->_max_mask->h = 0;
  s->_max_mask->mask = None;

  s->_icon_mask = new PixmapMask();
  s->_icon_mask->w = s->_icon_mask->h = 0;
  s->_icon_mask->mask = None;
  
  s->_alldesk_mask = new PixmapMask();
  s->_alldesk_mask->w = s->_alldesk_mask->h = 0;
  s->_alldesk_mask->mask = 0;
  
  s->_close_mask = new PixmapMask();
  s->_close_mask->w = s->_close_mask->h = 8;
  s->_close_mask->mask = 0;

  s->_bevel_width = 1;
  s->_handle_width = 4;
}

RenderStyle::~RenderStyle()
{
  assert(_root_color);
  delete _root_color;
  
  assert(_text_color_focus);
  delete _text_color_focus;
  assert(_text_color_unfocus);
  delete _text_color_unfocus;

  assert(_button_color_focus);
  delete _button_color_focus;
  assert(_button_color_unfocus);
  delete _button_color_unfocus;

  assert(_frame_border_color);
  delete _frame_border_color;

  assert(_client_border_color_focus);
  delete _client_border_color_focus; 
  assert(_client_border_color_unfocus);
  delete _client_border_color_unfocus;
 
  assert(_titlebar_focus);
  delete _titlebar_focus;
  assert(_titlebar_unfocus);
  delete _titlebar_unfocus;

  assert(_label_focus);
  delete _label_focus;
  assert(_label_unfocus);
  delete _label_unfocus;

  assert(_handle_focus);
  delete _handle_focus;
  assert(_handle_unfocus);
  delete _handle_unfocus;

  assert(_button_unpress_focus);
  delete _button_unpress_focus;
  assert(_button_unpress_unfocus);
  delete _button_unpress_unfocus;
  assert(_button_press_focus);
  delete _button_press_focus;
  assert(_button_press_unfocus);
  delete _button_press_unfocus;

  assert(_grip_focus);
  delete _grip_focus;
  assert(_grip_unfocus);
  delete _grip_unfocus;

  assert(_label_font);
  delete _label_font;

  assert(_max_mask);
  delete _max_mask;
  assert(_icon_mask);
  delete _icon_mask;
  assert(_alldesk_mask);
  delete _alldesk_mask;
  assert(_close_mask);
  delete _close_mask;
}

}
