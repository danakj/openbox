// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __oblabelwidget_hh
#define   __oblabelwidget_hh

#include "widgetbase.hh"
#include "otk/widget.hh"
#include "otk/font.hh"
#include "otk/style.hh"

namespace ob {

class LabelWidget : public otk::Widget, public WidgetBase
{
private:
  void setTextures();
  const otk::Font *_font;
  otk::Color *_text_color;
  int _sidemargin;
  otk::Style::TextJustify _justify;
  std::string _text;
  //! Object used by Xft to render to the drawable
  XftDraw *_xftdraw;
  
public:
  LabelWidget(otk::Widget *parent, WidgetBase::WidgetType type);
  virtual ~LabelWidget();

  virtual void setStyle(otk::Style *style);

  virtual void adjust();

  virtual void focus();
  virtual void unfocus();

  virtual void update();

  inline const std::string &text() const { return _text; }
  void setText(const std::string &text);
};

}

#endif // __oblabelwidget_hh
