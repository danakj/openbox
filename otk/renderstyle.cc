// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "renderstyle.hh"
#include "display.hh"
#include "screeninfo.hh"

namespace otk {

RenderStyle::RenderStyle(int screen, const std::string &stylefile)
  : _screen(screen),
    _file(stylefile)
{
// pick one..
#define FIERON
//#define MERRY

#ifdef FIERON
  _root_color = new RenderColor(_screen, 0x272a2f);
  
  _text_color_focus = new RenderColor(_screen, 0x272a2f);
  _text_color_unfocus = new RenderColor(_screen, 0x676869);

  _button_color_focus = new RenderColor(_screen, 0x96ba86);
  _button_color_unfocus = new RenderColor(_screen, 0x676869);

  _frame_border_color = new RenderColor(_screen, 0x181f24);
  _frame_border_width = 1;

  _client_border_color_focus = new RenderColor(_screen, 0x858687);
  _client_border_color_unfocus = new RenderColor(_screen, 0x555657);
  _client_border_width = 1;

  _titlebar_focus = new RenderTexture(_screen,
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
  _titlebar_unfocus = new RenderTexture(_screen,
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

  _label_focus = new RenderTexture(_screen,
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
  _label_unfocus = new RenderTexture(_screen,
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


  _handle_focus = new RenderTexture(_screen,
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
  _handle_unfocus = new RenderTexture(_screen,
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

  
  _button_unpress_focus = new RenderTexture(_screen,
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
  _button_unpress_unfocus = new RenderTexture(_screen,
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

  _button_press_focus = new RenderTexture(_screen,
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
  _button_press_unfocus = new RenderTexture(_screen,
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

  _grip_focus = new RenderTexture(_screen,
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
  _grip_unfocus = new RenderTexture(_screen,
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

  _label_font = new Font(_screen, "Arial,Sans-9:bold", true, 1, 0x40);
  _label_justify = RightJustify;

  _max_mask = new PixmapMask();
  _max_mask->w = _max_mask->h = 8;
  {
    //char data[] = { 0x7e, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0x7e };
    char data []  = {0x00, 0x00, 0x18, 0x3c, 0x66, 0x42, 0x00, 0x00 };
    _max_mask->mask =
      XCreateBitmapFromData(**display,
                            display->screenInfo(_screen)->rootWindow(),
                            data, 8, 8);
  }

  _icon_mask = new PixmapMask();
  _icon_mask->w = _icon_mask->h = 8;
  {
    //char data[] = { 0x00, 0x00, 0xc3, 0xe7, 0x7e, 0x3c, 0x18, 0x00 };
    char data[] = { 0x00, 0x00, 0x42, 0x66, 0x3c, 0x18, 0x00, 0x00 };
    _icon_mask->mask =
      XCreateBitmapFromData(**display,
                            display->screenInfo(_screen)->rootWindow(),
                            data, 8, 8);
  }
  
  _alldesk_mask = new PixmapMask();
  _alldesk_mask->w = _alldesk_mask->h = 8;
  {
    //char data[] = { 0x00, 0x00, 0x18, 0x3c, 0x3c, 0x18, 0x00, 0x00 };
    char data[] = { 0x00, 0x66, 0x66, 0x00, 0x00, 0x66, 0x66, 0x00 };
    _alldesk_mask->mask =
      XCreateBitmapFromData(**display,
                            display->screenInfo(_screen)->rootWindow(),
                            data, 8, 8);
  }
  
  _close_mask = new PixmapMask();
  _close_mask->w = _close_mask->h = 8;
  {
    //char data[] = { 0xc3, 0xe7, 0x7e, 0x3c, 0x3c, 0x7e, 0xe7, 0xc3 };
    char data[] = { 0x00, 0xc3, 0x66, 0x3c, 0x3c, 0x66, 0xc3, 0x00 };
    _close_mask->mask =
      XCreateBitmapFromData(**display,
                            display->screenInfo(_screen)->rootWindow(),
                            data, 8, 8);
  }

  _bevel_width = 1;
  _handle_width = 4;
#else
#  ifdef MERRY
  _root_color = new RenderColor(_screen, 0x7b756a);
  
  _text_color_focus = new RenderColor(_screen, 0xffffff);
  _text_color_unfocus = new RenderColor(_screen, 0xffffff);

  _button_color_focus = new RenderColor(_screen, 0x222222);
  _button_color_unfocus = new RenderColor(_screen, 0x333333);

  _frame_border_color = new RenderColor(_screen, 0x222222);
  _frame_border_width = 1;

  _client_border_color_focus = new RenderColor(_screen, 0x858687);
  _client_border_color_unfocus = new RenderColor(_screen, 0x555657);
  _client_border_width = 0;

  _titlebar_focus = new RenderTexture(_screen,
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
  _titlebar_unfocus = new RenderTexture(_screen,
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

  _label_focus = new RenderTexture(_screen,
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
_label_unfocus = new RenderTexture(_screen,
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


  _handle_focus = new RenderTexture(_screen,
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
  _handle_unfocus = new RenderTexture(_screen,
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

  
  _button_unpress_focus = new RenderTexture(_screen,
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
  _button_unpress_unfocus = new RenderTexture(_screen,
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

  _button_press_focus = new RenderTexture(_screen,
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
  _button_press_unfocus = new RenderTexture(_screen,
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

  _grip_focus = new RenderTexture(_screen,
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
  _grip_unfocus = new RenderTexture(_screen,
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

  _label_font = new Font(_screen, "Arial,Sans-8", true, 1, 0x3e);
  _label_justify = CenterJustify;

  _max_mask = new PixmapMask();
  _max_mask->w = _max_mask->h = 7;
  {
    char data []  = {0x7c, 0x44, 0x47, 0x47, 0x7f, 0x1f, 0x1f  };
    _max_mask->mask =
      XCreateBitmapFromData(**display,
                            display->screenInfo(_screen)->rootWindow(),
                            data, 7, 7);
  }

  _icon_mask = new PixmapMask();
  _icon_mask->w = _icon_mask->h = 7;
  {
    char data[] = {0x00, 0x00, 0x00, 0x00, 0x3e, 0x3e, 0x3e };
    _icon_mask->mask =
      XCreateBitmapFromData(**display,
                            display->screenInfo(_screen)->rootWindow(),
                            data, 7, 7);
  }
  
  _alldesk_mask = new PixmapMask();
  _alldesk_mask->w = _alldesk_mask->h = 7;
  {
    char data[] = {0x00, 0x36, 0x36, 0x00, 0x36, 0x36, 0x00 };
    _alldesk_mask->mask =
      XCreateBitmapFromData(**display,
                            display->screenInfo(_screen)->rootWindow(),
                            data, 7, 7);
  }
  
  _close_mask = new PixmapMask();
  _close_mask->w = _close_mask->h = 7;
  {
    char data[] = {  0x22, 0x77, 0x3e, 0x1c, 0x3e, 0x77, 0x22 };
    _close_mask->mask =
      XCreateBitmapFromData(**display,
                            display->screenInfo(_screen)->rootWindow(),
                            data, 7, 7);
  }

  _bevel_width = 1;
  _handle_width = 3;
#  else
#    error 1
#  endif
#endif
}

RenderStyle::~RenderStyle()
{
  delete _root_color;
  
  delete _text_color_focus;
  delete _text_color_unfocus;

  delete _button_color_focus;
  delete _button_color_unfocus;

  delete _frame_border_color;

  delete _client_border_color_focus; 
  delete _client_border_color_unfocus;
 
  delete _titlebar_focus;
  delete _titlebar_unfocus;

  delete _label_focus;
  delete _label_unfocus;

  delete _handle_focus;
  delete _handle_unfocus;

  delete _button_unpress_focus;
  delete _button_unpress_unfocus;
  delete _button_press_focus;
  delete _button_press_unfocus;

  delete _grip_focus;
  delete _grip_unfocus;

  delete _label_font;

  delete _max_mask;
  delete _icon_mask;
  delete _alldesk_mask;
  delete _close_mask;
}

}
