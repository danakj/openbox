# src/Makefile.am for Blackbox - an X11 Window manager
# Copyright (c) 1997 - 2000 Brad Hughes (bhughes@tcac.net)
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the 
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in 
# all copies or substantial portions of the Software. 
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
# DEALINGS IN THE SOFTWARE.

DEFAULT_MENU=$(pkgdatadir)/menu
DEFAULT_STYLE=$(pkgdatadir)/styles/steelblue

CPPFLAGS= @CPPFLAGS@ @SHAPE@ @ORDEREDPSEUDO@ \
@DEBUG@ @NLS@ @TIMEDCACHE@ @SLIT@ @CLOBBER@ \
-DLOCALEPATH=\"$(pkgdatadir)/nls\" \
-DDEFAULTMENU=\"$(DEFAULT_MENU)\" \
-DDEFAULTSTYLE=\"$(DEFAULT_STYLE)\"

bin_PROGRAMS= blackbox

blackbox_SOURCES= BaseDisplay.cc Basemenu.cc Clientmenu.cc Color.cc Configmenu.cc GCCache.cc Iconmenu.cc Image.cc ImageControl.cc Netizen.cc Rootmenu.cc Screen.cc Slit.cc Texture.cc Timer.cc Toolbar.cc Util.cc Window.cc Windowmenu.cc Workspace.cc Workspacemenu.cc blackbox.cc i18n.cc main.cc

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	rm -f *\~ *.orig *.rej

# local dependencies

BaseDisplay.o: BaseDisplay.cc ../config.h i18n.hh \
 ../nls/blackbox-nls.hh BaseDisplay.hh Timer.hh GCCache.hh Color.hh \
 Util.hh
Basemenu.o: Basemenu.cc ../config.h i18n.hh ../nls/blackbox-nls.hh \
 blackbox.hh BaseDisplay.hh Timer.hh Basemenu.hh GCCache.hh Color.hh \
 Image.hh Screen.hh Texture.hh Util.hh Configmenu.hh Iconmenu.hh \
 Netizen.hh Rootmenu.hh Workspace.hh Workspacemenu.hh
Clientmenu.o: Clientmenu.cc ../config.h blackbox.hh i18n.hh \
 ../nls/blackbox-nls.hh BaseDisplay.hh Timer.hh Clientmenu.hh \
 Basemenu.hh Screen.hh Color.hh Texture.hh Util.hh Configmenu.hh \
 Iconmenu.hh Netizen.hh Rootmenu.hh Workspace.hh Workspacemenu.hh \
 Window.hh Windowmenu.hh
Color.o: Color.cc ../config.h Color.hh BaseDisplay.hh Timer.hh
Configmenu.o: Configmenu.cc ../config.h i18n.hh ../nls/blackbox-nls.hh \
 Configmenu.hh Basemenu.hh Image.hh Timer.hh BaseDisplay.hh Color.hh \
 Toolbar.hh Screen.hh Texture.hh Util.hh Iconmenu.hh Netizen.hh \
 Rootmenu.hh Workspace.hh Workspacemenu.hh blackbox.hh Window.hh \
 Windowmenu.hh
GCCache.o: GCCache.cc ../config.h GCCache.hh BaseDisplay.hh Timer.hh \
 Color.hh Util.hh
Iconmenu.o: Iconmenu.cc ../config.h i18n.hh ../nls/blackbox-nls.hh \
 Iconmenu.hh Basemenu.hh Screen.hh Color.hh Texture.hh Util.hh \
 Configmenu.hh Netizen.hh Rootmenu.hh Timer.hh Workspace.hh \
 Workspacemenu.hh blackbox.hh BaseDisplay.hh Window.hh Windowmenu.hh
Image.o: Image.cc ../config.h blackbox.hh i18n.hh \
 ../nls/blackbox-nls.hh BaseDisplay.hh Timer.hh GCCache.hh Color.hh \
 Image.hh Texture.hh Util.hh
ImageControl.o: ImageControl.cc ../config.h blackbox.hh i18n.hh \
 ../nls/blackbox-nls.hh BaseDisplay.hh Timer.hh Color.hh Image.hh \
 Texture.hh Util.hh
Netizen.o: Netizen.cc ../config.h Netizen.hh Screen.hh Color.hh \
 Texture.hh Util.hh Configmenu.hh Basemenu.hh Iconmenu.hh Rootmenu.hh \
 Timer.hh Workspace.hh Workspacemenu.hh blackbox.hh i18n.hh \
 ../nls/blackbox-nls.hh BaseDisplay.hh
Rootmenu.o: Rootmenu.cc ../config.h blackbox.hh i18n.hh \
 ../nls/blackbox-nls.hh BaseDisplay.hh Timer.hh Rootmenu.hh \
 Basemenu.hh Screen.hh Color.hh Texture.hh Util.hh Configmenu.hh \
 Iconmenu.hh Netizen.hh Workspace.hh Workspacemenu.hh
Screen.o: Screen.cc ../config.h i18n.hh ../nls/blackbox-nls.hh \
 blackbox.hh BaseDisplay.hh Timer.hh Clientmenu.hh Basemenu.hh \
 GCCache.hh Color.hh Iconmenu.hh Image.hh Screen.hh Texture.hh Util.hh \
 Configmenu.hh Netizen.hh Rootmenu.hh Workspace.hh Workspacemenu.hh \
 Slit.hh Toolbar.hh Window.hh Windowmenu.hh
Slit.o: Slit.cc ../config.h i18n.hh ../nls/blackbox-nls.hh blackbox.hh \
 BaseDisplay.hh Timer.hh Image.hh Color.hh Screen.hh Texture.hh \
 Util.hh Configmenu.hh Basemenu.hh Iconmenu.hh Netizen.hh Rootmenu.hh \
 Workspace.hh Workspacemenu.hh Slit.hh Toolbar.hh
Texture.o: Texture.cc ../config.h Texture.hh Color.hh Util.hh \
 BaseDisplay.hh Timer.hh Image.hh Screen.hh Configmenu.hh Basemenu.hh \
 Iconmenu.hh Netizen.hh Rootmenu.hh Workspace.hh Workspacemenu.hh \
 blackbox.hh i18n.hh ../nls/blackbox-nls.hh
Timer.o: Timer.cc ../config.h BaseDisplay.hh Timer.hh Util.hh
Toolbar.o: Toolbar.cc ../config.h i18n.hh ../nls/blackbox-nls.hh \
 blackbox.hh BaseDisplay.hh Timer.hh Clientmenu.hh Basemenu.hh \
 GCCache.hh Color.hh Iconmenu.hh Image.hh Rootmenu.hh Screen.hh \
 Texture.hh Util.hh Configmenu.hh Netizen.hh Workspace.hh \
 Workspacemenu.hh Toolbar.hh Window.hh Windowmenu.hh Slit.hh
Util.o: Util.cc ../config.h Util.hh
Window.o: Window.cc ../config.h i18n.hh ../nls/blackbox-nls.hh \
 blackbox.hh BaseDisplay.hh Timer.hh GCCache.hh Color.hh Iconmenu.hh \
 Basemenu.hh Image.hh Screen.hh Texture.hh Util.hh Configmenu.hh \
 Netizen.hh Rootmenu.hh Workspace.hh Workspacemenu.hh Toolbar.hh \
 Window.hh Windowmenu.hh Slit.hh
Windowmenu.o: Windowmenu.cc ../config.h i18n.hh ../nls/blackbox-nls.hh \
 blackbox.hh BaseDisplay.hh Timer.hh Screen.hh Color.hh Texture.hh \
 Util.hh Configmenu.hh Basemenu.hh Iconmenu.hh Netizen.hh Rootmenu.hh \
 Workspace.hh Workspacemenu.hh Window.hh Windowmenu.hh
Workspace.o: Workspace.cc ../config.h i18n.hh ../nls/blackbox-nls.hh \
 blackbox.hh BaseDisplay.hh Timer.hh Clientmenu.hh Basemenu.hh \
 Netizen.hh Screen.hh Color.hh Texture.hh Util.hh Configmenu.hh \
 Iconmenu.hh Rootmenu.hh Workspace.hh Workspacemenu.hh Toolbar.hh \
 Window.hh Windowmenu.hh
Workspacemenu.o: Workspacemenu.cc ../config.h i18n.hh \
 ../nls/blackbox-nls.hh blackbox.hh BaseDisplay.hh Timer.hh Screen.hh \
 Color.hh Texture.hh Util.hh Configmenu.hh Basemenu.hh Iconmenu.hh \
 Netizen.hh Rootmenu.hh Workspace.hh Workspacemenu.hh Toolbar.hh
blackbox.o: blackbox.cc ../config.h i18n.hh ../nls/blackbox-nls.hh \
 blackbox.hh BaseDisplay.hh Timer.hh Basemenu.hh Clientmenu.hh \
 GCCache.hh Color.hh Image.hh Rootmenu.hh Screen.hh Texture.hh Util.hh \
 Configmenu.hh Iconmenu.hh Netizen.hh Workspace.hh Workspacemenu.hh \
 Slit.hh Toolbar.hh Window.hh Windowmenu.hh
i18n.o: i18n.cc ../config.h i18n.hh ../nls/blackbox-nls.hh
main.o: main.cc ../version.h ../config.h i18n.hh \
 ../nls/blackbox-nls.hh blackbox.hh BaseDisplay.hh Timer.hh
