###########################################################################
### Functions for cycling focus (in a 'stacked' order) between windows. ###
###########################################################################

###########################################################################
###    Options that affect the behavior of the stackedcycle module.     ###
###              Also see the options in the focus module.              ###
###########################################################################
include_all_desktops = 0
"""If this is non-zero then windows from all desktops will be included in
   the stacking list."""
include_icons = 1
"""If this is non-zero then windows which are iconified will be included
   in the stacking list."""
include_omnipresent = 1
"""If this is non-zero then windows which are on all-desktops at once will
   be included."""
title_size_limit = 80
"""This specifies a rough limit of characters for the cycling list titles.
   Titles which are larger will be chopped with an elipsis in their
   center."""
activate_while_cycling = 1
"""If this is non-zero then windows will be activated as they are
   highlighted in the cycling list (except iconified windows)."""
###########################################################################

def next(data):
    """Focus the next window."""
    _o.cycle(data, 1)
    
def previous(data):
    """Focus the previous window."""
    _o.cycle(data, 0)

###########################################################################
###########################################################################

###########################################################################
###      Internal stuff, should not be accessed outside the module.     ###
###########################################################################

import otk
import ob
import focus

class cycledata:
    def __init__(self):
        self.cycling = 0

    def createpopup(self):
        self.style = self.screen.style()
        self.widget = otk.Widget(ob.openbox, self.style, otk.Widget.Vertical,
                                 0, self.style.bevelWidth(), 1)
        self.widget.setTexture(self.style.titlebarFocusBackground())

    def destroypopup(self):
        self.menuwidgets = []
        self.widget = 0

    def shouldadd(self, client):
        """Determines if a client should be added to the list."""
        curdesk = self.screen.desktop()
        desk = client.desktop()

        if not client.normal(): return 0
        if not (client.canFocus() or client.focusNotify()): return 0
        if focus.avoid_skip_taskbar and client.skipTaskbar(): return 0

        if include_icons and client.iconic(): return 1
        if include_omnipresent and desk == 0xffffffff: return 1
        if include_all_desktops: return 1
        if desk == curdesk: return 1

        return 0

    def populatelist(self):
        """Populates self.clients and self.menuwidgets, and then shows and
           positions the cycling popup."""

        self.widget.hide()

        try:
            current = self.clients[self.menupos]
        except IndexError: current = 0
        oldpos = self.menupos
        self.menupos = -1

        # get the list of clients
        self.clients = []
        for i in focus._clients:
            c = ob.openbox.findClient(i)
            if c: self.clients.append(c)

        font = self.style.labelFont()
        longest = 0
        height = font.height()
            
        # make the widgets
        i = 0
        self.menuwidgets = []
        while i < len(self.clients):
            c = self.clients[i]
            if not self.shouldadd(c):
                # make the clients and menuwidgets lists match
                self.clients.pop(i) 
                continue
            
            w = otk.FocusLabel(self.widget)
            if current and c.window() == current.window():
                self.menupos = i
                w.focus()
            else:
                w.unfocus()
            self.menuwidgets.append(w)

            if c.iconic(): t = c.iconTitle()
            else: t = c.title()
            if len(t) > title_size_limit: # limit the length of titles
                t = t[:title_size_limit / 2 - 2] + "..." + \
                    t[0 - title_size_limit / 2 - 2:]
            length = font.measureString(t)
            if length > longest: longest = length
            w.setText(t)

            i += 1

        # the window we were on may be gone
        if self.menupos < 0:
            # try stay at the same spot in the menu
            if oldpos >= len(self.clients):
                self.menupos = len(self.clients) - 1
            else:
                self.menupos = oldpos

        # fit to the largest item in the menu
        for w in self.menuwidgets:
            w.fitSize(longest, height)

        # show or hide the list and its child widgets
        if len(self.clients) > 1:
            area = self.screeninfo.rect()
            self.widget.update()
            self.widget.move(area.x() + (area.width() -
                                         self.widget.width()) / 2,
                             area.y() + (area.height() -
                                         self.widget.height()) / 2)
            self.widget.show(1)

    def activatetarget(self, final):
        try:
            client = self.clients[self.menupos]
        except IndexError: return # empty list makes for this

        # move the to client's desktop if required
        if not (client.iconic() or client.desktop() == 0xffffffff or \
                client.desktop() == self.screen.desktop()):
            root = self.screeninfo.rootWindow()
            ob.send_client_msg(root, otk.Property_atoms().net_current_desktop,
                               root, client.desktop())
        
        # send a net_active_window message for the target
        if final or not client.iconic():
            if final: r = focus.raise_window
            else: r = 0
            ob.send_client_msg(self.screeninfo.rootWindow(),
                               otk.Property_atoms().openbox_active_window,
                               client.window(), final, r)

    def cycle(self, data, forward):
        if not self.cycling:
            self.cycling = 1
            focus._disable = 1
            self.state = data.state
            self.screen = ob.openbox.screen(data.screen)
            self.screeninfo = otk.display.screenInfo(data.screen)
            self.menupos = 0
            self.createpopup()
            self.clients = [] # so it doesnt try start partway through the list
            self.populatelist()
        
            ob.kgrab(self.screen.number(), _grabfunc)
            # the pointer grab causes pointer events during the keyboard grab
            # to go away, which means we don't get enter notifies when the
            # popup disappears, screwing up the focus
            ob.mgrab(self.screen.number())

        self.menuwidgets[self.menupos].unfocus()
        if forward:
            self.menupos += 1
        else:
            self.menupos -= 1
        # wrap around
        if self.menupos < 0: self.menupos = len(self.clients) - 1
        elif self.menupos >= len(self.clients): self.menupos = 0
        self.menuwidgets[self.menupos].focus()
        if activate_while_cycling:
            self.activatetarget(0) # activate, but dont deiconify/unshade/raise

    def grabfunc(self, data):
        if data.action == ob.KeyAction.Release:
            # have all the modifiers this started with been released?
            if not self.state & data.state:
                self.cycling = 0
                focus._disable = 0
                self.activatetarget(1) # activate, and deiconify/unshade/raise
                self.destroypopup()
                ob.kungrab()
                ob.mungrab()


def _newwindow(data):
    if _o.cycling: _o.populatelist()
        
def _closewindow(data):
    if _o.cycling: _o.populatelist()
        
def _grabfunc(data):
    _o.grabfunc(data)

ob.ebind(ob.EventAction.NewWindow, _newwindow)
ob.ebind(ob.EventAction.CloseWindow, _closewindow)

_o = cycledata()
