#include <string>
#include <list>

#include "rect.hh"
#include "point.hh"
#include "texture.hh"

namespace otk {

class OtkWidget {

public:

  typedef std::list<OtkWidget *> OtkWidgetList;

  OtkWidget(OtkWidget *parent);
  OtkWidget(unsigned int screen, Cursor);

  virtual ~OtkWidget();

  inline Window getWindow(void) const { return _window; }
  inline const OtkWidget *getParent(void) const { return _parent; }
  inline const OtkWidgetList &getChildren(void) const { return _children; }
  inline unsigned int getScreen(void) const { return _screen; }
  inline const Rect &getRect(void) const { return _rect; }

  void move(const Point &to);
  void move(int x, int y);

  virtual void resize(const Point &to);
  virtual void resize(int x, int y);

  virtual void setGeometry(const Rect &new_geom);
  virtual void setGeometry(const Point &topleft, int width, int height);
  virtual void setGeometry(int x, int y, int width, int height);

  inline bool isVisible(void) const { return _visible; };
  virtual void show(void);
  virtual void hide(void);

  inline bool isFocused(void) const { return _focused; };
  virtual void focus(void);
  virtual void blur(void);

  inline bool hasGrabbedMouse(void) const { return _grabbed_mouse; }
  bool grabMouse(void);
  void ungrabMouse(void);

  inline bool hasGrabbedKeyboard(void) const { return _grabbed_keyboard; }
  bool grabKeyboard(void);
  void ungrabKeyboard(void);

  inline const BTexture *getTexture(void) const { return _texture; }
  virtual void setTexture(BTexture *texture);

  virtual void addChild(OtkWidget *child);
  virtual void removeChild(OtkWidget *child);

  inline bool getStretchableHorz(void) const { return _stretchable_horz; }
  void setStretchableHorz(bool s_horz) { _stretchable_horz = s_horz; }

  inline bool getStretchableVert(void) const { return _stretchable_vert; }
  void setStretchableVert(bool s_vert)  { _stretchable_vert = s_vert; }

  inline Cursor getCursor(void) const { return _cursor; }

private:

  void create(void);

  Window _window;

  OtkWidget *_parent;
  OtkWidgetList _children;
 
  bool _visible;
  bool _focused;

  bool _grabbed_mouse;
  bool _grabbed_keyboard;

  bool _stretchable_vert;
  bool _stretchable_horz;

  BTexture *_texture;

  Rect _rect;
  unsigned int _screen;

  Cursor _cursor;
};

}
