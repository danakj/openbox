#include <otk/otk.hh>

int main(int, char **)
{
  otk::initialize();

  int screen = DefaultScreen(**otk::display);
  
  Pixmap old;
  Window root = otk::display->screenInfo(screen)->rootWindow();

  otk::Surface *s = new otk::Surface(screen,
				     otk::display->screenInfo(screen)->size());
  otk::RenderTexture *tx = new otk::RenderTexture(screen, false,
						  otk::RenderTexture::Flat,
						  otk::RenderTexture::Bevel1,
						  false,
						  otk::RenderTexture::Solid,
						  false, 0x202020, 0x0000ff,
						  0, 0);
  otk::display->renderControl(screen)->drawBackground(*s, *tx);

  otk::display->grab();

  otk::display->setIgnoreErrors(true);
  // get the current pixmap and free it
  if (otk::Property::get(root, otk::Property::atoms.rootpmapid,
			 otk::Property::atoms.pixmap, &old) && old) {
    XKillClient(**otk::display, old);
    XSync(**otk::display, false);
    XFreePixmap(**otk::display, old);
  }
  if (otk::Property::get(root, otk::Property::atoms.esetrootid,
			 otk::Property::atoms.pixmap, &old) && old)
    XFreePixmap(**otk::display, old);
  otk::display->setIgnoreErrors(false);

  //  XSetWindowBackground(**display, root, color.pixel());

  // don't kill us when someone wants to change the background!!
  Pixmap pixmap = XCreatePixmap(**otk::display, root, s->size().width(),
				s->size().height(),
				otk::display->screenInfo(screen)->depth());
  XCopyArea(**otk::display, s->pixmap(), pixmap,
	    DefaultGC(**otk::display, screen), 0, 0,
	    s->size().width(), s->size().height(), 0, 0);

  // set the new pixmap
  XSetWindowBackgroundPixmap(**otk::display, root, pixmap);
  XClearWindow(**otk::display, root);

  otk::Property::set(root, otk::Property::atoms.rootpmapid,
		     otk::Property::atoms.pixmap, pixmap);
  otk::Property::set(root, otk::Property::atoms.esetrootid,
		     otk::Property::atoms.pixmap, pixmap);
  
  otk::display->ungrab();

  delete tx;
  delete s;

  XSetCloseDownMode(**otk::display, RetainPermanent);
  XKillClient(**otk::display, AllTemporary);
  
  otk::destroy();
}
