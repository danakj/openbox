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

class Style {
public:

  enum Type { ButtonFocus, ButtonUnfocus, TitleFocus, TitleUnfocus,
              LabelFocus, LabelUnfocus, HandleFocus, HandleUnfocus,
              GripFocus, GripUnfocus };

  enum TextJustify { LeftJustify = 1, RightJustify, CenterJustify };
  enum BulletType { RoundBullet = 1, TriangleBullet, SquareBullet, NoBullet };

  struct PixmapMask {
    Pixmap mask;
    unsigned int w, h;
  };

// private:

  BImageControl *image_control;

  BColor
    l_text_focus, l_text_unfocus,
    b_pic_focus, b_pic_unfocus;
  
  BColor border_color;

  BFont *font;

  BTexture
    f_focus, f_unfocus,
    t_focus, t_unfocus,
    l_focus, l_unfocus,
    h_focus, h_unfocus,
    b_focus, b_unfocus,
    b_pressed, b_pressed_focus, b_pressed_unfocus,
    g_focus, g_unfocus;

  PixmapMask close_button, max_button, icon_button, stick_button;
  TextJustify justify;
  BulletType bullet_type;

  unsigned int handle_width, bevel_width, frame_width, border_width;

  unsigned int screen_number;

  bool shadow_fonts, aa_fonts;

public:

  Style();
  Style(BImageControl *);
  ~Style();

  void doJustify(const std::string &text, int &start_pos,
                 unsigned int max_length, unsigned int modifier) const;

  void readDatabaseMask(const std::string &rname,
                        PixmapMask &pixmapMask,
                        const Configuration &style);
  
  BTexture readDatabaseTexture(const std::string &rname,
                               const std::string &default_color,
                               const Configuration &style, 
                               bool allowNoTexture = false);

  BColor readDatabaseColor(const std::string &rname,
                           const std::string &default_color,
                           const Configuration &style);

  BFont *readDatabaseFont(const std::string &rbasename,
                          const Configuration &style);

  void load(const Configuration &style);

  inline BColor *getBorderColor(void) { return &border_color; }

  inline BColor *getTextFocus(void) { return &l_text_focus; }
  inline BColor *getTextUnfocus(void) { return &l_text_unfocus; }

  inline BColor *getButtonPicFocus(void) { return &b_pic_focus; }
  inline BColor *getButtonPicUnfocus(void) { return &b_pic_unfocus; }

  inline BTexture *getFrameFocus(void) { return &f_focus; }
  inline BTexture *getFrameUnfocus(void) { return &f_unfocus; }

  inline BTexture *getTitleFocus(void) { return &t_focus; }
  inline BTexture *getTitleUnfocus(void) { return &t_unfocus; }

  inline BTexture *getLabelFocus(void) { return &l_focus; }
  inline BTexture *getLabelUnfocus(void) { return &l_unfocus; }

  inline BTexture *getHandleFocus(void) { return &h_focus; }
  inline BTexture *getHandleUnfocus(void) { return &h_unfocus; }

  inline BTexture *getButtonFocus(void) { return &b_focus; }
  inline BTexture *getButtonUnfocus(void) { return &b_unfocus; }

  inline BTexture *getButtonPressedFocus(void)
  { return &b_pressed_focus; }
  inline BTexture *getButtonPressedUnfocus(void)
  { return &b_pressed_unfocus; }

  inline BTexture *getGripFocus(void) { return &g_focus; }
  inline BTexture *getGripUnfocus(void) { return &g_unfocus; }

  inline unsigned int getHandleWidth(void) const { return handle_width; }
  inline unsigned int getBevelWidth(void) const { return bevel_width; }
  inline unsigned int getFrameWidth(void) const { return frame_width; }
  inline unsigned int getBorderWidth(void) const { return border_width; }

  inline const BFont &getFont() const { return *font; }
  inline bool hasAAFonts(void) const { return aa_fonts; }

  inline TextJustify textJustify(void) { return justify; }
  inline BulletType bulletType(void) { return bullet_type; }

  inline const BColor &getBorderColor() const { return border_color; }

  inline const BTexture &getFrameFocus() const { return f_focus; }
  inline const BTexture &getFrameUnfocus() const { return f_unfocus; }

  inline void setImageControl(BImageControl *c) {
    image_control = c;
    screen_number = c->getScreenInfo()->getScreenNumber();
  }
  inline unsigned int getScreen(void) { return screen_number; }

  // XXX add inline accessors for the rest of the bummy
};

}

#endif // __style_hh
