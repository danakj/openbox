import ob, otk
class _Cycle:
    """
    This is a basic cycling class for anything, from xOr's stackedcycle.py, 
    that pops up a cycling menu when there's more than one thing to be cycled
    to.
    An example of inheriting from and modifying this class is _CycleWindows,
    which allows users to cycle around windows.

    This class could conceivably be used to cycle through anything -- desktops,
    windows of a specific class, XMMS playlists, etc.
    """

    """This specifies a rough limit of characters for the cycling list titles.
       Titles which are larger will be chopped with an elipsis in their
       center."""
    TITLE_SIZE_LIMIT = 80

    """If this is non-zero then windows will be activated as they are
       highlighted in the cycling list (except iconified windows)."""
    ACTIVATE_WHILE_CYCLING = 0

    """If this is true, we start cycling with the next (or previous) thing 
       selected."""
    START_WITH_NEXT = 1

    """If this is true, a popup window will be displayed with the options
       while cycling."""
    SHOW_POPUP = 1

    def __init__(self):
        """Initialize an instance of this class.  Subclasses should 
           do any necessary event binding in their constructor as well.
           """
        self.cycling = 0   # internal var used for going through the menu
        self.items = []    # items to cycle through

        self.widget = None    # the otk menu widget
        self.menuwidgets = [] # labels in the otk menu widget TODO: RENAME

    def createPopup(self):
        """Creates the cycling popup menu.
        """
        self.widget = otk.Widget(self.screen.number(), ob.openbox,
                                 otk.Widget.Vertical, 0, 1)

    def destroyPopup(self):
        """Destroys (or rather, cleans up after) the cycling popup menu.
        """
        self.menuwidgets = []
        self.widget = 0

    def populateItems(self):
        """Populate self.items with the appropriate items that can currently 
           be cycled through.  self.items may be cleared out before this 
           method is called.
           """
        pass

    def menuLabel(self, item):
        """Return a string indicating the menu label for the given item.
           Don't worry about title truncation.
           """
        pass

    def itemEqual(self, item1, item2):
        """Compare two items, return 1 if they're "equal" for purposes of 
           cycling, and 0 otherwise.
           """
        # suggestion: define __eq__ on item classes so that this works 
        # in the general case.  :)
        return item1 == item2

    def populateLists(self):
        """Populates self.items and self.menuwidgets, and then shows and
           positions the cycling popup.  You probably shouldn't mess with 
           this function; instead, see populateItems and menuLabel.
           """
        self.widget.hide()

        try:
            current = self.items[self.menupos]
        except IndexError: 
            current = None
        oldpos = self.menupos
        self.menupos = -1

        self.items = []
        self.populateItems()

        # make the widgets
        i = 0
        self.menuwidgets = []
        for i in range(len(self.items)):
            c = self.items[i]

            w = otk.Label(self.widget)
            # current item might have shifted after a populateItems() 
            # call, so we need to do this test.
            if current and self.itemEqual(c, current):
                self.menupos = i
                w.setHilighted(1)
            self.menuwidgets.append(w)

            t = self.menuLabel(c)
            # TODO: maybe subclasses will want to truncate in different ways?
            if len(t) > self.TITLE_SIZE_LIMIT: # limit the length of titles
                t = t[:self.TITLE_SIZE_LIMIT / 2 - 2] + "..." + \
                    t[0 - self.TITLE_SIZE_LIMIT / 2 - 2:]
            w.setText(t)

        # The item we were on might be gone entirely
        if self.menupos < 0:
            # try stay at the same spot in the menu
            if oldpos >= len(self.items):
                self.menupos = len(self.items) - 1
            else:
                self.menupos = oldpos

        # find the size for the popup
        width = 0
        height = 0
        for w in self.menuwidgets:
            size = w.minSize()
            if size.width() > width: width = size.width()
            height += size.height()

        # show or hide the list and its child widgets
        if len(self.items) > 1:
            size = self.screeninfo.size()
            self.widget.moveresize(otk.Rect((size.width() - width) / 2,
                                            (size.height() - height) / 2,
                                            width, height))
            if self.SHOW_POPUP: self.widget.show(1)

    def activateTarget(self, final):
        """Activates (focuses and, if the user requested it, raises a window).
           If final is true, then this is the very last window we're activating
           and the user has finished cycling.
           """
        pass

    def setDataInfo(self, data):
        """Retrieve and/or calculate information when we start cycling, 
           preferably caching it.  Data is what's given to callback functions.
           """
        self.screen = ob.openbox.screen(data.screen)
        self.screeninfo = otk.display.screenInfo(data.screen)

    def chooseStartPos(self):
        """Set self.menupos to a number between 0 and len(self.items) - 1.
           By default the initial menupos is 0, but this can be used to change
           it to some other position."""
        pass

    def cycle(self, data, forward):
        """Does the actual job of cycling through windows.  data is a callback 
           parameter, while forward is a boolean indicating whether the
           cycling goes forwards (true) or backwards (false).
           """

        initial = 0

        if not self.cycling:
            ob.kgrab(data.screen, self.grabfunc)
            # the pointer grab causes pointer events during the keyboard grab
            # to go away, which means we don't get enter notifies when the
            # popup disappears, screwing up the focus
            ob.mgrab(data.screen)

            self.cycling = 1
            self.state = data.state
            self.menupos = 0

            self.setDataInfo(data)

            self.createPopup()
            self.items = [] # so it doesnt try start partway through the list
            self.populateLists()

            self.chooseStartPos()
            self.initpos = self.menupos

            initial = 1
        
        if not self.items: return # don't bother doing anything
        
        self.menuwidgets[self.menupos].setHighlighted(0)

        if initial and not self.START_WITH_NEXT:
            pass
        else:
            if forward:
                self.menupos += 1
            else:
                self.menupos -= 1
        # wrap around
        if self.menupos < 0: self.menupos = len(self.items) - 1
        elif self.menupos >= len(self.items): self.menupos = 0
        self.menuwidgets[self.menupos].setHighlighted(1)
        if self.ACTIVATE_WHILE_CYCLING:
            self.activateTarget(0) # activate, but dont deiconify/unshade/raise

    def grabfunc(self, data):
        """A callback method that grabs away all keystrokes so that navigating 
           the cycling menu is possible."""
        done = 0
        notreverting = 1
        # have all the modifiers this started with been released?
        if not self.state & data.state:
            done = 1
        elif data.action == ob.KeyAction.Press:
            # has Escape been pressed?
            if data.key == "Escape":
                done = 1
                notreverting = 0
                # revert
                self.menupos = self.initpos
            # has Enter been pressed?
            elif data.key == "Return":
                done = 1

        if done:
            # activate, and deiconify/unshade/raise
            self.activateTarget(notreverting)
            self.destroyPopup()
            self.cycling = 0
            ob.kungrab()
            ob.mungrab()

    def next(self, data):
        """Focus the next window."""
        self.cycle(data, 1)
        
    def previous(self, data):
        """Focus the previous window."""
        self.cycle(data, 0)

#---------------------- Window Cycling --------------------
import focus
class _CycleWindows(_Cycle):
    """
    This is a basic cycling class for Windows.

    An example of inheriting from and modifying this class is _ClassCycleWindows,
    which allows users to cycle around windows of a certain application
    name/class only.

    This class has an underscored name because I use the singleton pattern 
    (so CycleWindows is an actual instance of this class).  This doesn't have 
    to be followed, but if it isn't followed then the user will have to create 
    their own instances of your class and use that (not always a bad thing).

    An example of using the CycleWindows singleton:

        from cycle import CycleWindows
        CycleWindows.INCLUDE_ICONS = 0  # I don't like cycling to icons
        ob.kbind(["A-Tab"], ob.KeyContext.All, CycleWindows.next)
        ob.kbind(["A-S-Tab"], ob.KeyContext.All, CycleWindows.previous)
    """

    """If this is non-zero then windows from all desktops will be included in
       the stacking list."""
    INCLUDE_ALL_DESKTOPS = 0

    """If this is non-zero then windows which are iconified on the current 
       desktop will be included in the stacking list."""
    INCLUDE_ICONS = 1

    """If this is non-zero then windows which are iconified from all desktops
       will be included in the stacking list."""
    INCLUDE_ICONS_ALL_DESKTOPS = 1

    """If this is non-zero then windows which are on all-desktops at once will
       be included."""
    INCLUDE_OMNIPRESENT = 1

    """A better default for window cycling than generic cycling."""
    ACTIVATE_WHILE_CYCLING = 1

    """When cycling focus, raise the window chosen as well as focusing it."""
    RAISE_WINDOW = 1

    def __init__(self):
        _Cycle.__init__(self)

        def newwindow(data):
            if self.cycling: self.populateLists()
        def closewindow(data):
            if self.cycling: self.populateLists()

        ob.ebind(ob.EventAction.NewWindow, newwindow)
        ob.ebind(ob.EventAction.CloseWindow, closewindow)

    def shouldAdd(self, client):
        """Determines if a client should be added to the cycling list."""
        curdesk = self.screen.desktop()
        desk = client.desktop()

        if not client.normal(): return 0
        if not (client.canFocus() or client.focusNotify()): return 0
        if focus.AVOID_SKIP_TASKBAR and client.skipTaskbar(): return 0

        if client.iconic():
            if self.INCLUDE_ICONS:
                if self.INCLUDE_ICONS_ALL_DESKTOPS: return 1
                if desk == curdesk: return 1
            return 0
        if self.INCLUDE_OMNIPRESENT and desk == 0xffffffff: return 1
        if self.INCLUDE_ALL_DESKTOPS: return 1
        if desk == curdesk: return 1

        return 0

    def populateItems(self):
        # get the list of clients, keeping iconic windows at the bottom
        iconic_clients = []
        for c in focus._clients:
            if self.shouldAdd(c):
                if c.iconic(): iconic_clients.append(c)
                else: self.items.append(c)
        self.items.extend(iconic_clients)

    def menuLabel(self, client):
        if client.iconic(): t = '[' + client.iconTitle() + ']'
        else: t = client.title()

        if self.INCLUDE_ALL_DESKTOPS:
            d = client.desktop()
            if d == 0xffffffff: d = self.screen.desktop()
            t = self.screen.desktopName(d) + " - " + t

        return t
    
    def itemEqual(self, client1, client2):
        return client1.window() == client2.window()

    def activateTarget(self, final):
        """Activates (focuses and, if the user requested it, raises a window).
           If final is true, then this is the very last window we're activating
           and the user has finished cycling."""
        try:
            client = self.items[self.menupos]
        except IndexError: return # empty list

        # move the to client's desktop if required
        if not (client.iconic() or client.desktop() == 0xffffffff or \
                client.desktop() == self.screen.desktop()):
            root = self.screeninfo.rootWindow()
            ob.send_client_msg(root, otk.atoms.net_current_desktop,
                               root, client.desktop())
        
        # send a net_active_window message for the target
        if final or not client.iconic():
            if final: r = self.RAISE_WINDOW
            else: r = 0
            ob.send_client_msg(self.screeninfo.rootWindow(),
                               otk.atoms.openbox_active_window,
                               client.window(), final, r)
            if not final:
                focus._skip += 1

# The singleton.
CycleWindows = _CycleWindows()

#---------------------- Window Cycling --------------------
import focus
class _CycleWindowsLinear(_CycleWindows):
    """
    This class is an example of how to inherit from and make use of the
    _CycleWindows class.  This class also uses the singleton pattern.

    An example of using the CycleWindowsLinear singleton:

        from cycle import CycleWindowsLinear
        CycleWindows.ALL_DESKTOPS = 1  # I want all my windows in the list
        ob.kbind(["A-Tab"], ob.KeyContext.All, CycleWindowsLinear.next)
        ob.kbind(["A-S-Tab"], ob.KeyContext.All, CycleWindowsLinear.previous)
    """

    """When cycling focus, raise the window chosen as well as focusing it."""
    RAISE_WINDOW = 0

    """If this is true, a popup window will be displayed with the options
       while cycling."""
    SHOW_POPUP = 0

    def __init__(self):
        _CycleWindows.__init__(self)

    def shouldAdd(self, client):
        """Determines if a client should be added to the cycling list."""
        curdesk = self.screen.desktop()
        desk = client.desktop()

        if not client.normal(): return 0
        if not (client.canFocus() or client.focusNotify()): return 0
        if focus.AVOID_SKIP_TASKBAR and client.skipTaskbar(): return 0

        if client.iconic(): return 0
        if self.INCLUDE_OMNIPRESENT and desk == 0xffffffff: return 1
        if self.INCLUDE_ALL_DESKTOPS: return 1
        if desk == curdesk: return 1

        return 0

    def populateItems(self):
        # get the list of clients, keeping iconic windows at the bottom
        iconic_clients = []
        for c in self.screen.clients:
            if self.shouldAdd(c):
                self.items.append(c)

    def chooseStartPos(self):
        if focus._clients:
            t = focus._clients[0]
            for i,c in zip(range(len(self.items)), self.items):
                if self.itemEqual(c, t):
                    self.menupos = i
                    break
        
    def menuLabel(self, client):
        t = client.title()

        if self.INCLUDE_ALL_DESKTOPS:
            d = client.desktop()
            if d == 0xffffffff: d = self.screen.desktop()
            t = self.screen.desktopName(d) + " - " + t

        return t
    
# The singleton.
CycleWindowsLinear = _CycleWindowsLinear()

#----------------------- Desktop Cycling ------------------
class _CycleDesktops(_Cycle):
    """
    Example of usage:

       from cycle import CycleDesktops
       ob.kbind(["W-d"], ob.KeyContext.All, CycleDesktops.next)
       ob.kbind(["W-S-d"], ob.KeyContext.All, CycleDesktops.previous)
    """
    class Desktop:
        def __init__(self, name, index):
            self.name = name
            self.index = index
        def __eq__(self, other):
            return other.index == self.index

    def __init__(self):
        _Cycle.__init__(self)

    def populateItems(self):
        for i in range(self.screen.numDesktops()):
            self.items.append(
                _CycleDesktops.Desktop(self.screen.desktopName(i), i))

    def menuLabel(self, desktop):
        return desktop.name

    def chooseStartPos(self):
        self.menupos = self.screen.desktop()

    def activateTarget(self, final):
        # TODO: refactor this bit
        try:
            desktop = self.items[self.menupos]
        except IndexError: return

        root = self.screeninfo.rootWindow()
        ob.send_client_msg(root, otk.atoms.net_current_desktop,
                           root, desktop.index)

CycleDesktops = _CycleDesktops()

print "Loaded cycle.py"
