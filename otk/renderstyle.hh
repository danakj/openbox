// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __renderstyle_hh
#define __renderstyle_hh

#include "rendertexture.hh"
#include "rendercolor.hh"
#include "font.hh"

#include <string>

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
  std::string _file;
  
  RenderColor *_text_color_focus;
  RenderColor *_text_color_unfocus;

  RenderColor *_button_color_focus;
  RenderColor *_button_color_unfocus;

  RenderColor *_frame_border_color;
  int _frame_border_width;

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

public:
  RenderStyle(int screen, const std::string &stylefile);
  virtual ~RenderStyle();

  inline RenderColor *textFocusColor() const { return _text_color_focus; }
  inline RenderColor *textUnfocusColor() const { return _text_color_unfocus; }

  inline RenderColor *buttonFocusColor() const { return _button_color_focus; }
  inline RenderColor *buttonUnfocusColor() const
    { return _button_color_unfocus; }

  inline RenderColor *frameBorderColor() const { return _frame_border_color; }
  inline int frameBorderWidth() const { return _frame_border_width; }

  inline RenderColor *clientBorderFocusColor() const
    { return _client_border_color_focus; }
  inline RenderColor *clientBorderUnfocusColor() const
    { return _client_border_color_unfocus; }
  inline int clientBorderWidth() const { return _client_border_width; }
 
  inline RenderTexture *titlebarFocusBackground() const
    { return _titlebar_focus; }
  inline RenderTexture *titlebarUnfocusBackground() const
    { return _titlebar_unfocus; }

  inline RenderTexture *labelFocusBackground() const { return _label_focus; }
  inline RenderTexture *labelUnfocusBackground() const { return _label_unfocus;}

  inline RenderTexture *handleFocusBackground() const { return _handle_focus; }
  inline RenderTexture *handleUnfocusBackground() const
    { return _handle_unfocus; }

  inline RenderTexture *buttonUnpressFocusBackground() const
    { return _button_unpress_focus; }
  inline RenderTexture *buttonUnpressUnfocusBackground() const
    { return _button_unpress_unfocus; }
  inline RenderTexture *buttonPressFocusBackground() const
    { return _button_press_focus; }
  inline RenderTexture *buttonPressUnfocusBackgrounf() const
    { return _button_press_unfocus; }

  inline RenderTexture *gripdFocusBackground() const { return _grip_focus; }
  inline RenderTexture *gripUnfocusBackground() const { return _grip_unfocus; }

  inline Font *labelFont() const { return _label_font; }
  inline TextJustify labelTextJustify() const { return _label_justify; }

  inline int handleWidth() const { return _handle_width; }
  inline int bevelWidth() const { return _bevel_width; }
};

}

#endif // __rendertexture_hh
