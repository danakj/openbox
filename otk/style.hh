// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __style_hh
#define __style_hh

#include <string>

#include "color.hh"
#include "font.hh"
#include "texture.hh"
#include "image.hh"
#include "configuration.hh"

// XXX: document

namespace otk {

struct PixmapMask {
  Pixmap mask;
  unsigned int w, h;
  PixmapMask() { mask = None; w = h = 0; }
};

class Style {
public:

  enum Type { ButtonFocus, ButtonUnfocus, TitleFocus, TitleUnfocus,
              LabelFocus, LabelUnfocus, HandleFocus, HandleUnfocus,
              GripFocus, GripUnfocus };

  enum TextJustify { LeftJustify = 1, RightJustify, CenterJustify };
  enum BulletType { RoundBullet = 1, TriangleBullet, SquareBullet, NoBullet };

// private:

  ImageControl *image_control;

  Color
    l_text_focus, l_text_unfocus,
    b_pic_focus, b_pic_unfocus;
  
  Color border_color;

  Font *font;

  Texture
    f_focus, f_unfocus,
    t_focus, t_unfocus,
    l_focus, l_unfocus,
    h_focus, h_unfocus,
    b_focus, b_unfocus,
    b_pressed_focus, b_pressed_unfocus,
    g_focus, g_unfocus;

  PixmapMask close_button, max_button, icon_button, stick_button;
  TextJustify justify;
  BulletType bullet_type;

  unsigned int handle_width, bevel_width, frame_width, border_width;

  unsigned int screen_number;

  bool shadow_fonts, aa_fonts;

public:

  Style();
  Style(ImageControl *);
  ~Style();

  void readDatabaseMask(const std::string &rname,
                        PixmapMask &pixmapMask,
                        const Configuration &style);
  
  Texture readDatabaseTexture(const std::string &rname,
                               const std::string &default_color,
                               const Configuration &style, 
                               bool allowNoTexture = false);

  Color readDatabaseColor(const std::string &rname,
                           const std::string &default_color,
                           const Configuration &style);

  Font *readDatabaseFont(const std::string &rbasename,
                          const Configuration &style);

  void load(const Configuration &style);

  inline PixmapMask *getCloseButtonMask(void) { return &close_button; }
  inline PixmapMask *getMaximizeButtonMask(void) { return &max_button; }
  inline PixmapMask *getIconifyButtonMask(void) { return &icon_button; }
  inline PixmapMask *getStickyButtonMask(void) { return &stick_button; }

  inline Color *getTextFocus(void) { return &l_text_focus; }
  inline Color *getTextUnfocus(void) { return &l_text_unfocus; }

  inline Color *getButtonPicFocus(void) { return &b_pic_focus; }
  inline Color *getButtonPicUnfocus(void) { return &b_pic_unfocus; }

  inline Texture *getTitleFocus(void) { return &t_focus; }
  inline Texture *getTitleUnfocus(void) { return &t_unfocus; }

  inline Texture *getLabelFocus(void) { return &l_focus; }
  inline Texture *getLabelUnfocus(void) { return &l_unfocus; }

  inline Texture *getHandleFocus(void) { return &h_focus; }
  inline Texture *getHandleUnfocus(void) { return &h_unfocus; }

  inline Texture *getButtonFocus(void) { return &b_focus; }
  inline Texture *getButtonUnfocus(void) { return &b_unfocus; }

  inline Texture *getButtonPressedFocus(void)
  { return &b_pressed_focus; }
  inline Texture *getButtonPressedUnfocus(void)
  { return &b_pressed_unfocus; }

  inline Texture *getGripFocus(void) { return &g_focus; }
  inline Texture *getGripUnfocus(void) { return &g_unfocus; }

  inline unsigned int getHandleWidth(void) const { return handle_width; }
  inline unsigned int getBevelWidth(void) const { return bevel_width; }
  inline unsigned int getFrameWidth(void) const { return frame_width; }
  inline unsigned int getBorderWidth(void) const { return border_width; }

  inline const Font *getFont() const { return font; }

  inline void setShadowFonts(bool fonts) { shadow_fonts = fonts; }
  inline bool hasShadowFonts(void) const { return shadow_fonts; }

  inline void setAAFonts(bool fonts) { aa_fonts = fonts; }
  inline bool hasAAFonts(void) const { return aa_fonts; }

  inline TextJustify textJustify(void) { return justify; }
  inline BulletType bulletType(void) { return bullet_type; }

  inline const Color *getBorderColor() const { return &border_color; }

  inline const Texture *getFrameFocus() const { return &f_focus; }
  inline const Texture *getFrameUnfocus() const { return &f_unfocus; }

  inline void setImageControl(ImageControl *c) {
    image_control = c;
    screen_number = c->getScreenInfo()->screen();
  }
  inline unsigned int getScreen(void) { return screen_number; }

  // XXX add inline accessors for the rest of the bummy
};

}

#endif // __style_hh
