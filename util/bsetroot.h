#ifndef   __bsetroot2_hh
#define   __bsetroot2_hh

#include "../src/BaseDisplay.h"
#include "../src/Image.h"


class bsetroot : public BaseDisplay {
private:
  BImageControl **img_ctrl;
  Pixmap *pixmaps;

  char *fore, *back, *grad;


protected:
  inline virtual void process_event(XEvent *) { }

 
public:
  bsetroot(int, char **, char * = 0);
  ~bsetroot(void);

  inline virtual Bool handleSignal(int) { return False; }

  void gradient(void);
  void modula(int, int);
  void solid(void);
  void usage(int = 0);
};


#endif // __bsetroot2_hh
