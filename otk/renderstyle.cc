// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "renderstyle.hh"
#include "rendercolor.hh"
#include "rendertexture.hh"

namespace otk {

RenderStyle(int screen, const std::string &stylefile)
  : _screen(screen),
    _file(stylefile)
{
  _text_focus_color = new RenderColor(_screen, 0x272a2f);
  _text_unfocus_color = new RenderColor(_screen, 0x676869);

  _frame_border_color = new RenderColor(_screen, 0x181f24);
  _frame_border_width = 1;

  _client_border_color_focus = new RenderColor(_screen, 0x858687);
  _client_border_color_unfocus = new RenderColor(_screen, 0x555657);
  _client_border_width = 1;

  _titlebar_focus = new RenderTexture(false,
                                      RenderTexture::Flat,
                                      RenderTexture::Bevel1,
                                      false,
                                      RenderTexture::Vertical,
                                      false,
                                      0x858687,
                                      0x373a3f,
                                      0x0,
                                      0x0,
                                      0x0,
                                      0x0);
  _titlebar_unfocus = new RenderTexture(false,
                                        RenderTexture::Flat,
                                        RenderTexture::Bevel1,
                                        false,
                                        RenderTexture::Vertical,
                                        false,
                                        0x555657,
                                        0x171a1f,
                                        0x0,
                                        0x0,
                                        0x0,
                                        0x0);

  _label_focus = new RenderTexture(false,
                                   RenderTexture::Flat,
                                   RenderTexture::Bevel1,
                                   true,
                                   RenderTexture::Vertical,
                                   false,
                                   0x858687,
                                   0x373a3f,
                                   0x0,
                                   0x0,
                                   0x181f24,
                                   0x0);
  _label_unfocus = new RenderTexture(false,
                                     RenderTexture::Sunken,
                                     RenderTexture::Bevel1,
                                     false,
                                     RenderTexture::CrossDiagonal,
                                     false,
                                     0x555657,
                                     0x272a2f,
                                     //XXX,
                                     //XXX,
                                     0x0,
                                     0x0);


  _handle_focus = new RenderTexture(false,
                                    RenderTexture::Flat,
                                    RenderTexture::Bevel1,
                                    true,
                                    RenderTexture::Vertical,
                                    false,
                                    0x858687,
                                    0x373a3f,
                                    0x0,
                                    0x0,
                                    0x0,
                                    0x0);
  _handle_unfocus = new RenderTexture(false,
                                      RenderTexture::Flat,
                                      RenderTexture::Bevel1,
                                      false,
                                      RenderTexture::Vertical,
                                      false,
                                      0x555657,
                                      0x171a1f,
                                      0x0,
                                      0x0,
                                      0x0,
                                      0x0);

}

virtual ~RenderStyle()
{
}

}
