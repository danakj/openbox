// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __oblabelwidget_hh
#define   __oblabelwidget_hh

#include "widgetbase.hh"
#include "otk/widget.hh"
#include "otk/font.hh"
#include "otk/renderstyle.hh"
#include "otk/ustring.hh"

namespace ob {

class LabelWidget : public otk::Widget, public WidgetBase
{
private:
  void setTextures();
  const otk::Font *_font;
  otk::RenderColor *_text_color;
  int _sidemargin;
  otk::RenderStyle::TextJustify _justify;
  otk::ustring _text;
  
public:
  LabelWidget(otk::Widget *parent, WidgetBase::WidgetType type);
  virtual ~LabelWidget();

  virtual void setStyle(otk::RenderStyle *style);

  virtual void adjust();

  virtual void focus();
  virtual void unfocus();

  virtual void update();

  virtual void renderForeground();

  inline const otk::ustring &text() const { return _text; }
  void setText(const otk::ustring &text);
};

}

#endif // __oblabelwidget_hh
