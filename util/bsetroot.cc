#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#ifdef    STDC_HEADERS
#  include <string.h>
#  include <stdlib.h>
#endif // STDC_HEADERS

#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#include "../src/i18n.h"
#include "bsetroot.h"


bsetroot::bsetroot(int argc, char **argv, char *dpy_name)
  : BaseDisplay(argv[0], dpy_name)
{
  pixmaps = (Pixmap *) 0;
  grad = fore = back = (char *) 0;

  Bool mod = False, sol = False, grd = False;
  int mod_x = 0, mod_y = 0, i = 0;

  img_ctrl = new BImageControl*[getNumberOfScreens()];
  for (; i < getNumberOfScreens(); i++)
    img_ctrl[i] = new BImageControl(*this, *getScreenInfo(i), True);

  for (i = 1; i < argc; i++) {
    if (! strcmp("-help", argv[i])) {
      usage();
    } else if ((! strcmp("-fg", argv[i])) ||
               (! strcmp("-foreground", argv[i])) ||
               (! strcmp("-from", argv[i]))) {
      if ((++i) >= argc) usage(1);

      fore = argv[i];
    } else if ((! strcmp("-bg", argv[i])) ||
               (! strcmp("-background", argv[i])) ||
               (! strcmp("-to", argv[i]))) {
      if ((++i) >= argc) usage(1);

      back = argv[i];
    } else if (! strcmp("-solid", argv[i])) {
      if ((++i) >= argc) usage(1);

      fore = argv[i];
      sol = True;
    } else if (! strcmp("-mod", argv[i])) {
      if ((++i) >= argc) usage();

      mod_x = atoi(argv[i]);

      if ((++i) >= argc) usage();

      mod_y = atoi(argv[i]);

      if (mod_x < 1) mod_x = 1;
      if (mod_y < 1) mod_y = 1;

      mod = True;
    } else if (! strcmp("-gradient", argv[i])) {
      if ((++i) >= argc) usage();

      grad = argv[i];
      grd = True;
    } else if (! strcmp("-display", argv[i])) {
      // -display passed through tests ealier... we just skip it now
      i++;
    } else
      usage();
  }

  if ((mod + sol + grd) != True) {
    fprintf(stderr,
	    i18n->
	    getMessage(
#ifdef    NLS
                       bsetrootSet, bsetrootMustSpecify,
#else // !NLS
                       0, 0,
#endif // NLS
		       "%s: error: must specify one of: -solid, -mod, -gradient\n"),
	    getApplicationName());
    
    usage(2);
  }
  
  if (sol && fore) solid();
  else if (mod && mod_x && mod_y && fore && back) modula(mod_x, mod_y);
  else if (grd && grad && fore && back) gradient();
  else usage();
}


bsetroot::~bsetroot(void) {
  XKillClient(getXDisplay(), AllTemporary);

  if (pixmaps) {
    int i;
    for (i = 0; i < getNumberOfScreens(); i++)
      if (pixmaps[i] != None) {
        XSetCloseDownMode(getXDisplay(), RetainTemporary);
        break;
      }

    delete [] pixmaps;
  }

  if (img_ctrl) {
    int i = 0;
    for (; i < getNumberOfScreens(); i++)
      delete img_ctrl[i];

    delete [] img_ctrl;
  }
}


void bsetroot::solid(void) {
  register int screen = 0;

  for (; screen < getNumberOfScreens(); screen++) {
    BColor c;

    img_ctrl[screen]->parseColor(&c, fore);
    if (! c.isAllocated()) c.setPixel(BlackPixel(getXDisplay(), screen));

    XSetWindowBackground(getXDisplay(), getScreenInfo(screen)->getRootWindow(),
                         c.getPixel());
    XClearWindow(getXDisplay(), getScreenInfo(screen)->getRootWindow());
  }
}


void bsetroot::modula(int x, int y) {
  char data[32];
  long pattern;

  register int screen, i;

  pixmaps = new Pixmap[getNumberOfScreens()];

  for (pattern = 0, screen = 0; screen < getNumberOfScreens(); screen++) {
    for (i = 0; i < 16; i++) {
      pattern <<= 1;
      if ((i % x) == 0)
        pattern |= 0x0001;
    }

    for (i = 0; i < 16; i++)
      if ((i %  y) == 0) {
        data[(i * 2)] = (char) 0xff;
        data[(i * 2) + 1] = (char) 0xff;
      } else {
        data[(i * 2)] = pattern & 0xff;
        data[(i * 2) + 1] = (pattern >> 8) & 0xff;
      }

    BColor f, b;
    GC gc;
    Pixmap bitmap;
    XGCValues gcv;

    bitmap =
      XCreateBitmapFromData(getXDisplay(),
                            getScreenInfo(screen)->getRootWindow(), data,
                            16, 16);

    img_ctrl[screen]->parseColor(&f, fore);
    img_ctrl[screen]->parseColor(&b, back);

    if (! f.isAllocated()) f.setPixel(WhitePixel(getXDisplay(), screen));
    if (! b.isAllocated()) b.setPixel(BlackPixel(getXDisplay(), screen));

    gcv.foreground = f.getPixel();
    gcv.background = b.getPixel();

    gc = XCreateGC(getXDisplay(), getScreenInfo(screen)->getRootWindow(),
                   GCForeground | GCBackground, &gcv);

    pixmaps[screen] =
      XCreatePixmap(getXDisplay(), getScreenInfo(screen)->getRootWindow(),
                    16, 16, getScreenInfo(screen)->getDepth());

    XCopyPlane(getXDisplay(), bitmap, pixmaps[screen], gc,
               0, 0, 16, 16, 0, 0, 1l);
    XSetWindowBackgroundPixmap(getXDisplay(),
                               getScreenInfo(screen)->getRootWindow(),
                               pixmaps[screen]);
    XClearWindow(getXDisplay(), getScreenInfo(screen)->getRootWindow());

    XFreeGC(getXDisplay(), gc);
    XFreePixmap(getXDisplay(), bitmap);

    if (! (getScreenInfo(screen)->getVisual()->c_class & 1)) {
      XFreePixmap(getXDisplay(), pixmaps[screen]);
      pixmaps[screen] = None;
    }
  }
}


void bsetroot::gradient(void) {
  register int screen;

  pixmaps = new Pixmap[getNumberOfScreens()];

  for (screen = 0; screen < getNumberOfScreens(); screen++) {
    BTexture texture;
    img_ctrl[screen]->parseTexture(&texture, grad);
    img_ctrl[screen]->parseColor(texture.getColor(), fore);
    img_ctrl[screen]->parseColor(texture.getColorTo(), back);

    if (! texture.getColor()->isAllocated())
      texture.getColor()->setPixel(WhitePixel(getXDisplay(), screen));
    if (! texture.getColorTo()->isAllocated())
      texture.getColorTo()->setPixel(BlackPixel(getXDisplay(), screen));

    pixmaps[screen] =
      img_ctrl[screen]->renderImage(getScreenInfo(screen)->getWidth(),
                                    getScreenInfo(screen)->getHeight(),
                                    &texture);

    XSetWindowBackgroundPixmap(getXDisplay(),
                               getScreenInfo(screen)->getRootWindow(),
                               pixmaps[screen]);
    XClearWindow(getXDisplay(), getScreenInfo(screen)->getRootWindow());

    if (! (getScreenInfo(screen)->getVisual()->c_class & 1)) {
      img_ctrl[screen]->removeImage(pixmaps[screen]);
      img_ctrl[screen]->timeout();
      pixmaps[screen] = None;
    }
  }
}


void bsetroot::usage(int exit_code) {
  fprintf(stderr,
          i18n->
	  getMessage(
#ifdef    NLS
                     bsetrootSet, bsetrootUsage,
#else // !NLS
                     0, 0,
#endif // NLS
	             "%s 2.0 : (c) 1997-1999 Brad Hughes\n\n"
		     "  -display <string>        display connection\n"
		     "  -mod <x> <y>             modula pattern\n"
		     "  -foreground, -fg <color> modula foreground color\n"
		     "  -background, -bg <color> modula background color\n\n"
		     "  -gradient <texture>      gradient texture\n"
		     "  -from <color>            gradient start color\n"
		     "  -to <color>              gradient end color\n\n"
		     "  -solid <color>           solid color\n\n"
		     "  -help                    print this help text and exit\n"),
	  getApplicationName());
  
  exit(exit_code);
}


int main(int argc, char **argv) {
  char *display_name = (char *) 0;
  int i = 1;
  
  NLSInit("openbox.cat");
  
  for (; i < argc; i++) {
    if (! strcmp(argv[i], "-display")) {
      // check for -display option
      
      if ((++i) >= argc) {
        fprintf(stderr,
		i18n->getMessage(
#ifdef    NLS
                                 mainSet, mainDISPLAYRequiresArg,
#else // !NLS
                                 0, 0,
#endif // NLS
				 "error: '-display' requires an argument\n"));
	
        ::exit(1);
      }
      
      display_name = argv[i];
    }
  }
  
  bsetroot app(argc, argv, display_name);
  
  return 0;
}
