import ob, config, hooks, focus
from input import Pointer, Keyboard

config.add('stackedcycle',
           'activate_while_cycling',
           'Activate While Cycling',
           "If this is True then windows will be activated as they are" + \
           "highlighted in the cycling list (except iconified windows).",
           'boolean',
           True)

config.add('stackedcycle',
           'raise_window',
           'Raise After Cycling',
           "If this is True, the selected window will be raised as well as " +\
           "focused.",
           'boolean',
           True)

config.add('stackedcycle',
           'include_all_desktops',
           'Include Windows From All Desktops',
           "If this is True then windows from all desktops will be included" +\
           " in the stacking list.",
           'boolean',
           False)

config.add('stackedcycle',
           'include_icons',
           'Include Icons',
           "If this is True then windows which are iconified on the current" +\
           " desktop will be included in the stacking list.",
           'boolean',
           False)

config.add('stackedcycle',
           'include_icons_all_desktops',
           'Include Icons From All Desktops',
           "If this is True then windows which are iconified from all " +\
           "desktops will be included in the stacking list (if Include Icons"+\
           " is also True).",
           'boolean',
           True)

config.add('stackedcycle',
           'include_omnipresent',
           'Include Omnipresent Windows',
           "If this is True then windows which are on all-desktops at once " +\
           "will be included in the stacking list.",
           'boolean',
           True)

def next(keydata, client): _cycle(keydata, client, True)
def previous(keydata, client): _cycle(keydata, client, False)

def _shouldAdd(client):
    """Determines if a client should be added to the cycling list."""
    curdesk = ob.Openbox.desktop()
    desk = client.desktop()

    if not (client.normal() and client.canFocus()): return False
    if config.get('focus', 'avoid_skip_taskbar') and client.skipTaskbar():
        return False

    if client.iconic():
        if config.get('stackedcycle', 'include_icons'):
            if config.get('stackedcycle', 'include_icons_all_desktops'):
                return True
            if desk == curdesk: return True
        return False
    if config.get('stackedcycle', 'include_omnipresent') and \
       desk == 0xffffffff: return True
    if config.get('stackedcycle', 'include_all_desktops'): return True
    if desk == curdesk: return True

    return False

def _populateItems():
    global _items
    # get the list of clients, keeping iconic windows at the bottom
    _items = []
    iconic_clients = []
    for c in focus._clients:
        if _shouldAdd(c):
            if c.iconic(): iconic_clients.append(c)
            else: _items.append(c)
    _items.extend(iconic_clients)

def _populate():
    global _pos, _items
    try:
        current = _items[_pos]
    except IndexError: 
        current = None
    oldpos = _pos
    _pos = -1

    _populateItems()

    i = 0
    for item in _items:
        # current item might have shifted after a populateItems() 
        # call, so we need to do this test.
        if current == item:
            _pos = i
        i += 1

        # The item we were on might be gone entirely
        if _pos < 0:
            # try stay at the same spot in the menu
            if oldpos >= len(_items):
                _pos = len(_items) - 1
            else:
                _pos = oldpos


def _activate(final):
    """
    Activates (focuses and, if the user requested it, raises a window).
    If final is True, then this is the very last window we're activating
    and the user has finished cycling.
    """
    print "_activate"
    try:
        client = _items[_pos]
    except IndexError: return # empty list
    print client

    # move the to client's desktop if required
    if not (client.iconic() or client.desktop() == 0xffffffff or \
            client.desktop() == ob.Openbox.desktop()):
        ob.Openbox.setDesktop(client.desktop())
        
    if final or not client.iconic():
        if final: r = config.get('stackedcycle', 'raise_window')
        else: r = False
        client.focus(True)
        if final and client.shaded(): client.setShaded(False)
        print "final", final, "raising", r
        if r: client.raiseWindow()
        if not final:
            focus._skip += 1

def _cycle(keydata, client, forward):
    global _cycling, _state, _pos, _inititem, _items

    if not _cycling:
        _items = [] # so it doesnt try start partway through the list
        _populate()

        if not _items: return # don't bother doing anything

        Keyboard.grab(_grabfunc)
        # the pointer grab causes pointer events during the keyboard grab
        # to go away, which means we don't get enter notifies when the
        # popup disappears, screwing up the focus
        Pointer.grabPointer(True)

        _cycling = True
        _state = keydata.state
        _pos = 0
        _inititem = _items[_pos]

    if forward:
        _pos += 1
    else:
        _pos -= 1
    # wrap around
    if _pos < 0: _pos = len(_items) - 1
    elif _pos >= len(_items): _pos = 0
    if config.get('stackedcycle', 'activate_while_cycling'):
        _activate(False) # activate, but dont deiconify/unshade/raise

def _grabfunc(keydata, client):
    global _cycling
    
    done = False
    notreverting = True
    # have all the modifiers this started with been released?
    if not _state & keydata.state:
        done = True
    elif keydata.press:
        # has Escape been pressed?
        if keydata.keychain == "Escape":
            done = True
            notreverting = False
            # revert
            try:
                _pos = _items.index(_inititem)
            except:
                _pos = -1
        # has Enter been pressed?
        elif keydata.keychain == "Return":
            done = True

    if done:
        # activate, and deiconify/unshade/raise
        _activate(notreverting)
        _cycling = False
        Keyboard.ungrab()
        Pointer.grabPointer(False)


_cycling  = False
_pos = 0
_inititem = None
_items = []
_state = 0

def _newwin(data):
    if _cycling: _populate()
def _closewin(data):
    if _cycling: _populate()

hooks.managed.append(_newwin)
hooks.closed.append(_closewin)
