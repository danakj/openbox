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
DEFAULT_STYLE=$(pkgdatadir)/styles/mbdtex

CPPFLAGS=@CPPFLAGS@ @DEBUG@ \
-DLOCALEPATH=\"$(pkgdatadir)/nls\" \
-DBUTTONSDIR=\"$(pkgdatadir)/buttons\" \
-DDEFAULTMENU=\"$(DEFAULT_MENU)\" \
-DDEFAULTSTYLE=\"$(DEFAULT_STYLE)\"

bin_PROGRAMS= openbox

openbox_SOURCES= BaseDisplay.cc Basemenu.cc Clientmenu.cc Color.cc Configmenu.cc Configuration.cc Font.cc GCCache.cc Iconmenu.cc Image.cc ImageControl.cc Netizen.cc Rootmenu.cc Screen.cc Slit.cc Texture.cc Timer.cc Toolbar.cc Util.cc Window.cc Windowmenu.cc Workspace.cc Workspacemenu.cc XAtom.cc blackbox.cc i18n.cc main.cc

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	rm -f *\~ *.orig *.rej .\#*

# local dependencies

BaseDisplay.o: BaseDisplay.cc i18n.hh ../nls/blackbox-nls.hh \
 BaseDisplay.hh Timer.hh Util.hh GCCache.hh Color.hh
Basemenu.o: Basemenu.cc i18n.hh ../nls/blackbox-nls.hh blackbox.hh \
 BaseDisplay.hh Timer.hh Util.hh Configuration.hh XAtom.hh Basemenu.hh \
 Font.hh Screen.hh Color.hh Texture.hh Image.hh Configmenu.hh \
 Iconmenu.hh Netizen.hh Rootmenu.hh Workspace.hh Workspacemenu.hh \
 GCCache.hh
Clientmenu.o: Clientmenu.cc blackbox.hh i18n.hh ../nls/blackbox-nls.hh \
 BaseDisplay.hh Timer.hh Util.hh Configuration.hh XAtom.hh \
 Clientmenu.hh Basemenu.hh Screen.hh Color.hh Texture.hh Image.hh \
 Configmenu.hh Iconmenu.hh Netizen.hh Rootmenu.hh Workspace.hh \
 Workspacemenu.hh Window.hh Windowmenu.hh
Color.o: Color.cc Color.hh BaseDisplay.hh Timer.hh Util.hh
Configmenu.o: Configmenu.cc i18n.hh ../nls/blackbox-nls.hh \
 Configmenu.hh Basemenu.hh Image.hh Timer.hh BaseDisplay.hh Util.hh \
 Color.hh Toolbar.hh Screen.hh Texture.hh Iconmenu.hh Netizen.hh \
 Rootmenu.hh Workspace.hh Workspacemenu.hh blackbox.hh \
 Configuration.hh XAtom.hh Window.hh Windowmenu.hh
Configuration.o: Configuration.cc ../config.h Configuration.hh Util.hh
Font.o: Font.cc i18n.hh ../nls/blackbox-nls.hh Font.hh Screen.hh \
 Color.hh Texture.hh Util.hh Image.hh Timer.hh BaseDisplay.hh \
 Configmenu.hh Basemenu.hh Iconmenu.hh Netizen.hh Rootmenu.hh \
 Workspace.hh Workspacemenu.hh blackbox.hh Configuration.hh XAtom.hh \
 GCCache.hh
GCCache.o: GCCache.cc GCCache.hh BaseDisplay.hh Timer.hh Util.hh \
 Color.hh
Iconmenu.o: Iconmenu.cc i18n.hh ../nls/blackbox-nls.hh Iconmenu.hh \
 Basemenu.hh Screen.hh Color.hh Texture.hh Util.hh Image.hh Timer.hh \
 BaseDisplay.hh Configmenu.hh Netizen.hh Rootmenu.hh Workspace.hh \
 Workspacemenu.hh blackbox.hh Configuration.hh XAtom.hh Window.hh \
 Windowmenu.hh
Image.o: Image.cc blackbox.hh i18n.hh ../nls/blackbox-nls.hh \
 BaseDisplay.hh Timer.hh Util.hh Configuration.hh XAtom.hh GCCache.hh \
 Color.hh Image.hh Texture.hh
ImageControl.o: ImageControl.cc blackbox.hh i18n.hh \
 ../nls/blackbox-nls.hh BaseDisplay.hh Timer.hh Util.hh \
 Configuration.hh XAtom.hh Color.hh Image.hh Texture.hh
Netizen.o: Netizen.cc Netizen.hh Screen.hh Color.hh Texture.hh Util.hh \
 Image.hh Timer.hh BaseDisplay.hh Configmenu.hh Basemenu.hh \
 Iconmenu.hh Rootmenu.hh Workspace.hh Workspacemenu.hh blackbox.hh \
 i18n.hh ../nls/blackbox-nls.hh Configuration.hh XAtom.hh
Rootmenu.o: Rootmenu.cc blackbox.hh i18n.hh ../nls/blackbox-nls.hh \
 BaseDisplay.hh Timer.hh Util.hh Configuration.hh XAtom.hh Rootmenu.hh \
 Basemenu.hh Screen.hh Color.hh Texture.hh Image.hh Configmenu.hh \
 Iconmenu.hh Netizen.hh Workspace.hh Workspacemenu.hh
Screen.o: Screen.cc ../config.h i18n.hh ../nls/blackbox-nls.hh \
 blackbox.hh BaseDisplay.hh Timer.hh Util.hh Configuration.hh XAtom.hh \
 Clientmenu.hh Basemenu.hh Font.hh Screen.hh Color.hh Texture.hh \
 Image.hh Configmenu.hh Iconmenu.hh Netizen.hh Rootmenu.hh \
 Workspace.hh Workspacemenu.hh GCCache.hh Slit.hh Toolbar.hh Window.hh \
 Windowmenu.hh
Slit.o: Slit.cc i18n.hh ../nls/blackbox-nls.hh blackbox.hh \
 BaseDisplay.hh Timer.hh Util.hh Configuration.hh XAtom.hh Image.hh \
 Color.hh Screen.hh Texture.hh Configmenu.hh Basemenu.hh Iconmenu.hh \
 Netizen.hh Rootmenu.hh Workspace.hh Workspacemenu.hh Slit.hh \
 Toolbar.hh
Texture.o: Texture.cc Texture.hh Color.hh Util.hh BaseDisplay.hh \
 Timer.hh Image.hh Screen.hh Configmenu.hh Basemenu.hh Iconmenu.hh \
 Netizen.hh Rootmenu.hh Workspace.hh Workspacemenu.hh blackbox.hh \
 i18n.hh ../nls/blackbox-nls.hh Configuration.hh XAtom.hh
Timer.o: Timer.cc BaseDisplay.hh Timer.hh Util.hh
Toolbar.o: Toolbar.cc i18n.hh ../nls/blackbox-nls.hh blackbox.hh \
 BaseDisplay.hh Timer.hh Util.hh Configuration.hh XAtom.hh \
 Clientmenu.hh Basemenu.hh Font.hh Screen.hh Color.hh Texture.hh \
 Image.hh Configmenu.hh Iconmenu.hh Netizen.hh Rootmenu.hh \
 Workspace.hh Workspacemenu.hh GCCache.hh Toolbar.hh Window.hh \
 Windowmenu.hh Slit.hh
Util.o: Util.cc Util.hh
Window.o: Window.cc i18n.hh ../nls/blackbox-nls.hh blackbox.hh \
 BaseDisplay.hh Timer.hh Util.hh Configuration.hh XAtom.hh Font.hh \
 Screen.hh Color.hh Texture.hh Image.hh Configmenu.hh Basemenu.hh \
 Iconmenu.hh Netizen.hh Rootmenu.hh Workspace.hh Workspacemenu.hh \
 GCCache.hh Toolbar.hh Window.hh Windowmenu.hh Slit.hh
Windowmenu.o: Windowmenu.cc i18n.hh ../nls/blackbox-nls.hh blackbox.hh \
 BaseDisplay.hh Timer.hh Util.hh Configuration.hh XAtom.hh Screen.hh \
 Color.hh Texture.hh Image.hh Configmenu.hh Basemenu.hh Iconmenu.hh \
 Netizen.hh Rootmenu.hh Workspace.hh Workspacemenu.hh Window.hh \
 Windowmenu.hh
Workspace.o: Workspace.cc i18n.hh ../nls/blackbox-nls.hh blackbox.hh \
 BaseDisplay.hh Timer.hh Util.hh Configuration.hh XAtom.hh \
 Clientmenu.hh Basemenu.hh Netizen.hh Screen.hh Color.hh Texture.hh \
 Image.hh Configmenu.hh Iconmenu.hh Rootmenu.hh Workspace.hh \
 Workspacemenu.hh Toolbar.hh Window.hh Windowmenu.hh
Workspacemenu.o: Workspacemenu.cc i18n.hh ../nls/blackbox-nls.hh \
 blackbox.hh BaseDisplay.hh Timer.hh Util.hh Configuration.hh XAtom.hh \
 Screen.hh Color.hh Texture.hh Image.hh Configmenu.hh Basemenu.hh \
 Iconmenu.hh Netizen.hh Rootmenu.hh Workspace.hh Workspacemenu.hh \
 Toolbar.hh
XAtom.o: XAtom.cc ../config.h XAtom.hh Screen.hh Color.hh Texture.hh \
 Util.hh Image.hh Timer.hh BaseDisplay.hh Configmenu.hh Basemenu.hh \
 Iconmenu.hh Netizen.hh Rootmenu.hh Workspace.hh Workspacemenu.hh \
 blackbox.hh i18n.hh ../nls/blackbox-nls.hh Configuration.hh
blackbox.o: blackbox.cc i18n.hh ../nls/blackbox-nls.hh blackbox.hh \
 BaseDisplay.hh Timer.hh Util.hh Configuration.hh XAtom.hh Basemenu.hh \
 Clientmenu.hh GCCache.hh Color.hh Image.hh Rootmenu.hh Screen.hh \
 Texture.hh Configmenu.hh Iconmenu.hh Netizen.hh Workspace.hh \
 Workspacemenu.hh Slit.hh Toolbar.hh Window.hh Windowmenu.hh
i18n.o: i18n.cc i18n.hh ../nls/blackbox-nls.hh
main.o: main.cc ../version.h i18n.hh ../nls/blackbox-nls.hh \
 blackbox.hh BaseDisplay.hh Timer.hh Util.hh Configuration.hh XAtom.hh
