// Screen.cc for Openbox
// Copyright (c) 2001 Sean 'Shaleh' Perry <shaleh@debian.org>
// Copyright (c) 1997 - 2000 Brad Hughes (bhughes@tcac.net)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

// stupid macros needed to access some functions in version 2 of the GNU C
// library
#ifndef   _GNU_SOURCE
#define   _GNU_SOURCE
#endif // _GNU_SOURCE

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include <X11/Xatom.h>
#include <X11/keysym.h>

#include "i18n.h"
#include "openbox.h"
#include "Clientmenu.h"
#include "Iconmenu.h"
#include "Image.h"
#include "Screen.h"

#ifdef    SLIT
#include "Slit.h"
#endif // SLIT

#include "Rootmenu.h"
#include "Toolbar.h"
#include "Window.h"
#include "Workspace.h"
#include "Workspacemenu.h"
#include "Util.h"

#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#ifdef    HAVE_STRING_H
#  include <string.h>
#endif // HAVE_STRING_H

#ifdef    HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif // HAVE_SYS_TYPES_H

#ifdef    HAVE_CTYPE_H
#  include <ctype.h>
#endif // HAVE_CTYPE_H

#ifdef    HAVE_DIRENT_H
#  include <dirent.h>
#endif // HAVE_DIRENT_H

#ifdef    HAVE_LOCALE_H
#  include <locale.h>
#endif // HAVE_LOCALE_H

#ifdef    HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif // HAVE_UNISTD_H

#ifdef    HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif // HAVE_SYS_STAT_H

#ifdef    HAVE_STDARG_H
#  include <stdarg.h>
#endif // HAVE_STDARG_H

#ifndef    HAVE_SNPRINTF
#  include "bsd-snprintf.h"
#endif // !HAVE_SNPRINTF

#ifndef   MAXPATHLEN
#define   MAXPATHLEN 255
#endif // MAXPATHLEN

#ifndef   FONT_ELEMENT_SIZE
#define   FONT_ELEMENT_SIZE 50
#endif // FONT_ELEMENT_SIZE

#include <strstream>
#include <string>
#include <algorithm>
#include <functional>
using std::ends;

static Bool running = True;

static int anotherWMRunning(Display *display, XErrorEvent *) {
  fprintf(stderr, i18n->getMessage(ScreenSet, ScreenAnotherWMRunning,
     "BScreen::BScreen: an error occured while querying the X server.\n"
	     "  another window manager already running on display %s.\n"),
          DisplayString(display));

  running = False;

  return(-1);
}

struct dcmp {
  bool operator()(const char *one, const char *two) const {
    return (strcmp(one, two) < 0) ? True : False;
  }
};

#ifndef    HAVE_STRCASESTR
static const char * strcasestr(const char *str, const char *ptn) {
  const char *s2, *p2;
  for( ; *str; str++) {
    for(s2=str,p2=ptn; ; s2++,p2++) {
      if (!*p2) return str;
      if (toupper(*s2) != toupper(*p2)) break;
    }
  }
  return NULL;
}
#endif // HAVE_STRCASESTR

static const char *getFontElement(const char *pattern, char *buf, int bufsiz, ...) {
  const char *p, *v;
  char *p2;
  va_list va;

  va_start(va, bufsiz);
  buf[bufsiz-1] = 0;
  buf[bufsiz-2] = '*';
  while((v = va_arg(va, char *)) != NULL) {
    p = strcasestr(pattern, v);
    if (p) {
      strncpy(buf, p+1, bufsiz-2);
      p2 = strchr(buf, '-');
      if (p2) *p2=0;
      va_end(va);
      return p;
    }
  }
  va_end(va);
  strncpy(buf, "*", bufsiz);
  return NULL;
}

static const char *getFontSize(const char *pattern, int *size) {
  const char *p;
  const char *p2=NULL;
  int n=0;

  for (p=pattern; 1; p++) {
    if (!*p) {
      if (p2!=NULL && n>1 && n<72) {
	*size = n; return p2+1;
      } else {
	*size = 16; return NULL;
      }
    } else if (*p=='-') {
      if (n>1 && n<72 && p2!=NULL) {
	*size = n;
	return p2+1;
      }
      p2=p; n=0;
    } else if (*p>='0' && *p<='9' && p2!=NULL) {
      n *= 10;
      n += *p-'0';
    } else {
      p2=NULL; n=0;
    }
  }
}


BScreen::BScreen(Openbox &ob, int scrn, Resource &conf) : ScreenInfo(ob, scrn),
  openbox(ob), config(conf)
{
  event_mask = ColormapChangeMask | EnterWindowMask | PropertyChangeMask |
	       SubstructureRedirectMask | KeyPressMask | KeyReleaseMask |
	       ButtonPressMask | ButtonReleaseMask;

  XErrorHandler old = XSetErrorHandler((XErrorHandler) anotherWMRunning);
  XSelectInput(getBaseDisplay().getXDisplay(), getRootWindow(), event_mask);
  XSync(getBaseDisplay().getXDisplay(), False);
  XSetErrorHandler((XErrorHandler) old);

  managed = running;
  if (! managed) return;

  fprintf(stderr, i18n->getMessage(ScreenSet, ScreenManagingScreen,
		     "BScreen::BScreen: managing screen %d "
		     "using visual 0x%lx, depth %d\n"),
	  getScreenNumber(), XVisualIDFromVisual(getVisual()),
          getDepth());

  rootmenu = 0;

  resource.mstyle.t_fontset = resource.mstyle.f_fontset =
    resource.tstyle.fontset = resource.wstyle.fontset = NULL;
  resource.mstyle.t_font = resource.mstyle.f_font = resource.tstyle.font =
    resource.wstyle.font = NULL;
  resource.root_command = NULL;
#ifdef    HAVE_STRFTIME
  resource.strftime_format = NULL;
#endif // HAVE_STRFTIME

#ifdef   SLIT
  slit = NULL;
#endif // SLIT
  toolbar = NULL;
  current_workspace = (Workspace *) 0;

#ifdef    HAVE_GETPID
  pid_t bpid = getpid();

  XChangeProperty(getBaseDisplay().getXDisplay(), getRootWindow(),
                  openbox.getOpenboxPidAtom(), XA_CARDINAL,
                  sizeof(pid_t) * 8, PropModeReplace,
                  (unsigned char *) &bpid, 1);
#endif // HAVE_GETPID

  XDefineCursor(getBaseDisplay().getXDisplay(), getRootWindow(),
                openbox.getSessionCursor());

  image_control =
    new BImageControl(openbox, *this, True, openbox.getColorsPerChannel(),
                      openbox.getCacheLife(), openbox.getCacheMax());
  image_control->installRootColormap();
  root_colormap_installed = True;

  load();       // load config options from Resources
  LoadStyle();

  XGCValues gcv;
  unsigned long gc_value_mask = GCForeground;
  if (! i18n->multibyte()) gc_value_mask |= GCFont;

  gcv.foreground = WhitePixel(getBaseDisplay().getXDisplay(),
			      getScreenNumber())
                 ^ BlackPixel(getBaseDisplay().getXDisplay(),
			      getScreenNumber());
  gcv.function = GXxor;
  gcv.subwindow_mode = IncludeInferiors;
  opGC = XCreateGC(getBaseDisplay().getXDisplay(), getRootWindow(),
                   GCForeground | GCFunction | GCSubwindowMode, &gcv);

  gcv.foreground = resource.wstyle.l_text_focus.getPixel();
  if (resource.wstyle.font)
    gcv.font = resource.wstyle.font->fid;
  resource.wstyle.l_text_focus_gc =
    XCreateGC(getBaseDisplay().getXDisplay(), getRootWindow(),
	      gc_value_mask, &gcv);

  gcv.foreground = resource.wstyle.l_text_unfocus.getPixel();
  if (resource.wstyle.font)
    gcv.font = resource.wstyle.font->fid;
  resource.wstyle.l_text_unfocus_gc =
    XCreateGC(getBaseDisplay().getXDisplay(), getRootWindow(),
	      gc_value_mask, &gcv);

  gcv.foreground = resource.wstyle.b_pic_focus.getPixel();
  resource.wstyle.b_pic_focus_gc =
    XCreateGC(getBaseDisplay().getXDisplay(), getRootWindow(),
	      GCForeground, &gcv);

  gcv.foreground = resource.wstyle.b_pic_unfocus.getPixel();
  resource.wstyle.b_pic_unfocus_gc =
    XCreateGC(getBaseDisplay().getXDisplay(), getRootWindow(),
	      GCForeground, &gcv);

  gcv.foreground = resource.mstyle.t_text.getPixel();
  if (resource.mstyle.t_font)
    gcv.font = resource.mstyle.t_font->fid;
  resource.mstyle.t_text_gc =
    XCreateGC(getBaseDisplay().getXDisplay(), getRootWindow(),
	      gc_value_mask, &gcv);

  gcv.foreground = resource.mstyle.f_text.getPixel();
  if (resource.mstyle.f_font)
    gcv.font = resource.mstyle.f_font->fid;
  resource.mstyle.f_text_gc =
    XCreateGC(getBaseDisplay().getXDisplay(), getRootWindow(),
	      gc_value_mask, &gcv);

  gcv.foreground = resource.mstyle.h_text.getPixel();
  resource.mstyle.h_text_gc =
    XCreateGC(getBaseDisplay().getXDisplay(), getRootWindow(),
	      gc_value_mask, &gcv);

  gcv.foreground = resource.mstyle.d_text.getPixel();
  resource.mstyle.d_text_gc =
    XCreateGC(getBaseDisplay().getXDisplay(), getRootWindow(),
	      gc_value_mask, &gcv);

  gcv.foreground = resource.mstyle.hilite.getColor()->getPixel();
  resource.mstyle.hilite_gc =
    XCreateGC(getBaseDisplay().getXDisplay(), getRootWindow(),
	      gc_value_mask, &gcv);

  gcv.foreground = resource.tstyle.l_text.getPixel();
  if (resource.tstyle.font)
    gcv.font = resource.tstyle.font->fid;
  resource.tstyle.l_text_gc =
    XCreateGC(getBaseDisplay().getXDisplay(), getRootWindow(),
	      gc_value_mask, &gcv);

  gcv.foreground = resource.tstyle.w_text.getPixel();
  resource.tstyle.w_text_gc =
    XCreateGC(getBaseDisplay().getXDisplay(), getRootWindow(),
	      gc_value_mask, &gcv);

  gcv.foreground = resource.tstyle.c_text.getPixel();
  resource.tstyle.c_text_gc =
    XCreateGC(getBaseDisplay().getXDisplay(), getRootWindow(),
	      gc_value_mask, &gcv);

  gcv.foreground = resource.tstyle.b_pic.getPixel();
  resource.tstyle.b_pic_gc =
    XCreateGC(getBaseDisplay().getXDisplay(), getRootWindow(),
	      gc_value_mask, &gcv);

  const char *s =  i18n->getMessage(ScreenSet, ScreenPositionLength,
				    "0: 0000 x 0: 0000");
  int l = strlen(s);

  if (i18n->multibyte()) {
    XRectangle ink, logical;
    XmbTextExtents(resource.wstyle.fontset, s, l, &ink, &logical);
    geom_w = logical.width;

    geom_h = resource.wstyle.fontset_extents->max_ink_extent.height;
  } else {
    geom_h = resource.wstyle.font->ascent +
	     resource.wstyle.font->descent;

    geom_w = XTextWidth(resource.wstyle.font, s, l);
  }

  geom_w += (resource.bevel_width * 2);
  geom_h += (resource.bevel_width * 2);

  XSetWindowAttributes attrib;
  unsigned long mask = CWBorderPixel | CWColormap | CWSaveUnder;
  attrib.border_pixel = getBorderColor()->getPixel();
  attrib.colormap = getColormap();
  attrib.save_under = True;

  geom_window =
    XCreateWindow(getBaseDisplay().getXDisplay(), getRootWindow(),
                  0, 0, geom_w, geom_h, resource.border_width, getDepth(),
                  InputOutput, getVisual(), mask, &attrib);
  geom_visible = False;

  if (resource.wstyle.l_focus.getTexture() & BImage_ParentRelative) {
    if (resource.wstyle.t_focus.getTexture() ==
	                              (BImage_Flat | BImage_Solid)) {
      geom_pixmap = None;
      XSetWindowBackground(getBaseDisplay().getXDisplay(), geom_window,
			   resource.wstyle.t_focus.getColor()->getPixel());
    } else {
      geom_pixmap = image_control->renderImage(geom_w, geom_h,
					       &resource.wstyle.t_focus);
      XSetWindowBackgroundPixmap(getBaseDisplay().getXDisplay(),
				 geom_window, geom_pixmap);
    }
  } else {
    if (resource.wstyle.l_focus.getTexture() ==
	                              (BImage_Flat | BImage_Solid)) {
      geom_pixmap = None;
      XSetWindowBackground(getBaseDisplay().getXDisplay(), geom_window,
			   resource.wstyle.l_focus.getColor()->getPixel());
    } else {
      geom_pixmap = image_control->renderImage(geom_w, geom_h,
					       &resource.wstyle.l_focus);
      XSetWindowBackgroundPixmap(getBaseDisplay().getXDisplay(),
				 geom_window, geom_pixmap);
    }
  }

  workspacemenu = new Workspacemenu(*this);
  iconmenu = new Iconmenu(*this);
  configmenu = new Configmenu(*this);

  Workspace *wkspc = NULL;
  if (resource.workspaces != 0) {
    for (int i = 0; i < resource.workspaces; ++i) {
      wkspc = new Workspace(*this, workspacesList.size());
      workspacesList.push_back(wkspc);
      workspacemenu->insert(wkspc->getName(), wkspc->getMenu());
    }
  } else {
    setWorkspaceCount(1);
    wkspc = new Workspace(*this, workspacesList.size());
    workspacesList.push_back(wkspc);
    workspacemenu->insert(wkspc->getName(), wkspc->getMenu());
  }
  saveWorkspaceNames();

  workspacemenu->insert(i18n->getMessage(IconSet, IconIcons, "Icons"),
			iconmenu);
  workspacemenu->update();

  current_workspace = workspacesList.front();
  workspacemenu->setItemSelected(2, True);

  toolbar = new Toolbar(*this, config);

#ifdef    SLIT
  slit = new Slit(*this, config);
#endif // SLIT

  InitMenu();

  raiseWindows(0, 0);
  rootmenu->update();

  changeWorkspaceID(0);

  int i;
  unsigned int nchild;
  Window r, p, *children;
  XQueryTree(getBaseDisplay().getXDisplay(), getRootWindow(), &r, &p,
	     &children, &nchild);

  // preen the window list of all icon windows... for better dockapp support
  for (i = 0; i < (int) nchild; i++) {
    if (children[i] == None) continue;

    XWMHints *wmhints = XGetWMHints(getBaseDisplay().getXDisplay(),
				    children[i]);

    if (wmhints) {
      if ((wmhints->flags & IconWindowHint) &&
	  (wmhints->icon_window != children[i]))
        for (int j = 0; j < (int) nchild; j++)
          if (children[j] == wmhints->icon_window) {
            children[j] = None;

            break;
          }

      XFree(wmhints);
    }
  }

  // manage shown windows
  for (i = 0; i < (int) nchild; ++i) {
    if (children[i] == None || (! openbox.validateWindow(children[i])))
      continue;

    XWindowAttributes attrib;
    if (XGetWindowAttributes(getBaseDisplay().getXDisplay(), children[i],
                             &attrib)) {
      if (attrib.override_redirect) continue;

      if (attrib.map_state != IsUnmapped) {
        new OpenboxWindow(openbox, children[i], this);

        OpenboxWindow *win = openbox.searchWindow(children[i]);
        if (win) {
          XMapRequestEvent mre;
          mre.window = children[i];
          win->restoreAttributes();
	  win->mapRequestEvent(&mre);
        }
      }
    }
  }

  XFree(children);
  XFlush(getBaseDisplay().getXDisplay());
}


BScreen::~BScreen(void) {
  if (! managed) return;

  if (geom_pixmap != None)
    image_control->removeImage(geom_pixmap);

  if (geom_window != None)
    XDestroyWindow(getBaseDisplay().getXDisplay(), geom_window);

  removeWorkspaceNames();

  std::for_each(workspacesList.begin(), workspacesList.end(),
                PointerAssassin());
  std::for_each(iconList.begin(), iconList.end(), PointerAssassin());
  std::for_each(netizenList.begin(), netizenList.end(), PointerAssassin());

#ifdef    HAVE_STRFTIME
  if (resource.strftime_format)
    delete [] resource.strftime_format;
#endif // HAVE_STRFTIME

  delete rootmenu;
  delete workspacemenu;
  delete iconmenu;
  delete configmenu;

#ifdef    SLIT
  delete slit;
#endif // SLIT

  delete toolbar;
  delete image_control;

  if (resource.wstyle.fontset)
    XFreeFontSet(getBaseDisplay().getXDisplay(), resource.wstyle.fontset);
  if (resource.mstyle.t_fontset)
    XFreeFontSet(getBaseDisplay().getXDisplay(), resource.mstyle.t_fontset);
  if (resource.mstyle.f_fontset)
    XFreeFontSet(getBaseDisplay().getXDisplay(), resource.mstyle.f_fontset);
  if (resource.tstyle.fontset)
    XFreeFontSet(getBaseDisplay().getXDisplay(), resource.tstyle.fontset);

  if (resource.wstyle.font)
    XFreeFont(getBaseDisplay().getXDisplay(), resource.wstyle.font);
  if (resource.mstyle.t_font)
    XFreeFont(getBaseDisplay().getXDisplay(), resource.mstyle.t_font);
  if (resource.mstyle.f_font)
    XFreeFont(getBaseDisplay().getXDisplay(), resource.mstyle.f_font);
  if (resource.tstyle.font)
    XFreeFont(getBaseDisplay().getXDisplay(), resource.tstyle.font);
  if (resource.root_command != NULL)
    delete [] resource.root_command;

  XFreeGC(getBaseDisplay().getXDisplay(), opGC);

  XFreeGC(getBaseDisplay().getXDisplay(),
	  resource.wstyle.l_text_focus_gc);
  XFreeGC(getBaseDisplay().getXDisplay(),
	  resource.wstyle.l_text_unfocus_gc);
  XFreeGC(getBaseDisplay().getXDisplay(),
	  resource.wstyle.b_pic_focus_gc);
  XFreeGC(getBaseDisplay().getXDisplay(),
	  resource.wstyle.b_pic_unfocus_gc);

  XFreeGC(getBaseDisplay().getXDisplay(),
	  resource.mstyle.t_text_gc);
  XFreeGC(getBaseDisplay().getXDisplay(),
	  resource.mstyle.f_text_gc);
  XFreeGC(getBaseDisplay().getXDisplay(),
	  resource.mstyle.h_text_gc);
  XFreeGC(getBaseDisplay().getXDisplay(),
	  resource.mstyle.d_text_gc);
  XFreeGC(getBaseDisplay().getXDisplay(),
	  resource.mstyle.hilite_gc);

  XFreeGC(getBaseDisplay().getXDisplay(),
	  resource.tstyle.l_text_gc);
  XFreeGC(getBaseDisplay().getXDisplay(),
	  resource.tstyle.w_text_gc);
  XFreeGC(getBaseDisplay().getXDisplay(),
	  resource.tstyle.c_text_gc);
  XFreeGC(getBaseDisplay().getXDisplay(),
	  resource.tstyle.b_pic_gc);
}


Rect BScreen::availableArea() const {
  // the following code is temporary and will be taken care of by Screen in the
  // future (with the NETWM 'strut')
  Rect space(0, 0, size().w(), size().h());
  if (!resource.full_max) {
#ifdef    SLIT
    int slit_x = slit->autoHide() ? slit->hiddenOrigin().x() : slit->area().x(),
    slit_y = slit->autoHide() ? slit->hiddenOrigin().y() : slit->area().y();
    unsigned int tbarh = resource.hide_toolbar ? 0 :
      toolbar->getExposedHeight() + resource.border_width * 2;
    bool tbartop;
    switch (toolbar->placement()) {
    case Toolbar::TopLeft:
    case Toolbar::TopCenter:
    case Toolbar::TopRight:
      tbartop = true;
      break;
    case Toolbar::BottomLeft:
    case Toolbar::BottomCenter:
    case Toolbar::BottomRight:
      tbartop = false;
      break;
    default:
      ASSERT(false);      // unhandled placement
    }
    if ((slit->direction() == Slit::Horizontal &&
         (slit->placement() == Slit::TopLeft ||
          slit->placement() == Slit::TopRight)) ||
        slit->placement() == Slit::TopCenter) {
      // exclude top
      if (tbartop && slit_y + slit->area().h() < tbarh) {
        space.setY(space.y() + tbarh);
        space.setH(space.h() - tbarh);
      } else {
        space.setY(space.y() + (slit_y + slit->area().h() +
                                resource.border_width * 2));
        space.setH(space.h() - (slit_y + slit->area().h() +
                                resource.border_width * 2));
        if (!tbartop)
          space.setH(space.h() - tbarh);
      }
    } else if ((slit->direction() == Slit::Vertical &&
                (slit->placement() == Slit::TopRight ||
                 slit->placement() == Slit::BottomRight)) ||
               slit->placement() == Slit::CenterRight) {
      // exclude right
      space.setW(space.w() - (size().w() - slit_x));
      if (tbartop)
        space.setY(space.y() + tbarh);
      space.setH(space.h() - tbarh);
    } else if ((slit->direction() == Slit::Horizontal &&
                (slit->placement() == Slit::BottomLeft ||
                 slit->placement() == Slit::BottomRight)) ||
               slit->placement() == Slit::BottomCenter) {
      // exclude bottom
      if (!tbartop && (size().h() - slit_y) < tbarh) {
        space.setH(space.h() - tbarh);
      } else {
        space.setH(space.h() - (size().h() - slit_y));
        if (tbartop) {
          space.setY(space.y() + tbarh);
          space.setH(space.h() - tbarh);
        }
      }
    } else {// if ((slit->direction() == Slit::Vertical &&
      //      (slit->placement() == Slit::TopLeft ||
      //       slit->placement() == Slit::BottomLeft)) ||
      //     slit->placement() == Slit::CenterLeft)
      // exclude left
      space.setX(slit_x + slit->area().w() +
                 resource.border_width * 2);
      space.setW(space.w() - (slit_x + slit->area().w() +
                              resource.border_width * 2));
      if (tbartop)
        space.setY(space.y() + tbarh);
      space.setH(space.h() - tbarh);
    }
#else // !SLIT
    int tbarh = resource.hide_toolbar() ? 0 :
      toolbar->getExposedHeight() + resource.border_width * 2;
    switch (toolbar->placement()) {
    case Toolbar::TopLeft:
    case Toolbar::TopCenter:
    case Toolbar::TopRight:
      space.setY(toolbar->getExposedHeight());
      space.setH(space.h() - toolbar->getExposedHeight());
      break;
    case Toolbar::BottomLeft:
    case Toolbar::BottomCenter:
    case Toolbar::BottomRight:
      space.setH(space.h() - tbarh);
      break;
    default:
      ASSERT(false);      // unhandled placement
    }
#endif // SLIT
  }
  return space;
}


void BScreen::readDatabaseTexture(const char *rname, const char *rclass,
				  BTexture *texture,
				  unsigned long default_pixel)
{
  std::string s;
  
  if (resource.styleconfig.getValue(rname, rclass, s))
    image_control->parseTexture(texture, s.c_str());
  else
    texture->setTexture(BImage_Solid | BImage_Flat);

  if (texture->getTexture() & BImage_Solid) {
    int clen = strlen(rclass) + 32, nlen = strlen(rname) + 32;

    char *colorclass = new char[clen], *colorname = new char[nlen];

    sprintf(colorclass, "%s.Color", rclass);
    sprintf(colorname,  "%s.color", rname);

    readDatabaseColor(colorname, colorclass, texture->getColor(),
		      default_pixel);

#ifdef    INTERLACE
    sprintf(colorclass, "%s.ColorTo", rclass);
    sprintf(colorname,  "%s.colorTo", rname);

    readDatabaseColor(colorname, colorclass, texture->getColorTo(),
                      default_pixel);
#endif // INTERLACE

    delete [] colorclass;
    delete [] colorname;

    if ((! texture->getColor()->isAllocated()) ||
	(texture->getTexture() & BImage_Flat))
      return;

    XColor xcol;

    xcol.red = (unsigned int) (texture->getColor()->getRed() +
			       (texture->getColor()->getRed() >> 1));
    if (xcol.red >= 0xff) xcol.red = 0xffff;
    else xcol.red *= 0xff;
    xcol.green = (unsigned int) (texture->getColor()->getGreen() +
				 (texture->getColor()->getGreen() >> 1));
    if (xcol.green >= 0xff) xcol.green = 0xffff;
    else xcol.green *= 0xff;
    xcol.blue = (unsigned int) (texture->getColor()->getBlue() +
				(texture->getColor()->getBlue() >> 1));
    if (xcol.blue >= 0xff) xcol.blue = 0xffff;
    else xcol.blue *= 0xff;

    if (! XAllocColor(getBaseDisplay().getXDisplay(),
		      getColormap(), &xcol))
      xcol.pixel = 0;

    texture->getHiColor()->setPixel(xcol.pixel);

    xcol.red =
      (unsigned int) ((texture->getColor()->getRed() >> 2) +
		      (texture->getColor()->getRed() >> 1)) * 0xff;
    xcol.green =
      (unsigned int) ((texture->getColor()->getGreen() >> 2) +
		      (texture->getColor()->getGreen() >> 1)) * 0xff;
    xcol.blue =
      (unsigned int) ((texture->getColor()->getBlue() >> 2) +
		      (texture->getColor()->getBlue() >> 1)) * 0xff;

    if (! XAllocColor(getBaseDisplay().getXDisplay(),
		      getColormap(), &xcol))
      xcol.pixel = 0;

    texture->getLoColor()->setPixel(xcol.pixel);
  } else if (texture->getTexture() & BImage_Gradient) {
    int clen = strlen(rclass) + 10, nlen = strlen(rname) + 10;

    char *colorclass = new char[clen], *colorname = new char[nlen],
      *colortoclass = new char[clen], *colortoname = new char[nlen];

    sprintf(colorclass, "%s.Color", rclass);
    sprintf(colorname,  "%s.color", rname);

    sprintf(colortoclass, "%s.ColorTo", rclass);
    sprintf(colortoname,  "%s.colorTo", rname);

    readDatabaseColor(colorname, colorclass, texture->getColor(),
		      default_pixel);
    readDatabaseColor(colortoname, colortoclass, texture->getColorTo(),
		      default_pixel);

    delete [] colorclass;
    delete [] colorname;
    delete [] colortoclass;
    delete [] colortoname;
  }
}


void BScreen::readDatabaseColor(const char *rname, const  char *rclass,
                                BColor *color, unsigned long default_pixel)
{
  std::string s;
  
  if (resource.styleconfig.getValue(rname, rclass, s))
    image_control->parseColor(color, s.c_str());
  else {
    // parsing with no color std::string just deallocates the color, if it has
    // been previously allocated
    image_control->parseColor(color);
    color->setPixel(default_pixel);
  }
}


void BScreen::readDatabaseFontSet(const char *rname, const char *rclass,
				  XFontSet *fontset) {
  if (! fontset) return;

  static char *defaultFont = "fixed";
  bool load_default = false;
  std::string s;

  if (*fontset)
    XFreeFontSet(getBaseDisplay().getXDisplay(), *fontset);

  if (resource.styleconfig.getValue(rname, rclass, s)) {
    if (! (*fontset = createFontSet(s.c_str())))
      load_default = true;
  } else
    load_default = true;

  if (load_default) {
    *fontset = createFontSet(defaultFont);

    if (! *fontset) {
      fprintf(stderr, i18n->getMessage(ScreenSet, ScreenDefaultFontLoadFail,
                       "BScreen::LoadStyle(): couldn't load default font.\n"));
      exit(2);
    }
  }
}


void BScreen::readDatabaseFont(const char *rname, const char *rclass,
                               XFontStruct **font) {
  if (! font) return;

  static char *defaultFont = "fixed";
  bool load_default = false;
  std::string s;

  if (*font)
    XFreeFont(getBaseDisplay().getXDisplay(), *font);

  if (resource.styleconfig.getValue(rname, rclass, s)) {
    if ((*font = XLoadQueryFont(getBaseDisplay().getXDisplay(),
				s.c_str())) == NULL) {
      fprintf(stderr, i18n->getMessage(ScreenSet, ScreenFontLoadFail,
			 "BScreen::LoadStyle(): couldn't load font '%s'\n"),
	      s.c_str());
      load_default = true;
    }
  } else
    load_default = true;

  if (load_default) {
    if ((*font = XLoadQueryFont(getBaseDisplay().getXDisplay(),
				defaultFont)) == NULL) {
      fprintf(stderr, i18n->getMessage(ScreenSet, ScreenDefaultFontLoadFail,
	         "BScreen::LoadStyle(): couldn't load default font.\n"));
      exit(2);
    }
  }
}


XFontSet BScreen::createFontSet(const char *fontname) {
  XFontSet fs;
  char **missing, *def = "-";
  int nmissing, pixel_size = 0, buf_size = 0;
  char weight[FONT_ELEMENT_SIZE], slant[FONT_ELEMENT_SIZE];

  fs = XCreateFontSet(getBaseDisplay().getXDisplay(),
		      fontname, &missing, &nmissing, &def);
  if (fs && (! nmissing)) return fs;

#ifdef    HAVE_SETLOCALE
  if (! fs) {
    if (nmissing) XFreeStringList(missing);

    setlocale(LC_CTYPE, "C");
    fs = XCreateFontSet(getBaseDisplay().getXDisplay(), fontname,
			&missing, &nmissing, &def);
    setlocale(LC_CTYPE, "");
  }
#endif // HAVE_SETLOCALE

  if (fs) {
    XFontStruct **fontstructs;
    char **fontnames;
    XFontsOfFontSet(fs, &fontstructs, &fontnames);
    fontname = fontnames[0];
  }

  getFontElement(fontname, weight, FONT_ELEMENT_SIZE,
		 "-medium-", "-bold-", "-demibold-", "-regular-", NULL);
  getFontElement(fontname, slant, FONT_ELEMENT_SIZE,
		 "-r-", "-i-", "-o-", "-ri-", "-ro-", NULL);
  getFontSize(fontname, &pixel_size);

  if (! strcmp(weight, "*")) strncpy(weight, "medium", FONT_ELEMENT_SIZE);
  if (! strcmp(slant, "*")) strncpy(slant, "r", FONT_ELEMENT_SIZE);
  if (pixel_size < 3) pixel_size = 3;
  else if (pixel_size > 97) pixel_size = 97;

  buf_size = strlen(fontname) + (FONT_ELEMENT_SIZE * 2) + 64;
  char *pattern2 = new char[buf_size];
  snprintf(pattern2, buf_size - 1,
	   "%s,"
	   "-*-*-%s-%s-*-*-%d-*-*-*-*-*-*-*,"
	   "-*-*-*-*-*-*-%d-*-*-*-*-*-*-*,*",
	   fontname, weight, slant, pixel_size, pixel_size);
  fontname = pattern2;

  if (nmissing) XFreeStringList(missing);
  if (fs) XFreeFontSet(getBaseDisplay().getXDisplay(), fs);

  fs = XCreateFontSet(getBaseDisplay().getXDisplay(), fontname,
		      &missing, &nmissing, &def);
  delete [] pattern2;

  return fs;
}


void BScreen::setSloppyFocus(bool b) {
  resource.sloppy_focus = b;
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".focusModel" << ends;
  config.setValue(s.str(),
                  (resource.sloppy_focus ?
                  (resource.auto_raise ? "AutoRaiseSloppyFocus" : "SloppyFocus")
                  : "ClickToFocus"));
  s.rdbuf()->freeze(0);
}


void BScreen::setAutoRaise(bool a) {
  resource.auto_raise = a;
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".focusModel" << ends;
  config.setValue(s.str(),
                  (resource.sloppy_focus ?
                  (resource.auto_raise ? "AutoRaiseSloppyFocus" : "SloppyFocus")
                  : "ClickToFocus"));
  s.rdbuf()->freeze(0);
}


void BScreen::setImageDither(bool d, bool reconfig) {
  image_control->setDither(d);
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".imageDither" << ends;
  config.setValue(s.str(), imageDither());
  if (reconfig)
    reconfigure();
  s.rdbuf()->freeze(0);
}


void BScreen::setOpaqueMove(bool o) {
  resource.opaque_move = o;
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".opaqueMove" << ends;
  config.setValue(s.str(), resource.opaque_move);
  s.rdbuf()->freeze(0);
}


void BScreen::setFullMax(bool f) {
  resource.full_max = f;
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".fullMaximization" << ends;
  config.setValue(s.str(), resource.full_max);
  s.rdbuf()->freeze(0);
}


void BScreen::setFocusNew(bool f) {
  resource.focus_new = f;
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".focusNewWindows" << ends;
  config.setValue(s.str(), resource.focus_new);
  s.rdbuf()->freeze(0);
}


void BScreen::setFocusLast(bool f) {
  resource.focus_last = f;
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".focusLastWindow" << ends;
  config.setValue(s.str(), resource.focus_last);
  s.rdbuf()->freeze(0);
}


void BScreen::setWindowZones(int z) {
  resource.zones = z;
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".windowZones" << ends;
  config.setValue(s.str(), resource.zones);
  s.rdbuf()->freeze(0);
}


void BScreen::setWorkspaceCount(int w) {
  resource.workspaces = w;
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".workspaces" << ends;
  config.setValue(s.str(), resource.workspaces);
  s.rdbuf()->freeze(0);
}


void BScreen::setPlacementPolicy(int p) {
  resource.placement_policy = p;
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".windowPlacement" << ends;
  const char *placement;
  switch (resource.placement_policy) {
  case CascadePlacement: placement = "CascadePlacement"; break;
  case BestFitPlacement: placement = "BestFitPlacement"; break;
  case ColSmartPlacement: placement = "ColSmartPlacement"; break;
  case UnderMousePlacement: placement = "UnderMousePlacement"; break;
  case ClickMousePlacement: placement = "ClickMousePlacement"; break;
  default:
  case RowSmartPlacement: placement = "RowSmartPlacement"; break;
  }
  config.setValue(s.str(), placement);
  s.rdbuf()->freeze(0);
}


void BScreen::setEdgeSnapThreshold(int t) {
  resource.edge_snap_threshold = t;
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".edgeSnapThreshold" << ends;
  config.setValue(s.str(), resource.edge_snap_threshold);
  s.rdbuf()->freeze(0);
}


void BScreen::setRowPlacementDirection(int d) {
  resource.row_direction = d;
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".rowPlacementDirection" << ends;
  config.setValue(s.str(),
                  resource.row_direction == LeftRight ?
                  "LeftToRight" : "RightToLeft");
  s.rdbuf()->freeze(0);
}


void BScreen::setColPlacementDirection(int d) {
  resource.col_direction = d;
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".colPlacementDirection" << ends;
  config.setValue(s.str(),
                  resource.col_direction == TopBottom ?
                  "TopToBottom" : "BottomToTop");
  s.rdbuf()->freeze(0);
}


void BScreen::setRootCommand(const char *cmd) {
if (resource.root_command != NULL)
    delete [] resource.root_command;
  if (cmd != NULL)
    resource.root_command = bstrdup(cmd);
  else
    resource.root_command = NULL;
  // this doesn't save to the Resources config because it can't be changed
  // inside Openbox, and this way we dont add an empty command which would over-
  // ride the styles command when none has been specified
}


#ifdef    HAVE_STRFTIME
void BScreen::setStrftimeFormat(const char *f) {
  if (resource.strftime_format != NULL)
    delete [] resource.strftime_format;

  resource.strftime_format = bstrdup(f);
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".strftimeFormat" << ends;
  config.setValue(s.str(), resource.strftime_format);
  s.rdbuf()->freeze(0);
}

#else // !HAVE_STRFTIME

void BScreen::setDateFormat(int f) {
  resource.date_format = f;
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".dateFormat" << ends;
  config.setValue(s.str(), resource.date_format == B_EuropeanDate ?
                  "European" : "American");
  s.rdbuf()->freeze(0);
}


void BScreen::setClock24Hour(Bool c) {
  resource.clock24hour = c;
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".clockFormat" << ends;
  config.setValue(s.str(), resource.clock24hour ? 24 : 12);
  s.rdbuf()->freeze(0);
}
#endif // HAVE_STRFTIME


void BScreen::setHideToolbar(bool b) {
  resource.hide_toolbar = b;
  if (resource.hide_toolbar)
    getToolbar()->unMapToolbar();
  else
    getToolbar()->mapToolbar();
  std::ostrstream s;
  s << "session.screen" << getScreenNumber() << ".hideToolbar" << ends;
  config.setValue(s.str(), resource.hide_toolbar ? "True" : "False");
  s.rdbuf()->freeze(0);
}


void BScreen::saveWorkspaceNames() {
  std::ostrstream rc, names;

  wkspList::iterator it;
  wkspList::iterator last = workspacesList.end() - 1;
  for (it = workspacesList.begin(); it != workspacesList.end(); ++it) {
    names << (*it)->getName();
    if (it != last)
      names << ",";
  }
  names << ends;

  rc << "session.screen" << getScreenNumber() << ".workspaceNames" << ends;
  config.setValue(rc.str(), names.str());
  rc.rdbuf()->freeze(0);
  names.rdbuf()->freeze(0);
}

void BScreen::save() {
  setSloppyFocus(resource.sloppy_focus);
  setAutoRaise(resource.auto_raise);
  setImageDither(imageDither(), false);
  setOpaqueMove(resource.opaque_move);
  setFullMax(resource.full_max);
  setFocusNew(resource.focus_new);
  setFocusLast(resource.focus_last);
  setWindowZones(resource.zones);
  setWorkspaceCount(resource.workspaces);
  setPlacementPolicy(resource.placement_policy);
  setEdgeSnapThreshold(resource.edge_snap_threshold);
  setRowPlacementDirection(resource.row_direction);
  setColPlacementDirection(resource.col_direction);
  setRootCommand(resource.root_command);
#ifdef    HAVE_STRFTIME
  // it deletes the current value before setting the new one, so we have to
  // duplicate the current value.
  std::string s = resource.strftime_format;
  setStrftimeFormat(s.c_str()); 
#else // !HAVE_STRFTIME
  setDateFormat(resource.date_format);
  setClock24Hour(resource.clock24hour);
#endif // HAVE_STRFTIME
  setHideToolbar(resource.hide_toolbar);
    
  toolbar->save();
#ifdef    SLIT
  slit->save();
#endif // SLIT
}


void BScreen::load() {
  std::ostrstream rscreen, rname, rclass;
  std::string s;
  bool b;
  long l;
  rscreen << "session.screen" << getScreenNumber() << '.' << ends;

  rname << rscreen.str() << "hideToolbar" << ends;
  rclass << rscreen.str() << "HideToolbar" << ends;
  if (config.getValue(rname.str(), rclass.str(), b))
    resource.hide_toolbar = b;
  else
    resource.hide_toolbar = false;
  Toolbar *t = getToolbar();
  if (t != NULL) {
    if (resource.hide_toolbar)
      t->unMapToolbar();
    else
      t->mapToolbar();
  }

  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "fullMaximization" << ends;
  rclass << rscreen.str() << "FullMaximization" << ends;
  if (config.getValue(rname.str(), rclass.str(), b))
    resource.full_max = b;
  else
    resource.full_max = false;

  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "focusNewWindows" << ends;
  rclass << rscreen.str() << "FocusNewWindows" << ends;
  if (config.getValue(rname.str(), rclass.str(), b))
    resource.focus_new = b;
  else
    resource.focus_new = false;

  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "focusLastWindow" << ends;
  rclass << rscreen.str() << "FocusLastWindow" << ends;
  if (config.getValue(rname.str(), rclass.str(), b))
    resource.focus_last = b;
  else
    resource.focus_last = false;

  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "rowPlacementDirection" << ends;
  rclass << rscreen.str() << "RowPlacementDirection" << ends;
  if (config.getValue(rname.str(), rclass.str(), s)) {
    if (0 == strncasecmp(s.c_str(), "RightToLeft", s.length()))
      resource.row_direction = RightLeft;
    else //if (0 == strncasecmp(s.c_str(), "LeftToRight", s.length()))
      resource.row_direction = LeftRight;
  } else
    resource.row_direction = LeftRight;

  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "colPlacementDirection" << ends;
  rclass << rscreen.str() << "ColPlacementDirection" << ends;
  if (config.getValue(rname.str(), rclass.str(), s)) {
    if (0 == strncasecmp(s.c_str(), "BottomToTop", s.length()))
      resource.col_direction = BottomTop;
    else //if (0 == strncasecmp(s.c_str(), "TopToBottom", s.length()))
      resource.col_direction = TopBottom;
  } else
    resource.col_direction = TopBottom;

  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "workspaces" << ends;
  rclass << rscreen.str() << "Workspaces" << ends;
  if (config.getValue(rname.str(), rclass.str(), l)) {
    resource.workspaces = l;
  }  else
    resource.workspaces = 1;

  removeWorkspaceNames();
  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "workspaceNames" << ends;
  rclass << rscreen.str() << "WorkspaceNames" << ends;
  if (config.getValue(rname.str(), rclass.str(), s)) {
    std::string::const_iterator it = s.begin(), end = s.end();
    while(1) {
      std::string::const_iterator tmp = it;// current string.begin()
      it = std::find(tmp, end, ',');       // look for comma between tmp and end
      std::string name(tmp, it);           // name = s[tmp:it]
      addWorkspaceName(name.c_str());
      if (it == end)
        break;
      ++it;
    }
  }
  
  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "focusModel" << ends;
  rclass << rscreen.str() << "FocusModel" << ends;
  if (config.getValue(rname.str(), rclass.str(), s)) {
    if (0 == strncasecmp(s.c_str(), "ClickToFocus", s.length())) {
      resource.auto_raise = false;
      resource.sloppy_focus = false;
    } else if (0 == strncasecmp(s.c_str(), "AutoRaiseSloppyFocus",
                                s.length())) {
      resource.sloppy_focus = true;
      resource.auto_raise = true;
    } else { //if (0 == strncasecmp(s.c_str(), "SloppyFocus", s.length())) {
      resource.sloppy_focus = true;
      resource.auto_raise = false;
    }
  } else {
    resource.sloppy_focus = true;
    resource.auto_raise = false;
  }

  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "windowZones" << ends;
  rclass << rscreen.str() << "WindowZones" << ends;
  if (config.getValue(rname.str(), rclass.str(), l))
    resource.zones = (l == 1 || l == 2 || l == 4) ? l : 1;
  else
    resource.zones = 4;

  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "windowPlacement" << ends;
  rclass << rscreen.str() << "WindowPlacement" << ends;
  if (config.getValue(rname.str(), rclass.str(), s)) {
    if (0 == strncasecmp(s.c_str(), "RowSmartPlacement", s.length()))
      resource.placement_policy = RowSmartPlacement;
    else if (0 == strncasecmp(s.c_str(), "ColSmartPlacement", s.length()))
      resource.placement_policy = ColSmartPlacement;
    else if (0 == strncasecmp(s.c_str(), "BestFitPlacement", s.length()))
      resource.placement_policy = BestFitPlacement;
    else if (0 == strncasecmp(s.c_str(), "UnderMousePlacement", s.length()))
      resource.placement_policy = UnderMousePlacement;
    else if (0 == strncasecmp(s.c_str(), "ClickMousePlacement", s.length()))
      resource.placement_policy = ClickMousePlacement;
    else //if (0 == strncasecmp(s.c_str(), "CascadePlacement", s.length()))
      resource.placement_policy = CascadePlacement;
  } else
    resource.placement_policy = CascadePlacement;

#ifdef    HAVE_STRFTIME
  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "strftimeFormat" << ends;
  rclass << rscreen.str() << "StrftimeFormat" << ends;

  if (resource.strftime_format != NULL)
    delete [] resource.strftime_format;

  if (config.getValue(rname.str(), rclass.str(), s))
    resource.strftime_format = bstrdup(s.c_str());
  else
    resource.strftime_format = bstrdup("%I:%M %p");
#else // !HAVE_STRFTIME
  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "dateFormat" << ends;
  rclass << rscreen.str() << "DateFormat" << ends;
  if (config.getValue(rname.str(), rclass.str(), s)) {
    if (strncasecmp(s.c_str(), "European", s.length()))
      resource.date_format = B_EuropeanDate;
    else //if (strncasecmp(s.c_str(), "American", s.length()))
      resource.date_format = B_AmericanDate;
  } else
    resource.date_format = B_AmericanDate;

  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "clockFormat" << ends;
  rclass << rscreen.str() << "ClockFormat" << ends;
  if (config.getValue(rname.str(), rclass.str(), l)) {
    if (clock == 24)
      resource.clock24hour = true;
    else if (clock == 12)
      resource.clock24hour =  false;
  } else
    resource.clock24hour =  false;
#endif // HAVE_STRFTIME

  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "edgeSnapThreshold" << ends;
  rclass << rscreen.str() << "EdgeSnapThreshold" << ends;
  if (config.getValue(rname.str(), rclass.str(), l))
    resource.edge_snap_threshold = l;
  else
    resource.edge_snap_threshold = 4;

  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "imageDither" << ends;
  rclass << rscreen.str() << "ImageDither" << ends;
  if (config.getValue(rname.str(), rclass.str(), b))
    image_control->setDither(b);
  else
    image_control->setDither(true);

  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "rootCommand" << ends;
  rclass << rscreen.str() << "RootCommand" << ends;

  if (resource.root_command != NULL)
    delete [] resource.root_command;
  
  if (config.getValue(rname.str(), rclass.str(), s))
    resource.root_command = bstrdup(s.c_str());
  else
    resource.root_command = NULL;

  rname.seekp(0); rclass.seekp(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
  rname << rscreen.str() << "opaqueMove" << ends;
  rclass << rscreen.str() << "OpaqueMove" << ends;
  if (config.getValue(rname.str(), rclass.str(), b))
    resource.opaque_move = b;
  else
    resource.opaque_move = false;

  rscreen.rdbuf()->freeze(0);
  rname.rdbuf()->freeze(0); rclass.rdbuf()->freeze(0);
}

void BScreen::reconfigure(void) {
  load();
  toolbar->load();
#ifdef    SLIT
  slit->load();
#endif // SLIT
  LoadStyle();

  XGCValues gcv;
  unsigned long gc_value_mask = GCForeground;
  if (! i18n->multibyte()) gc_value_mask |= GCFont;

  gcv.foreground = WhitePixel(getBaseDisplay().getXDisplay(),
			      getScreenNumber());
  gcv.function = GXinvert;
  gcv.subwindow_mode = IncludeInferiors;
  XChangeGC(getBaseDisplay().getXDisplay(), opGC,
	    GCForeground | GCFunction | GCSubwindowMode, &gcv);

  gcv.foreground = resource.wstyle.l_text_focus.getPixel();
  if (resource.wstyle.font)
    gcv.font = resource.wstyle.font->fid;
  XChangeGC(getBaseDisplay().getXDisplay(), resource.wstyle.l_text_focus_gc,
	    gc_value_mask, &gcv);

  gcv.foreground = resource.wstyle.l_text_unfocus.getPixel();
  XChangeGC(getBaseDisplay().getXDisplay(), resource.wstyle.l_text_unfocus_gc,
	    gc_value_mask, &gcv);

  gcv.foreground = resource.wstyle.b_pic_focus.getPixel();
  XChangeGC(getBaseDisplay().getXDisplay(), resource.wstyle.b_pic_focus_gc,
	    GCForeground, &gcv);

  gcv.foreground = resource.wstyle.b_pic_unfocus.getPixel();
  XChangeGC(getBaseDisplay().getXDisplay(), resource.wstyle.b_pic_unfocus_gc,
	    GCForeground, &gcv);

  gcv.foreground = resource.mstyle.t_text.getPixel();
  if (resource.mstyle.t_font)
    gcv.font = resource.mstyle.t_font->fid;
  XChangeGC(getBaseDisplay().getXDisplay(), resource.mstyle.t_text_gc,
	    gc_value_mask, &gcv);

  gcv.foreground = resource.mstyle.f_text.getPixel();
  if (resource.mstyle.f_font)
    gcv.font = resource.mstyle.f_font->fid;
  XChangeGC(getBaseDisplay().getXDisplay(), resource.mstyle.f_text_gc,
	    gc_value_mask, &gcv);

  gcv.foreground = resource.mstyle.h_text.getPixel();
  XChangeGC(getBaseDisplay().getXDisplay(), resource.mstyle.h_text_gc,
	    gc_value_mask, &gcv);

  gcv.foreground = resource.mstyle.d_text.getPixel();
  XChangeGC(getBaseDisplay().getXDisplay(), resource.mstyle.d_text_gc,
	    gc_value_mask, &gcv);

  gcv.foreground = resource.mstyle.hilite.getColor()->getPixel();
  XChangeGC(getBaseDisplay().getXDisplay(), resource.mstyle.hilite_gc,
	    gc_value_mask, &gcv);

  gcv.foreground = resource.tstyle.l_text.getPixel();
  if (resource.tstyle.font)
    gcv.font = resource.tstyle.font->fid;
  XChangeGC(getBaseDisplay().getXDisplay(), resource.tstyle.l_text_gc,
	    gc_value_mask, &gcv);

  gcv.foreground = resource.tstyle.w_text.getPixel();
  XChangeGC(getBaseDisplay().getXDisplay(), resource.tstyle.w_text_gc,
	    gc_value_mask, &gcv);

  gcv.foreground = resource.tstyle.c_text.getPixel();
  XChangeGC(getBaseDisplay().getXDisplay(), resource.tstyle.c_text_gc,
	    gc_value_mask, &gcv);

  gcv.foreground = resource.tstyle.b_pic.getPixel();
  XChangeGC(getBaseDisplay().getXDisplay(), resource.tstyle.b_pic_gc,
	    gc_value_mask, &gcv);

  const char *s = i18n->getMessage(ScreenSet, ScreenPositionLength,
				   "0: 0000 x 0: 0000");
  int l = strlen(s);

  if (i18n->multibyte()) {
    XRectangle ink, logical;
    XmbTextExtents(resource.wstyle.fontset, s, l, &ink, &logical);
    geom_w = logical.width;

    geom_h = resource.wstyle.fontset_extents->max_ink_extent.height;
  } else {
    geom_w = XTextWidth(resource.wstyle.font, s, l);

    geom_h = resource.wstyle.font->ascent +
	     resource.wstyle.font->descent; 
  }

  geom_w += (resource.bevel_width * 2);
  geom_h += (resource.bevel_width * 2);

  Pixmap tmp = geom_pixmap;
  if (resource.wstyle.l_focus.getTexture() & BImage_ParentRelative) {
    if (resource.wstyle.t_focus.getTexture() ==
	                              (BImage_Flat | BImage_Solid)) {
      geom_pixmap = None;
      XSetWindowBackground(getBaseDisplay().getXDisplay(), geom_window,
			 resource.wstyle.t_focus.getColor()->getPixel());
    } else {
      geom_pixmap = image_control->renderImage(geom_w, geom_h,
					       &resource.wstyle.t_focus);
      XSetWindowBackgroundPixmap(getBaseDisplay().getXDisplay(),
				 geom_window, geom_pixmap);
    }
  } else {
    if (resource.wstyle.l_focus.getTexture() ==
	                              (BImage_Flat | BImage_Solid)) {
      geom_pixmap = None;
      XSetWindowBackground(getBaseDisplay().getXDisplay(), geom_window,
			 resource.wstyle.l_focus.getColor()->getPixel());
    } else {
      geom_pixmap = image_control->renderImage(geom_w, geom_h,
					       &resource.wstyle.l_focus);
      XSetWindowBackgroundPixmap(getBaseDisplay().getXDisplay(),
				 geom_window, geom_pixmap);
    }
  }
  if (tmp) image_control->removeImage(tmp);

  XSetWindowBorderWidth(getBaseDisplay().getXDisplay(), geom_window,
                        resource.border_width);
  XSetWindowBorder(getBaseDisplay().getXDisplay(), geom_window,
                   resource.border_color.getPixel());

  workspacemenu->reconfigure();
  iconmenu->reconfigure();

  {
    int remember_sub = rootmenu->getCurrentSubmenu();
    InitMenu();
    raiseWindows(0, 0);
    rootmenu->reconfigure();
    rootmenu->drawSubmenu(remember_sub);
  }

  configmenu->reconfigure();

  toolbar->reconfigure();

#ifdef    SLIT
  slit->reconfigure();
#endif // SLIT

  std::for_each(workspacesList.begin(), workspacesList.end(),
                std::mem_fun(&Workspace::reconfigure));

  for (winList::iterator it = iconList.begin(); it != iconList.end(); ++it)
    if ((*it)->validateClient())
      (*it)->reconfigure();

  image_control->timeout();
}


void BScreen::rereadMenu(void) {
  InitMenu();
  raiseWindows(0, 0);

  rootmenu->reconfigure();
}


void BScreen::removeWorkspaceNames(void) {
  workspaceNames.clear();
}


void BScreen::LoadStyle(void) {
  Resource &conf = resource.styleconfig;
  
  const char *sfile = openbox.getStyleFilename();
  bool loaded = false;
  if (sfile != NULL) {
    conf.setFile(sfile);
    loaded = conf.load();
  }
  if (!loaded) {
    conf.setFile(DEFAULTSTYLE);
    if (!conf.load()) {
      fprintf(stderr, i18n->getMessage(ScreenSet, ScreenDefaultStyleLoadFail,
                                       "BScreen::LoadStyle(): couldn't load "
                                       "default style.\n"));
      exit(2);
    }
  }

  std::string s;
  long l;
  
  // load fonts/fontsets

  if (i18n->multibyte()) {
    readDatabaseFontSet("window.font", "Window.Font",
			&resource.wstyle.fontset);
    readDatabaseFontSet("toolbar.font", "Toolbar.Font",
			&resource.tstyle.fontset);
    readDatabaseFontSet("menu.title.font", "Menu.Title.Font",
			&resource.mstyle.t_fontset);
    readDatabaseFontSet("menu.frame.font", "Menu.Frame.Font",
			&resource.mstyle.f_fontset);

    resource.mstyle.t_fontset_extents =
      XExtentsOfFontSet(resource.mstyle.t_fontset);
    resource.mstyle.f_fontset_extents =
      XExtentsOfFontSet(resource.mstyle.f_fontset);
    resource.tstyle.fontset_extents =
      XExtentsOfFontSet(resource.tstyle.fontset);
    resource.wstyle.fontset_extents =
      XExtentsOfFontSet(resource.wstyle.fontset);
  } else {
    readDatabaseFont("window.font", "Window.Font",
		     &resource.wstyle.font);
    readDatabaseFont("menu.title.font", "Menu.Title.Font",
		     &resource.mstyle.t_font);
    readDatabaseFont("menu.frame.font", "Menu.Frame.Font",
		     &resource.mstyle.f_font);
    readDatabaseFont("toolbar.font", "Toolbar.Font",
		     &resource.tstyle.font);
  }

  // load window config
  readDatabaseTexture("window.title.focus", "Window.Title.Focus",
		      &resource.wstyle.t_focus,
		      WhitePixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("window.title.unfocus", "Window.Title.Unfocus",
		      &resource.wstyle.t_unfocus,
		      BlackPixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("window.label.focus", "Window.Label.Focus",
		      &resource.wstyle.l_focus,
		      WhitePixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("window.label.unfocus", "Window.Label.Unfocus",
		      &resource.wstyle.l_unfocus,
		      BlackPixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("window.handle.focus", "Window.Handle.Focus",
		      &resource.wstyle.h_focus,
		      WhitePixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("window.handle.unfocus", "Window.Handle.Unfocus",
		      &resource.wstyle.h_unfocus,
		      BlackPixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("window.grip.focus", "Window.Grip.Focus",
                      &resource.wstyle.g_focus,
		      WhitePixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("window.grip.unfocus", "Window.Grip.Unfocus",
                      &resource.wstyle.g_unfocus,
		      BlackPixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("window.button.focus", "Window.Button.Focus",
		      &resource.wstyle.b_focus,
		      WhitePixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("window.button.unfocus", "Window.Button.Unfocus",
		      &resource.wstyle.b_unfocus,
		      BlackPixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("window.button.pressed", "Window.Button.Pressed",
		      &resource.wstyle.b_pressed,
		      BlackPixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseColor("window.frame.focusColor",
		    "Window.Frame.FocusColor",
		    &resource.wstyle.f_focus,
		    WhitePixel(getBaseDisplay().getXDisplay(),
			       getScreenNumber()));
  readDatabaseColor("window.frame.unfocusColor",
		    "Window.Frame.UnfocusColor",
		    &resource.wstyle.f_unfocus,
		    BlackPixel(getBaseDisplay().getXDisplay(),
			       getScreenNumber()));
  readDatabaseColor("window.label.focus.textColor",
		    "Window.Label.Focus.TextColor",
		    &resource.wstyle.l_text_focus,
		    BlackPixel(getBaseDisplay().getXDisplay(),
			       getScreenNumber()));
  readDatabaseColor("window.label.unfocus.textColor",
		    "Window.Label.Unfocus.TextColor",
		    &resource.wstyle.l_text_unfocus,
		    WhitePixel(getBaseDisplay().getXDisplay(),
			       getScreenNumber()));
  readDatabaseColor("window.button.focus.picColor",
		    "Window.Button.Focus.PicColor",
		    &resource.wstyle.b_pic_focus,
		    BlackPixel(getBaseDisplay().getXDisplay(),
			       getScreenNumber()));
  readDatabaseColor("window.button.unfocus.picColor",
		    "Window.Button.Unfocus.PicColor",
		    &resource.wstyle.b_pic_unfocus,
		    WhitePixel(getBaseDisplay().getXDisplay(),
			       getScreenNumber()));

  if (conf.getValue("window.justify", "Window.Justify", s)) {
    if (0 == strncasecmp(s.c_str(), "right", s.length()))
      resource.wstyle.justify = BScreen::RightJustify;
    else if (0 == strncasecmp(s.c_str(), "center", s.length()))
      resource.wstyle.justify = BScreen::CenterJustify;
    else
      resource.wstyle.justify = BScreen::LeftJustify;
  } else
    resource.wstyle.justify = BScreen::LeftJustify;

  // load toolbar config
  readDatabaseTexture("toolbar", "Toolbar",
		      &resource.tstyle.toolbar,
		      BlackPixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("toolbar.label", "Toolbar.Label",
		      &resource.tstyle.label,
		      BlackPixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("toolbar.windowLabel", "Toolbar.WindowLabel",
		      &resource.tstyle.window,
		      BlackPixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("toolbar.button", "Toolbar.Button",
		      &resource.tstyle.button,
		      WhitePixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("toolbar.button.pressed", "Toolbar.Button.Pressed",
		      &resource.tstyle.pressed,
		      BlackPixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("toolbar.clock", "Toolbar.Clock",
		      &resource.tstyle.clock,
		      BlackPixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseColor("toolbar.label.textColor", "Toolbar.Label.TextColor",
		    &resource.tstyle.l_text,
		    WhitePixel(getBaseDisplay().getXDisplay(),
			       getScreenNumber()));
  readDatabaseColor("toolbar.windowLabel.textColor",
		    "Toolbar.WindowLabel.TextColor",
		    &resource.tstyle.w_text,
		    WhitePixel(getBaseDisplay().getXDisplay(),
			       getScreenNumber()));
  readDatabaseColor("toolbar.clock.textColor", "Toolbar.Clock.TextColor",
		    &resource.tstyle.c_text,
		    WhitePixel(getBaseDisplay().getXDisplay(),
			       getScreenNumber()));
  readDatabaseColor("toolbar.button.picColor", "Toolbar.Button.PicColor",
		    &resource.tstyle.b_pic,
		    BlackPixel(getBaseDisplay().getXDisplay(),
			       getScreenNumber()));

  if (conf.getValue("toolbar.justify", "Toolbar.Justify", s)) {
    if (0 == strncasecmp(s.c_str(), "right", s.length()))
      resource.tstyle.justify = BScreen::RightJustify;
    else if (0 == strncasecmp(s.c_str(), "center", s.length()))
      resource.tstyle.justify = BScreen::CenterJustify;
    else
      resource.tstyle.justify = BScreen::LeftJustify;
  } else
    resource.tstyle.justify = BScreen::LeftJustify;

  // load menu config
  readDatabaseTexture("menu.title", "Menu.Title",
		      &resource.mstyle.title,
		      WhitePixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("menu.frame", "Menu.Frame",
		      &resource.mstyle.frame,
		      BlackPixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseTexture("menu.hilite", "Menu.Hilite",
		      &resource.mstyle.hilite,
		      WhitePixel(getBaseDisplay().getXDisplay(),
				 getScreenNumber()));
  readDatabaseColor("menu.title.textColor", "Menu.Title.TextColor",
		    &resource.mstyle.t_text,
		    BlackPixel(getBaseDisplay().getXDisplay(),
			       getScreenNumber()));
  readDatabaseColor("menu.frame.textColor", "Menu.Frame.TextColor",
		    &resource.mstyle.f_text,
		    WhitePixel(getBaseDisplay().getXDisplay(),
			       getScreenNumber()));
  readDatabaseColor("menu.frame.disableColor", "Menu.Frame.DisableColor",
		    &resource.mstyle.d_text,
		    BlackPixel(getBaseDisplay().getXDisplay(),
			       getScreenNumber()));
  readDatabaseColor("menu.hilite.textColor", "Menu.Hilite.TextColor",
		    &resource.mstyle.h_text,
		    BlackPixel(getBaseDisplay().getXDisplay(),
			       getScreenNumber()));

  if (conf.getValue("menu.title.justify", "Menu.Title.Justify", s)) {
    if (0 == strncasecmp(s.c_str(), "right", s.length()))
      resource.mstyle.t_justify = BScreen::RightJustify;
    else if (0 == strncasecmp(s.c_str(), "center", s.length()))
      resource.mstyle.t_justify = BScreen::CenterJustify;
    else
      resource.mstyle.t_justify = BScreen::LeftJustify;
  } else
    resource.mstyle.t_justify = BScreen::LeftJustify;

  if (conf.getValue("menu.frame.justify", "Menu.Frame.Justify", s)) {
    if (0 == strncasecmp(s.c_str(), "right", s.length()))
      resource.mstyle.f_justify = BScreen::RightJustify;
    else if (0 == strncasecmp(s.c_str(), "center", s.length()))
      resource.mstyle.f_justify = BScreen::CenterJustify;
    else
      resource.mstyle.f_justify = BScreen::LeftJustify;
  } else
    resource.mstyle.f_justify = BScreen::LeftJustify;

  if (conf.getValue("menu.bullet", "Menu.Bullet", s)) {
    if (0 == strncasecmp(s.c_str(), "empty", s.length()))
      resource.mstyle.bullet = Basemenu::Empty;
    else if (0 == strncasecmp(s.c_str(), "square", s.length()))
      resource.mstyle.bullet = Basemenu::Square;
    else if (0 == strncasecmp(s.c_str(), "diamond", s.length()))
      resource.mstyle.bullet = Basemenu::Diamond;
    else
      resource.mstyle.bullet = Basemenu::Triangle;
  } else
    resource.mstyle.bullet = Basemenu::Triangle;

  if (conf.getValue("menu.bullet.position", "Menu.Bullet.Position", s)) {
    if (0 == strncasecmp(s.c_str(), "right", s.length()))
      resource.mstyle.bullet_pos = Basemenu::Right;
    else
      resource.mstyle.bullet_pos = Basemenu::Left;
  } else
    resource.mstyle.bullet_pos = Basemenu::Left;

  readDatabaseColor("borderColor", "BorderColor", &resource.border_color,
		    BlackPixel(getBaseDisplay().getXDisplay(),
			       getScreenNumber()));

  // load bevel, border and handle widths
  if (conf.getValue("handleWidth", "HandleWidth", l)) {
    if (l <= (signed)size().w() / 2 && l != 0)
      resource.handle_width = l;
    else
      resource.handle_width = 6;
  } else
    resource.handle_width = 6;

  if (conf.getValue("borderWidth", "BorderWidth", l))
    resource.border_width = l;
  else
    resource.border_width = 1;

  if (conf.getValue("bevelWidth", "BevelWidth", l)) {
    if (l <= (signed)size().w() / 2 && l != 0)
      resource.bevel_width = l;
    else
      resource.bevel_width = 3;
  } else
    resource.bevel_width = 3;

  if (conf.getValue("frameWidth", "FrameWidth", l)) {
    if (l <= (signed)size().w() / 2)
      resource.frame_width = l;
    else
      resource.frame_width = resource.bevel_width;
  } else
    resource.frame_width = resource.bevel_width;

  const char *cmd = resource.root_command;
  if (cmd != NULL || conf.getValue("rootCommand", "RootCommand", s)) {
    if (cmd == NULL)
      cmd = s.c_str(); // not specified by the screen, so use the one from the
                       // style file
#ifndef    __EMX__
    char displaystring[MAXPATHLEN];
    sprintf(displaystring, "DISPLAY=%s",
	    DisplayString(getBaseDisplay().getXDisplay()));
    sprintf(displaystring + strlen(displaystring) - 1, "%d",
	    getScreenNumber());

    bexec(cmd, displaystring);
#else //   __EMX__
    spawnlp(P_NOWAIT, "cmd.exe", "cmd.exe", "/c", cmd, NULL);
#endif // !__EMX__
  }
}


void BScreen::addIcon(OpenboxWindow *w) {
  if (! w) return;

  w->setWorkspace(-1);
  w->setWindowNumber(iconList.size());

  iconList.push_back(w);

  iconmenu->insert((const char **) w->getIconTitle());
  iconmenu->update();
}


void BScreen::removeIcon(OpenboxWindow *w) {
  if (! w) return;

  iconList.remove(w);

  iconmenu->remove(w->getWindowNumber());
  iconmenu->update();

  winList::iterator it = iconList.begin();
  for (int i = 0; it != iconList.end(); ++it, ++i)
    (*it)->setWindowNumber(i);
}


OpenboxWindow *BScreen::getIcon(int index) {
  if (index < 0 || index >= (signed)iconList.size())
    return (OpenboxWindow *) 0;

  winList::iterator it = iconList.begin();
  for (; index > 0; --index, ++it);     // increment to index
  return *it;
}


int BScreen::addWorkspace(void) {
  Workspace *wkspc = new Workspace(*this, workspacesList.size());
  workspacesList.push_back(wkspc);
  setWorkspaceCount(workspaceCount()+1);
  saveWorkspaceNames();

  workspacemenu->insert(wkspc->getName(), wkspc->getMenu(),
			wkspc->getWorkspaceID() + 2);
  workspacemenu->update();

  toolbar->reconfigure();

  updateNetizenWorkspaceCount();

  return workspacesList.size();
}


int BScreen::removeLastWorkspace(void) {
  if (workspacesList.size() == 1)
    return 0;

  Workspace *wkspc = workspacesList.back();

  if (current_workspace->getWorkspaceID() == wkspc->getWorkspaceID())
    changeWorkspaceID(current_workspace->getWorkspaceID() - 1);

  wkspc->removeAll();

  workspacemenu->remove(wkspc->getWorkspaceID() + 2);
  workspacemenu->update();

  workspacesList.pop_back();
  delete wkspc;
  
  setWorkspaceCount(workspaceCount()-1);
  saveWorkspaceNames();

  toolbar->reconfigure();

  updateNetizenWorkspaceCount();

  return workspacesList.size();
}


void BScreen::changeWorkspaceID(int id) {
  if (! current_workspace) return;

  if (id != current_workspace->getWorkspaceID()) {
    current_workspace->hideAll();

    workspacemenu->setItemSelected(current_workspace->getWorkspaceID() + 2,
				   False);

    if (openbox.focusedWindow() &&
	openbox.focusedWindow()->getScreen() == this &&
        (! openbox.focusedWindow()->isStuck())) {
      openbox.focusWindow(0);
    }

    current_workspace = getWorkspace(id);

    workspacemenu->setItemSelected(current_workspace->getWorkspaceID() + 2,
				   True);
    toolbar->redrawWorkspaceLabel(True);

    current_workspace->showAll();

    if (resource.focus_last && current_workspace->lastFocusedWindow()) {
      XSync(openbox.getXDisplay(), False);
      current_workspace->lastFocusedWindow()->setInputFocus();
    }
  }

  updateNetizenCurrentWorkspace();
}


void BScreen::addNetizen(Netizen *n) {
  netizenList.push_back(n);

  n->sendWorkspaceCount();
  n->sendCurrentWorkspace();

  wkspList::iterator it;
  for (it = workspacesList.begin(); it != workspacesList.end(); ++it) {
    for (int i = 0; i < (*it)->getCount(); i++)
      n->sendWindowAdd((*it)->getWindow(i)->getClientWindow(),
		       (*it)->getWorkspaceID());
  }

  Window f = ((openbox.focusedWindow()) ?
              openbox.focusedWindow()->getClientWindow() : None);
  n->sendWindowFocus(f);
}


void BScreen::removeNetizen(Window w) {
  netList::iterator it;

  for (it = netizenList.begin(); it != netizenList.end(); ++it)
    if ((*it)->getWindowID() == w) {
      Netizen *tmp = *it;
      netizenList.erase(it);
      delete tmp;
      break;
    }
}


void BScreen::updateNetizenCurrentWorkspace(void) {
  netList::iterator it;
  for (it = netizenList.begin(); it != netizenList.end(); ++it)
    (*it)->sendCurrentWorkspace();
}


void BScreen::updateNetizenWorkspaceCount(void) {
  netList::iterator it;
  for (it = netizenList.begin(); it != netizenList.end(); ++it)
    (*it)->sendWorkspaceCount();
}


void BScreen::updateNetizenWindowFocus(void) {
  Window f = ((openbox.focusedWindow()) ?
              openbox.focusedWindow()->getClientWindow() : None);
  netList::iterator it;
  for (it = netizenList.begin(); it != netizenList.end(); ++it)
    (*it)->sendWindowFocus(f);
}


void BScreen::updateNetizenWindowAdd(Window w, unsigned long p) {
  netList::iterator it;
  for (it = netizenList.begin(); it != netizenList.end(); ++it)
    (*it)->sendWindowAdd(w, p);
}


void BScreen::updateNetizenWindowDel(Window w) {
  netList::iterator it;
  for (it = netizenList.begin(); it != netizenList.end(); ++it)
    (*it)->sendWindowDel(w);
}


void BScreen::updateNetizenWindowRaise(Window w) {
  netList::iterator it;
  for (it = netizenList.begin(); it != netizenList.end(); ++it)
    (*it)->sendWindowRaise(w);
}


void BScreen::updateNetizenWindowLower(Window w) {
  netList::iterator it;
  for (it = netizenList.begin(); it != netizenList.end(); ++it)
    (*it)->sendWindowLower(w);
}


void BScreen::updateNetizenConfigNotify(XEvent *e) {
  netList::iterator it;
  for (it = netizenList.begin(); it != netizenList.end(); ++it)
    (*it)->sendConfigNotify(e);
}


void BScreen::raiseWindows(Window *workspace_stack, int num) {
  Window *session_stack = new
    Window[(num + workspacesList.size() + rootmenuList.size() + 13)];
  int i = 0, k = num;

  XRaiseWindow(getBaseDisplay().getXDisplay(), iconmenu->getWindowID());
  *(session_stack + i++) = iconmenu->getWindowID();

  wkspList::iterator it;
  for (it = workspacesList.begin(); it != workspacesList.end(); ++it)
    *(session_stack + i++) = (*it)->getMenu()->getWindowID();

  *(session_stack + i++) = workspacemenu->getWindowID();

  *(session_stack + i++) = configmenu->getFocusmenu()->getWindowID();
  *(session_stack + i++) = configmenu->getPlacementmenu()->getWindowID();
  *(session_stack + i++) = configmenu->getWindowID();

#ifdef    SLIT
  *(session_stack + i++) = slit->getMenu()->getDirectionmenu()->getWindowID();
  *(session_stack + i++) = slit->getMenu()->getPlacementmenu()->getWindowID();
  *(session_stack + i++) = slit->getMenu()->getWindowID();
#endif // SLIT

  *(session_stack + i++) =
    toolbar->getMenu()->getPlacementmenu()->getWindowID();
  *(session_stack + i++) = toolbar->getMenu()->getWindowID();

  menuList::iterator rit;
  for (rit = rootmenuList.begin(); rit != rootmenuList.end(); ++rit)
    *(session_stack + i++) = (*rit)->getWindowID();
  *(session_stack + i++) = rootmenu->getWindowID();

  if (toolbar->onTop())
    *(session_stack + i++) = toolbar->getWindowID();

#ifdef    SLIT
  if (slit->onTop())
    *(session_stack + i++) = slit->getWindowID();
#endif // SLIT

  while (k--)
    *(session_stack + i++) = *(workspace_stack + k);

  XRestackWindows(getBaseDisplay().getXDisplay(), session_stack, i);

  delete [] session_stack;
}


void BScreen::addWorkspaceName(const char *name) {
  workspaceNames.push_back(name);
}


const char *BScreen::getNameOfWorkspace(int id) {
  if (id < 0 || id >= (signed)workspaceNames.size())
    return (const char *) 0;
  return workspaceNames[id].c_str();
}


void BScreen::reassociateWindow(OpenboxWindow *w, int wkspc_id, Bool ignore_sticky) {
  if (! w) return;

  if (wkspc_id == -1)
    wkspc_id = current_workspace->getWorkspaceID();

  if (w->getWorkspaceNumber() == wkspc_id)
    return;

  if (w->isIconic()) {
    removeIcon(w);
    getWorkspace(wkspc_id)->addWindow(w);
  } else if (ignore_sticky || ! w->isStuck()) {
    getWorkspace(w->getWorkspaceNumber())->removeWindow(w);
    getWorkspace(wkspc_id)->addWindow(w);
  }
}


void BScreen::nextFocus(void) {
  Bool have_focused = False;
  int focused_window_number = -1;
  OpenboxWindow *next;

  if (openbox.focusedWindow()) {
    if (openbox.focusedWindow()->getScreen()->getScreenNumber() ==
	getScreenNumber()) {
      have_focused = True;
      focused_window_number = openbox.focusedWindow()->getWindowNumber();
    }
  }

  if ((getCurrentWorkspace()->getCount() > 1) && have_focused) {
    int next_window_number = focused_window_number;
    do {
      if ((++next_window_number) >= getCurrentWorkspace()->getCount())
	next_window_number = 0;

      next = getCurrentWorkspace()->getWindow(next_window_number);
    } while ((! next->setInputFocus()) && (next_window_number !=
					   focused_window_number));

    if (next_window_number != focused_window_number)
      getCurrentWorkspace()->raiseWindow(next);
  } else if (getCurrentWorkspace()->getCount() >= 1) {
    next = current_workspace->getWindow(0);

    current_workspace->raiseWindow(next);
    next->setInputFocus();
  }
}


void BScreen::prevFocus(void) {
  Bool have_focused = False;
  int focused_window_number = -1;
  OpenboxWindow *prev;

  if (openbox.focusedWindow()) {
    if (openbox.focusedWindow()->getScreen()->getScreenNumber() ==
	getScreenNumber()) {
      have_focused = True;
      focused_window_number = openbox.focusedWindow()->getWindowNumber();
    }
  }

  if ((getCurrentWorkspace()->getCount() > 1) && have_focused) {
    int prev_window_number = focused_window_number;
    do {
      if ((--prev_window_number) < 0)
	prev_window_number = getCurrentWorkspace()->getCount() - 1;

      prev = getCurrentWorkspace()->getWindow(prev_window_number);
    } while ((! prev->setInputFocus()) && (prev_window_number !=
					   focused_window_number));

    if (prev_window_number != focused_window_number)
      getCurrentWorkspace()->raiseWindow(prev);
  } else if (getCurrentWorkspace()->getCount() >= 1) {
    prev = current_workspace->getWindow(0);

    current_workspace->raiseWindow(prev);
    prev->setInputFocus();
  }
}


void BScreen::raiseFocus(void) {
  Bool have_focused = False;
  int focused_window_number = -1;

  if (openbox.focusedWindow()) {
    if (openbox.focusedWindow()->getScreen()->getScreenNumber() ==
	getScreenNumber()) {
      have_focused = True;
      focused_window_number = openbox.focusedWindow()->getWindowNumber();
    }
  }

  if ((getCurrentWorkspace()->getCount() > 1) && have_focused)
    getWorkspace(openbox.focusedWindow()->getWorkspaceNumber())->
      raiseWindow(openbox.focusedWindow());
}


void BScreen::InitMenu(void) {
  if (rootmenu) {
    rootmenuList.clear();
    while (rootmenu->getCount())
      rootmenu->remove(0);
  } else {
    rootmenu = new Rootmenu(*this);
  }
  Bool defaultMenu = True;

  if (openbox.getMenuFilename()) {
    FILE *menu_file = fopen(openbox.getMenuFilename(), "r");

    if (!menu_file) {
      perror(openbox.getMenuFilename());
    } else {
      if (feof(menu_file)) {
	fprintf(stderr, i18n->getMessage(ScreenSet, ScreenEmptyMenuFile,
					 "%s: Empty menu file"),
		openbox.getMenuFilename());
      } else {
	char line[1024], label[1024];
	memset(line, 0, 1024);
	memset(label, 0, 1024);

	while (fgets(line, 1024, menu_file) && ! feof(menu_file)) {
	  if (line[0] != '#') {
	    int i, key = 0, index = -1, len = strlen(line);

	    key = 0;
	    for (i = 0; i < len; i++) {
	      if (line[i] == '[') index = 0;
	      else if (line[i] == ']') break;
	      else if (line[i] != ' ')
		if (index++ >= 0)
		  key += tolower(line[i]);
	    }

	    if (key == 517) {
	      index = -1;
	      for (i = index; i < len; i++) {
		if (line[i] == '(') index = 0;
		else if (line[i] == ')') break;
		else if (index++ >= 0) {
		  if (line[i] == '\\' && i < len - 1) i++;
		  label[index - 1] = line[i];
		}
	      }

	      if (index == -1) index = 0;
	      label[index] = '\0';

	      rootmenu->setLabel(label);
	      defaultMenu = parseMenuFile(menu_file, rootmenu);
	      break;
	    }
	  }
	}
      }
      fclose(menu_file);
    }
  }

  if (defaultMenu) {
    rootmenu->setInternalMenu();
    rootmenu->insert(i18n->getMessage(ScreenSet, Screenxterm, "xterm"),
		     BScreen::Execute,
		     i18n->getMessage(ScreenSet, Screenxterm, "xterm"));
    rootmenu->insert(i18n->getMessage(ScreenSet, ScreenRestart, "Restart"),
		     BScreen::Restart);
    rootmenu->insert(i18n->getMessage(ScreenSet, ScreenExit, "Exit"),
		     BScreen::Exit);
  } else {
    openbox.setMenuFilename(openbox.getMenuFilename());
  }
}


Bool BScreen::parseMenuFile(FILE *file, Rootmenu *menu) {
  char line[1024], label[1024], command[1024];

  while (! feof(file)) {
    memset(line, 0, 1024);
    memset(label, 0, 1024);
    memset(command, 0, 1024);

    if (fgets(line, 1024, file)) {
      if (line[0] != '#') {
	register int i, key = 0, parse = 0, index = -1,
	  line_length = strlen(line),
	  label_length = 0, command_length = 0;

	// determine the keyword
	key = 0;
	for (i = 0; i < line_length; i++) {
	  if (line[i] == '[') parse = 1;
	  else if (line[i] == ']') break;
	  else if (line[i] != ' ')
	    if (parse)
	      key += tolower(line[i]);
	}

	// get the label enclosed in ()'s
	parse = 0;

	for (i = 0; i < line_length; i++) {
	  if (line[i] == '(') {
	    index = 0;
	    parse = 1;
	  } else if (line[i] == ')') break;
	  else if (index++ >= 0) {
	    if (line[i] == '\\' && i < line_length - 1) i++;
	    label[index - 1] = line[i];
	  }
	}

	if (parse) {
	  label[index] = '\0';
	  label_length = index;
	} else {
	  label[0] = '\0';
	  label_length = 0;
	}

	// get the command enclosed in {}'s
	parse = 0;
	index = -1;
	for (i = 0; i < line_length; i++) {
	  if (line[i] == '{') {
	    index = 0;
	    parse = 1;
	  } else if (line[i] == '}') break;
	  else if (index++ >= 0) {
	    if (line[i] == '\\' && i < line_length - 1) i++;
	    command[index - 1] = line[i];
	  }
	}

	if (parse) {
	  command[index] = '\0';
	  command_length = index;
	} else {
	  command[0] = '\0';
	  command_length = 0;
	}

	switch (key) {
        case 311: //end
          return ((menu->getCount() == 0) ? True : False);

          break;

        case 333: // nop
	  menu->insert(label);

	  break;

	case 421: // exec
	  if ((! *label) && (! *command)) {
	    fprintf(stderr, i18n->getMessage(ScreenSet, ScreenEXECError,
			     "BScreen::parseMenuFile: [exec] error, "
			     "no menu label and/or command defined\n"));
	    continue;
	  }

	  menu->insert(label, BScreen::Execute, command);

	  break;

	case 442: // exit
	  if (! *label) {
	    fprintf(stderr, i18n->getMessage(ScreenSet, ScreenEXITError,
				     "BScreen::parseMenuFile: [exit] error, "
				     "no menu label defined\n"));
	    continue;
	  }

	  menu->insert(label, BScreen::Exit);

	  break;

	case 561: // style
	  {
	    if ((! *label) || (! *command)) {
	      fprintf(stderr, i18n->getMessage(ScreenSet, ScreenSTYLEError,
				 "BScreen::parseMenuFile: [style] error, "
				 "no menu label and/or filename defined\n"));
	      continue;
	    }

	    char style[MAXPATHLEN];

	    // perform shell style ~ home directory expansion
	    char *homedir = 0;
	    int homedir_len = 0;
	    if (*command == '~' && *(command + 1) == '/') {
	      homedir = getenv("HOME");
	      homedir_len = strlen(homedir);
	    }

	    if (homedir && homedir_len != 0) {
	      strncpy(style, homedir, homedir_len);

	      strncpy(style + homedir_len, command + 1,
		      command_length - 1);
	      *(style + command_length + homedir_len - 1) = '\0';
	    } else {
	      strncpy(style, command, command_length);
	      *(style + command_length) = '\0';
	    }

	    menu->insert(label, BScreen::SetStyle, style);
	  }

	  break;

	case 630: // config
	  if (! *label) {
	    fprintf(stderr, i18n->getMessage(ScreenSet, ScreenCONFIGError,
			       "BScreen::parseMenufile: [config] error, "
			       "no label defined"));
	    continue;
	  }

	  menu->insert(label, configmenu);

	  break;

	case 740: // include
	  {
	    if (! *label) {
	      fprintf(stderr, i18n->getMessage(ScreenSet, ScreenINCLUDEError,
				 "BScreen::parseMenuFile: [include] error, "
				 "no filename defined\n"));
	      continue;
	    }

	    char newfile[MAXPATHLEN];

	    // perform shell style ~ home directory expansion
	    char *homedir = 0;
	    int homedir_len = 0;
	    if (*label == '~' && *(label + 1) == '/') {
	      homedir = getenv("HOME");
	      homedir_len = strlen(homedir);
	    }

	    if (homedir && homedir_len != 0) {
	      strncpy(newfile, homedir, homedir_len);

	      strncpy(newfile + homedir_len, label + 1,
		      label_length - 1);
	      *(newfile + label_length + homedir_len - 1) = '\0';
	    } else {
	      strncpy(newfile, label, label_length);
	      *(newfile + label_length) = '\0';
	    }

	    if (newfile) {
	      FILE *submenufile = fopen(newfile, "r");

	      if (submenufile) {
                struct stat buf;
                if (fstat(fileno(submenufile), &buf) ||
                    (! S_ISREG(buf.st_mode))) {
                  fprintf(stderr,
			  i18n->getMessage(ScreenSet, ScreenINCLUDEErrorReg,
			     "BScreen::parseMenuFile: [include] error: "
			     "'%s' is not a regular file\n"), newfile);
                  break;
                }

		if (! feof(submenufile)) {
		  if (! parseMenuFile(submenufile, menu))
		    openbox.setMenuFilename(newfile);

		  fclose(submenufile);
		}
	      } else
		perror(newfile);
	    }
	  }

	  break;

	case 767: // submenu
	  {
	    if (! *label) {
	      fprintf(stderr, i18n->getMessage(ScreenSet, ScreenSUBMENUError,
				 "BScreen::parseMenuFile: [submenu] error, "
				 "no menu label defined\n"));
	      continue;
	    }

	    Rootmenu *submenu = new Rootmenu(*this);

	    if (*command)
	      submenu->setLabel(command);
	    else
	      submenu->setLabel(label);

	    parseMenuFile(file, submenu);
	    submenu->update();
	    menu->insert(label, submenu);
	    rootmenuList.push_back(submenu);
	  }

	  break;

	case 773: // restart
	  {
	    if (! *label) {
	      fprintf(stderr, i18n->getMessage(ScreenSet, ScreenRESTARTError,
				 "BScreen::parseMenuFile: [restart] error, "
				 "no menu label defined\n"));
	      continue;
	    }

	    if (*command)
	      menu->insert(label, BScreen::RestartOther, command);
	    else
	      menu->insert(label, BScreen::Restart);
	  }

	  break;

	case 845: // reconfig
	  {
	    if (! *label) {
	      fprintf(stderr, i18n->getMessage(ScreenSet, ScreenRECONFIGError,
				 "BScreen::parseMenuFile: [reconfig] error, "
				 "no menu label defined\n"));
	      continue;
	    }

	    menu->insert(label, BScreen::Reconfigure);
	  }

	  break;

        case 995: // stylesdir
        case 1113: // stylesmenu
          {
            Bool newmenu = ((key == 1113) ? True : False);

            if ((! *label) || ((! *command) && newmenu)) {
              fprintf(stderr,
		      i18n->getMessage(ScreenSet, ScreenSTYLESDIRError,
			 "BScreen::parseMenuFile: [stylesdir/stylesmenu]"
			 " error, no directory defined\n"));
              continue;
            }

            char stylesdir[MAXPATHLEN];

            char *directory = ((newmenu) ? command : label);
            int directory_length = ((newmenu) ? command_length : label_length);

            // perform shell style ~ home directory expansion
            char *homedir = 0;
            int homedir_len = 0;

            if (*directory == '~' && *(directory + 1) == '/') {
              homedir = getenv("HOME");
              homedir_len = strlen(homedir);
            }

            if (homedir && homedir_len != 0) {
              strncpy(stylesdir, homedir, homedir_len);

              strncpy(stylesdir + homedir_len, directory + 1,
                      directory_length - 1);
              *(stylesdir + directory_length + homedir_len - 1) = '\0';
            } else {
              strncpy(stylesdir, directory, directory_length);
              *(stylesdir + directory_length) = '\0';
            }

            struct stat statbuf;

            if (! stat(stylesdir, &statbuf)) {
              if (S_ISDIR(statbuf.st_mode)) {
                Rootmenu *stylesmenu;

                if (newmenu)
                  stylesmenu = new Rootmenu(*this);
                else
                  stylesmenu = menu;

                DIR *d = opendir(stylesdir);
                int entries = 0;
                struct dirent *p;

                // get the total number of directory entries
                while ((p = readdir(d))) entries++;
                rewinddir(d);

                char **ls = new char* [entries];
                int index = 0;
                while ((p = readdir(d)))
		  ls[index++] = bstrdup(p->d_name);

		closedir(d);

                std::sort(ls, ls + entries, dcmp());

                int n, slen = strlen(stylesdir);
                for (n = 0; n < entries; n++) {
                  if (ls[n][strlen(ls[n])-1] != '~') {
                    int nlen = strlen(ls[n]);
                    char style[MAXPATHLEN + 1];

                    strncpy(style, stylesdir, slen);
                    *(style + slen) = '/';
                    strncpy(style + slen + 1, ls[n], nlen + 1);

                    if ((! stat(style, &statbuf)) && S_ISREG(statbuf.st_mode))
                      stylesmenu->insert(ls[n], BScreen::SetStyle, style);
                  }

                  delete [] ls[n];
                }

                delete [] ls;

                stylesmenu->update();

                if (newmenu) {
                  stylesmenu->setLabel(label);
                  menu->insert(label, stylesmenu);
                  rootmenuList.push_back(stylesmenu);
                }

                openbox.setMenuFilename(stylesdir);
              } else {
                fprintf(stderr, i18n->getMessage(ScreenSet,
						 ScreenSTYLESDIRErrorNotDir,
				   "BScreen::parseMenuFile:"
				   " [stylesdir/stylesmenu] error, %s is not a"
				   " directory\n"), stylesdir);
              }
            } else {
              fprintf(stderr,
		      i18n->getMessage(ScreenSet, ScreenSTYLESDIRErrorNoExist,
			 "BScreen::parseMenuFile: [stylesdir/stylesmenu]"
			 " error, %s does not exist\n"), stylesdir);
            }

            break;
          }

	case 1090: // workspaces
	  {
	    if (! *label) {
	      fprintf(stderr,
		      i18n->getMessage(ScreenSet, ScreenWORKSPACESError,
			       "BScreen:parseMenuFile: [workspaces] error, "
			       "no menu label defined\n"));
	      continue;
	    }

	    menu->insert(label, workspacemenu);

	    break;
	  }
	}
      }
    }
  }

  return ((menu->getCount() == 0) ? True : False);
}


void BScreen::shutdown(void) {
  openbox.grab();

  XSelectInput(getBaseDisplay().getXDisplay(), getRootWindow(), NoEventMask);
  XSync(getBaseDisplay().getXDisplay(), False);

  std::for_each(workspacesList.begin(), workspacesList.end(),
                std::mem_fun(&Workspace::shutdown));

  while (!iconList.empty())
    iconList.front()->restore();

#ifdef    SLIT
  slit->shutdown();
#endif // SLIT

  openbox.ungrab();
}


void BScreen::showPosition(int x, int y) {
  if (! geom_visible) {
    XMoveResizeWindow(getBaseDisplay().getXDisplay(), geom_window,
                      (size().w() - geom_w) / 2,
                      (size().h() - geom_h) / 2, geom_w, geom_h);
    XMapWindow(getBaseDisplay().getXDisplay(), geom_window);
    XRaiseWindow(getBaseDisplay().getXDisplay(), geom_window);

    geom_visible = True;
  }

  char label[1024];

  sprintf(label, i18n->getMessage(ScreenSet, ScreenPositionFormat,
				  "X: %4d x Y: %4d"), x, y);

  XClearWindow(getBaseDisplay().getXDisplay(), geom_window);

  if (i18n->multibyte()) {
    XmbDrawString(getBaseDisplay().getXDisplay(), geom_window,
		  resource.wstyle.fontset, resource.wstyle.l_text_focus_gc,
		  resource.bevel_width, resource.bevel_width -
		  resource.wstyle.fontset_extents->max_ink_extent.y,
		  label, strlen(label));
  } else {
    XDrawString(getBaseDisplay().getXDisplay(), geom_window,
		resource.wstyle.l_text_focus_gc,
		resource.bevel_width,
		resource.wstyle.font->ascent +
		resource.bevel_width, label, strlen(label));
  }
}


void BScreen::showGeometry(unsigned int gx, unsigned int gy) {
  if (! geom_visible) {
    XMoveResizeWindow(getBaseDisplay().getXDisplay(), geom_window,
                      (size().w() - geom_w) / 2,
                      (size().h() - geom_h) / 2, geom_w, geom_h);
    XMapWindow(getBaseDisplay().getXDisplay(), geom_window);
    XRaiseWindow(getBaseDisplay().getXDisplay(), geom_window);

    geom_visible = True;
  }

  char label[1024];

  sprintf(label, i18n->getMessage(ScreenSet, ScreenGeometryFormat,
				  "W: %4d x H: %4d"), gx, gy);

  XClearWindow(getBaseDisplay().getXDisplay(), geom_window);

  if (i18n->multibyte()) {
    XmbDrawString(getBaseDisplay().getXDisplay(), geom_window,
		  resource.wstyle.fontset, resource.wstyle.l_text_focus_gc,
		  resource.bevel_width, resource.bevel_width -
		  resource.wstyle.fontset_extents->max_ink_extent.y,
		  label, strlen(label));
  } else {
    XDrawString(getBaseDisplay().getXDisplay(), geom_window,
		resource.wstyle.l_text_focus_gc,
		resource.bevel_width,
		resource.wstyle.font->ascent +
		resource.bevel_width, label, strlen(label));
  }
}

void BScreen::hideGeometry(void) {
  if (geom_visible) {
    XUnmapWindow(getBaseDisplay().getXDisplay(), geom_window);
    geom_visible = False;
  }
}
