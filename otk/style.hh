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

  unsigned int handle_width, bevel_width, frame_width, border_width;

  unsigned int screen_number;

  bool shadow_fonts, aa_fonts;

public:

  Style();
  Style(unsigned int);
  Style(unsigned int, BImageControl *);
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

  void load(Configuration &);

  inline unsigned int getHandleWidth(void) const { return handle_width; }
  inline unsigned int getBevelWidth(void) const { return bevel_width; }
  inline unsigned int getFrameWidth(void) const { return frame_width; }
  inline unsigned int getBorderWidth(void) const { return border_width; }

  inline const BFont *getFont() const { return font; }

  inline void setImageControl(BImageControl *c) { image_control = c; }
  inline void setScreenNumber(unsigned int scr) { screen_number = scr; }

  // XXX add inline accessors for the rest of the bummy
};

}

#endif // __style_hh
