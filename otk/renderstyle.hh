// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __renderstyle_hh
#define __renderstyle_hh

#include "rendertexture.hh"

namespace otk {

class RenderStyle {
public:
  enum TextJustify {
    LeftJustify,
    RightJustify,
    CenterJustify
  };

private:
  int _screen;
  
  RenderColor *_text_focus_color;
  RenderColor *_text_unfocus_color;

  RenderColor *_frame_border_color;
  int _frame_border_wirth;
  RenderColor *_client_border_color_focus; 
  RenderColor *_client_border_color_unfocus;
  int _client_border_width;
 
  RenderTexture *_titlebar_focus;
  RenderTexture *_titlebar_unfocus;

  RenderTexture *_label_focus;
  RenderTexture *_label_unfocus;

  RenderTexture *_handle_focus;
  RenderTexture *_handle_unfocus;

  RenderTexture *_button_unpress_focus;
  RenderTexture *_button_unpress_unfocus;
  RenderTexture *_button_press_focus;
  RenderTexture *_button_press_unfocus;

  RenderTexture *_grip_focus;
  RenderTexture *_grip_unfocus;

  Font *_label_font;
  TextJustify _label_justify;

  int _handle_width;
  int _bevel_width;
};

}

#endif // __rendertexture_hh
