# src/Makefile.am for Openbox
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

CPPFLAGS= @CPPFLAGS@ @SHAPE@ @SLIT@ @INTERLACE@ @ORDEREDPSEUDO@ \
@DEBUG@ @NEWWMSPEC@ @NLS@ @TIMEDCACHE@ @CLOBBER@ \
-DLOCALEPATH=\"$(pkgdatadir)/nls\" \
-DDEFAULTMENU=\"$(DEFAULT_MENU)\" \
-DDEFAULTSTYLE=\"$(DEFAULT_STYLE)\"

bin_PROGRAMS= openbox

openbox_SOURCES= BaseDisplay.cc Basemenu.cc Clientmenu.cc Configmenu.cc Iconmenu.cc Image.cc LinkedList.cc Netizen.cc Resource.cc Rootmenu.cc Screen.cc Slit.cc Timer.cc Toolbar.cc Window.cc Windowmenu.cc Workspace.cc Workspacemenu.cc openbox.cc bsd-snprintf.c i18n.cc main.cc

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	rm -f *\~ *.orig *.rej

# local dependencies

Resource.o: Resource.cc Resource.h
BaseDisplay.o: BaseDisplay.cc i18n.h BaseDisplay.h LinkedList.h \
 Timer.h
Basemenu.o: Basemenu.cc i18n.h openbox.h BaseDisplay.h \
 LinkedList.h Timer.h Image.h Window.h Iconmenu.h Basemenu.h \
 Windowmenu.h Slit.h Screen.h Configmenu.h Netizen.h Rootmenu.h \
 Workspace.h Workspacemenu.h
Clientmenu.o: Clientmenu.cc openbox.h BaseDisplay.h LinkedList.h \
 Timer.h Image.h Window.h Iconmenu.h Basemenu.h Windowmenu.h Slit.h \
 Clientmenu.h Workspace.h Screen.h Configmenu.h Netizen.h \
 Rootmenu.h Workspacemenu.h
Configmenu.o: Configmenu.cc i18n.h Configmenu.h Basemenu.h \
 LinkedList.h Screen.h BaseDisplay.h Timer.h Iconmenu.h Netizen.h \
 Rootmenu.h Workspace.h Workspacemenu.h openbox.h Image.h \
 Window.h Windowmenu.h Slit.h Toolbar.h
Icon.o: Iconmenu.cc i18n.h openbox.h BaseDisplay.h LinkedList.h \
 Timer.h Image.h Window.h Iconmenu.h Basemenu.h Windowmenu.h Slit.h \
 Screen.h Configmenu.h Netizen.h Rootmenu.h Workspace.h \
 Workspacemenu.h Toolbar.h
Image.o: Image.cc i18n.h BaseDisplay.h LinkedList.h Timer.h \
 Image.h
LinkedList.o: LinkedList.cc LinkedList.h
Netizen.o: Netizen.cc Netizen.h BaseDisplay.h LinkedList.h Timer.h \
 Screen.h Configmenu.h Basemenu.h openbox.h Image.h Window.h \
 Iconmenu.h Windowmenu.h Slit.h Rootmenu.h Workspace.h \
 Workspacemenu.h
Rootmenu.o: Rootmenu.cc openbox.h BaseDisplay.h LinkedList.h \
 Timer.h Image.h Window.h Iconmenu.h Basemenu.h Windowmenu.h Slit.h \
 Rootmenu.h Screen.h Configmenu.h Netizen.h Workspace.h \
 Workspacemenu.h
Screen.o: Screen.cc i18n.h bsd-snprintf.h openbox.h BaseDisplay.h \
 LinkedList.h Timer.h Image.h Window.h Iconmenu.h Basemenu.h \
 Windowmenu.h Slit.h Clientmenu.h Workspace.h Screen.h Configmenu.h \
 Netizen.h Rootmenu.h Workspacemenu.h Toolbar.h
Slit.o: Slit.cc i18n.h openbox.h BaseDisplay.h LinkedList.h \
 Timer.h Image.h Window.h Iconmenu.h Basemenu.h Windowmenu.h Slit.h \
 Screen.h Configmenu.h Netizen.h Rootmenu.h Workspace.h \
 Workspacemenu.h Toolbar.h
Timer.o: Timer.cc BaseDisplay.h LinkedList.h Timer.h
Toolbar.o: Toolbar.cc i18n.h openbox.h BaseDisplay.h LinkedList.h \
 Timer.h Image.h Window.h Iconmenu.h Basemenu.h Windowmenu.h Slit.h \
 Clientmenu.h Workspace.h Rootmenu.h Screen.h Configmenu.h \
 Netizen.h Workspacemenu.h Toolbar.h
Window.o: Window.cc i18n.h openbox.h BaseDisplay.h LinkedList.h \
 Timer.h Image.h Window.h Iconmenu.h Basemenu.h Windowmenu.h Slit.h \
 Screen.h Configmenu.h Netizen.h Rootmenu.h Workspace.h \
 Workspacemenu.h Toolbar.h
Windowmenu.o: Windowmenu.cc i18n.h openbox.h BaseDisplay.h \
 LinkedList.h Timer.h Image.h Window.h Iconmenu.h Basemenu.h \
 Windowmenu.h Slit.h Screen.h Configmenu.h Netizen.h Rootmenu.h \
 Workspace.h Workspacemenu.h
Workspace.o: Workspace.cc i18n.h openbox.h BaseDisplay.h \
 LinkedList.h Timer.h Image.h Window.h Iconmenu.h Basemenu.h \
 Windowmenu.h Slit.h Clientmenu.h Workspace.h Screen.h \
 Configmenu.h Netizen.h Rootmenu.h Workspacemenu.h Toolbar.h
Workspacemenu.o: Workspacemenu.cc i18n.h openbox.h BaseDisplay.h \
 LinkedList.h Timer.h Image.h Window.h Iconmenu.h Basemenu.h \
 Windowmenu.h Slit.h Screen.h Configmenu.h Netizen.h Rootmenu.h \
 Workspace.h Workspacemenu.h Toolbar.h
openbox.o: openbox.cc i18n.h openbox.h BaseDisplay.h \
 LinkedList.h Timer.h Image.h Window.h Iconmenu.h Basemenu.h \
 Windowmenu.h Slit.h Clientmenu.h Workspace.h Rootmenu.h \
 Screen.h Configmenu.h Netizen.h Workspacemenu.h Toolbar.h
bsd-snprintf.o: bsd-snprintf.c bsd-snprintf.h
i18n.o: i18n.cc i18n.h
main.o: main.cc ../version.h i18n.h openbox.h BaseDisplay.h \
 LinkedList.h Timer.h Image.h Window.h Iconmenu.h Basemenu.h \
 Windowmenu.h Slit.h
