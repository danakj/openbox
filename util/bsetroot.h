#ifndef   __bsetroot2_hh
#define   __bsetroot2_hh

#include "../src/BaseDisplay.h"
#include "../src/Image.h"

class bsetroot : public BaseDisplay {
private:
  BImageControl **img_ctrl;

  char *fore, *back, *grad;

  // no copying!!
  bsetroot(const bsetroot &);
  bsetroot& operator=(const bsetroot&);

  inline virtual void process_event(XEvent * /*unused*/) { }

public:
  bsetroot(int argc, char **argv, char *dpy_name = 0);
  ~bsetroot(void);

  inline virtual Bool handleSignal(int /*unused*/) { return False; }

  void setPixmapProperty(int screen, Pixmap pixmap);
  Pixmap duplicatePixmap(int screen, Pixmap pixmap, int width, int height);

  void gradient(void);
  void modula(int x, int y);
  void solid(void);
  void usage(int exit_code = 0);
};

#endif // __bsetroot2_hh
