// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __rendertexture_hh
#define __rendertexture_hh

#include "rendercolor.hh"

namespace otk {

//! Superclass for all the Textures
class RenderTexture {
public:
  enum ReliefType {
    Flat,
    Raised,
    Sunken
  };
  enum BevelType {
    Bevel1,
    Bevel2
  };
  enum GradientType {
    Solid,
    Horizontal,
    Vertical,
    Diagonal,
    CrossDiagonal,
    PipeCross,
    Rectangle,
    Pyramid,
    Elliptic
  };

private:
  int _screen;
  
  //! If true, the texture is not rendered at all, so all options are ignored
  bool _parent_relative;
  //! The relief type of the texture
  ReliefType _relief;
  //! The way the bevel should be drawn
  BevelType _bevel;
  //! If a flat border is drawn on the outside, ignored for all ReliefType
  //! values except ReliefType::Flat
  bool _border;
  //! The type of gradient to fill the texture with (if any)
  GradientType _gradient;
  //! If interlace lines should be drawn over the texture
  bool _interlaced;

  //! The base color for the texture, the only color when the texture is solid.
  //! This must always be defined
  const RenderColor *_color;
  //! The secondary color for a gradient texture.
  //! This is only defined for gradients
  const RenderColor *_secondary_color;
  //! The shadow color for the bevel. This must be defined if
  //! RenderTexture::_relief is not RenderTexture::ReliefType::Flat
  const RenderColor *_bevel_dark_color;
  //! The light color for the bevel. This must be defined if
  //! RenderTexture::_relief is not RenderTexture::ReliefType::Flat
  const RenderColor *_bevel_light_color;
  //! The color for the flat border if RenderTexture::_border is true. This
  //! must be defined if it is true
  const RenderColor *_border_color;
  //! The color for the interlace lines if RenderTexture. This must be defined
  //! if it is true
  const RenderColor *_interlace_color;

public:
  RenderTexture(int screen,
                bool parent_relative, ReliefType relief, BevelType bevel,
                bool border, GradientType gradient, bool interlaced,
                const RenderColor::RGB &color,
                const RenderColor::RGB &secondary_color,
                const RenderColor::RGB &border_color,
                const RenderColor::RGB &interlace_color)
    : _screen(screen),
      _parent_relative(parent_relative),
      _relief(relief),
      _bevel(bevel),
      _border(border),
      _gradient(gradient),
      _interlaced(interlaced),
      _color(new RenderColor(screen, color)),
      _secondary_color(new RenderColor(screen, secondary_color)),
      _bevel_dark_color(0),
      _bevel_light_color(0),
      _border_color(new RenderColor(screen, border_color)),
      _interlace_color(new RenderColor(screen, interlace_color))
  {
    if (_relief != Flat) {
      unsigned char r, g, b;

      // calculate the light bevel color
      r = _color->red() + _color->red() / 2;
      g = _color->green() + _color->green() / 2;
      b = _color->blue() + _color->blue() / 2;
      // watch for wraparound
      if (r < _color->red()) r = 0xff;
      if (g < _color->green()) g = 0xff;
      if (b < _color->blue()) b = 0xff;
      _bevel_dark_color = new RenderColor(screen, r, g, b);
      
      // calculate the dark bevel color
      r = _color->red() / 4 + _color->red() / 2;
      g = _color->green() / 4 + _color->green() / 2;
      b = _color->blue() / 4 + _color->blue() / 2;
      _bevel_light_color = new RenderColor(screen, r, g, b);
    }
    
    assert(_relief == Flat || (_bevel_dark_color && _bevel_light_color));
    assert(!_border || _border_color);
    assert(!_interlaced || _interlace_color);
    assert(_color);
  }
  
  virtual ~RenderTexture() {
    delete _color;
    delete _secondary_color;
    if (_bevel_dark_color) delete _bevel_dark_color;
     if (_bevel_dark_color) delete _bevel_light_color;
    delete _border_color;
    delete _interlace_color;
  }

  //! If true, the texture is not rendered at all, so all options are ignored
  inline bool parentRelative() const { return _parent_relative; }
  //! The relief type of the texture
  inline ReliefType relief() const { return _relief; }
  //! The way the bevel should be drawn
  inline BevelType bevel() const { return _bevel; }
  //! If a flat border is drawn on the outside, ignored for all ReliefType
  //! values except ReliefType::Flat
  inline bool border() const { return _border; }
  //! The type of gradient to fill the texture with (if any)
  inline GradientType gradient() const { return _gradient; }
  //! If interlace lines should be drawn over the texture
  inline bool interlaced() const { return _interlaced; }

  //! The base color for the texture, the only color when the texture is solid.
  //! This must always be defined
  inline const RenderColor& color() const { return *_color; }
  //! The secondary color for gradient textures.
  //! This is only defined for gradients
  inline const RenderColor& secondary_color() const
    { return *_secondary_color; }
  //! The shadow color for the bevel. This must be defined if
  //! RenderTexture::_relief is not RenderTexture::ReliefType::Flat
  inline const RenderColor& bevelDarkColor() const
    { return *_bevel_dark_color; }
  //! The light color for the bevel. This must be defined if
  //! RenderTexture::)relief is not RenderTexture::ReliefType::Flat
  inline const RenderColor& bevelLightColor() const
    { return *_bevel_light_color; }
  //! The color for the flat border if RenderTexture::_border is true. This
  //! must be defined if it is true
  inline const RenderColor& borderColor() const { return *_border_color; }
  //! The color for the interlace lines if RenderTexture. This must be defined
  //! if it is true
  inline const RenderColor& interlaceColor() const
    { return *_interlace_color; }
};

}

#endif // __rendertexture_hh
