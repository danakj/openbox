// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include <assert.h>
#include <iostream>

#include "display.hh"
#include "util.hh"
#include "style.hh"

namespace otk {

Style::Style() : font(NULL)
{
}

Style::Style(ImageControl *ctrl)
  : image_control(ctrl), font(0),
    screen_number(ctrl->getScreenInfo()->screen())
{
}

Style::~Style() {
  if (font)
    delete font;

  if (close_button.mask != None)
    XFreePixmap(Display::display, close_button.mask);
  if (max_button.mask != None)
    XFreePixmap(Display::display, max_button.mask);
  if (icon_button.mask != None)
    XFreePixmap(Display::display, icon_button.mask);
  if (stick_button.mask != None)
    XFreePixmap(Display::display, stick_button.mask);

  max_button.mask = None;
  close_button.mask = None;
  icon_button.mask = None;
  stick_button.mask = None;
}

void Style::load(const Configuration &style) {
  std::string s;

  // load fonts/fontsets
  if (font)
    delete font;

  font = readDatabaseFont("window.", style);

  // load window config
  t_focus = readDatabaseTexture("window.title.focus", "white", style);
  t_unfocus = readDatabaseTexture("window.title.unfocus", "black", style);

  l_focus = readDatabaseTexture("window.label.focus", "white", style);
  l_unfocus = readDatabaseTexture("window.label.unfocus", "black", style);

  h_focus = readDatabaseTexture("window.handle.focus", "white", style);
  h_unfocus = readDatabaseTexture("window.handle.unfocus", "black", style);

  g_focus = readDatabaseTexture("window.grip.focus", "white", style);
  g_unfocus = readDatabaseTexture("window.grip.unfocus", "black", style);

  b_focus = readDatabaseTexture("window.button.focus", "white", style);
  b_unfocus = readDatabaseTexture("window.button.unfocus", "black", style);

  //if neither of these can be found, we will use the previous resource
  b_pressed_focus = readDatabaseTexture("window.button.pressed.focus",
                                        "black", style, true);
  if (b_pressed_focus.texture() == Texture::NoTexture) {
    b_pressed_focus = readDatabaseTexture("window.button.pressed", "black",
                                          style);
  }
    
  b_pressed_unfocus = readDatabaseTexture("window.button.pressed.unfocus",
                                          "black", style, true);
  if (b_pressed_unfocus.texture() == Texture::NoTexture) {
    b_pressed_unfocus = readDatabaseTexture("window.button.pressed", "black",
                                            style);
  }

  if (close_button.mask != None)
    XFreePixmap(Display::display, close_button.mask);
  if (max_button.mask != None)
    XFreePixmap(Display::display, max_button.mask);
  if (icon_button.mask != None)
    XFreePixmap(Display::display, icon_button.mask);
  if (stick_button.mask != None)
    XFreePixmap(Display::display, stick_button.mask);

  close_button.mask = max_button.mask = icon_button.mask
                    = icon_button.mask = None;
  
  readDatabaseMask("window.button.close.mask", close_button, style);
  readDatabaseMask("window.button.max.mask", max_button, style);
  readDatabaseMask("window.button.icon.mask", icon_button, style);
  readDatabaseMask("window.button.stick.mask", stick_button, style);

  // we create the window.frame texture by hand because it exists only to
  // make the code cleaner and is not actually used for display
  Color color = readDatabaseColor("window.frame.focusColor", "white",
                                        style);
  f_focus = Texture("solid flat", screen_number, image_control);
  f_focus.setColor(color);

  color = readDatabaseColor("window.frame.unfocusColor", "white", style);
  f_unfocus = Texture("solid flat", screen_number, image_control);
  f_unfocus.setColor(color);

  l_text_focus = readDatabaseColor("window.label.focus.textColor",
                                   "black", style);
  l_text_unfocus = readDatabaseColor("window.label.unfocus.textColor",
                                     "white", style);

  b_pic_focus = readDatabaseColor("window.button.focus.picColor",
                                  "black", style);
  b_pic_unfocus = readDatabaseColor("window.button.unfocus.picColor",
                                    "white", style);

  justify = LeftJustify;

  if (style.getValue("window.justify", s)) {
    if (s == "right" || s == "Right")
      justify = RightJustify;
    else if (s == "center" || s == "Center")
      justify = CenterJustify;
  }

  // sanity checks
  if (t_focus.texture() == Texture::Parent_Relative)
    t_focus = f_focus;
  if (t_unfocus.texture() == Texture::Parent_Relative)
    t_unfocus = f_unfocus;
  if (h_focus.texture() == Texture::Parent_Relative)
    h_focus = f_focus;
  if (h_unfocus.texture() == Texture::Parent_Relative)
    h_unfocus = f_unfocus;

  border_color = readDatabaseColor("borderColor", "black", style);

  // load bevel, border and handle widths

  const ScreenInfo *s_info = Display::screenInfo(screen_number);
  unsigned int width = s_info->rect().width();

  if (! style.getValue("handleWidth", handle_width) ||
      handle_width > width/2 || handle_width == 0)
    handle_width = 6;

  if (! style.getValue("borderWidth", border_width))
    border_width = 1;

  if (! style.getValue("bevelWidth", bevel_width)
      || bevel_width > width/2 || bevel_width == 0)
    bevel_width = 3;

  if (! style.getValue("frameWidth", frame_width)
      || frame_width > width/2)
    frame_width = bevel_width;

  if (style.getValue("rootCommand", s))
    bexec(s, s_info->displayString());
}


void Style::readDatabaseMask(const std::string &rname, PixmapMask &pixmapMask,
                             const Configuration &style) {
  Window root_window = Display::screenInfo(screen_number)->rootWindow();
  std::string s;
  int hx, hy; //ignored
  int ret = BitmapOpenFailed; //default to failure.
  
  if (style.getValue(rname, s)) {
    if (s[0] != '/' && s[0] != '~') {
      std::string xbmFile = std::string("~/.openbox/buttons/") + s;
      ret = XReadBitmapFile(Display::display, root_window,
                            expandTilde(xbmFile).c_str(), &pixmapMask.w,
                            &pixmapMask.h, &pixmapMask.mask, &hx, &hy);
      if (ret != BitmapSuccess) {
        xbmFile = std::string(BUTTONSDIR) + "/" + s;
        ret = XReadBitmapFile(Display::display, root_window,
                              xbmFile.c_str(), &pixmapMask.w,
                              &pixmapMask.h, &pixmapMask.mask, &hx, &hy);
      }
    } else
      ret = XReadBitmapFile(Display::display, root_window,
                            expandTilde(s).c_str(), &pixmapMask.w,
                            &pixmapMask.h, &pixmapMask.mask, &hx, &hy);
    
    if (ret == BitmapSuccess)
      return;
  }

  pixmapMask.mask = None;
  pixmapMask.w = pixmapMask.h = 0;
}


Texture Style::readDatabaseTexture(const std::string &rname,
                                   const std::string &default_color,
                                   const Configuration &style, 
                                   bool allowNoTexture)
{
  Texture texture;
  std::string s;

  if (style.getValue(rname, s))
    texture = Texture(s);
  else if (allowNoTexture) //no default
    texture.setTexture(Texture::NoTexture);
  else
    texture.setTexture(Texture::Solid | Texture::Flat);

  // associate this texture with this screen
  texture.setScreen(screen_number);
  texture.setImageControl(image_control);

  if (texture.texture() != Texture::NoTexture) {
    texture.setColor(readDatabaseColor(rname + ".color", default_color,
                                       style));
    texture.setColorTo(readDatabaseColor(rname + ".colorTo", default_color,
                                         style));
    texture.setBorderColor(readDatabaseColor(rname + ".borderColor",
                                             default_color, style));
  }

  return texture;
}


Color Style::readDatabaseColor(const std::string &rname,
                               const std::string &default_color,
                               const Configuration &style) {
  Color color;
  std::string s;
  if (style.getValue(rname, s))
    color = Color(s, screen_number);
  else
    color = Color(default_color, screen_number);
  return color;
}


Font *Style::readDatabaseFont(const std::string &rbasename,
                              const Configuration &style) {
  std::string fontstring, s;

  // XXX: load all this font stuff from the style...

  bool dropShadow = True;
    
  unsigned char offset = 1;
  if (style.getValue(rbasename + "xft.shadow.offset", s)) {
    offset = atoi(s.c_str()); //doesn't detect errors
    if (offset > CHAR_MAX)
      offset = 1;
  }

  unsigned char tint = 0x40;
  if (style.getValue(rbasename + "xft.shadow.tint", s)) {
    tint = atoi(s.c_str());
  }

  fontstring = "Arial,Sans-9:bold";

  // if this fails, it ::exit()'s
  return new Font(screen_number, fontstring, dropShadow, offset, tint);
}

}
