#include "widget.hh"
#include "style.hh"
#include "texture.hh"
//#include "pixmap.hh"

namespace otk {

class OtkButton : public OtkWidget {

public:

  OtkButton(OtkWidget *parent);
  ~OtkButton();

  inline const std::string &getText(void) const { return _text; }
  void setText(const std::string &text);

  //inline const OtkPixmap &getPixmap(void) const { return _pixmap; }
  //void setPixmap(const OtkPixmap &pixmap);

  inline bool isPressed(void) const { return _pressed; }
  void press(void);
  void release(void);

private:

  std::string _text;
  //OtkPixmap _pixmap;
  bool _pressed;
  BTexture *_unfocus_tx;

};

}
