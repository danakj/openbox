###########################################################################
###          Functions for helping out with your window focus.          ###
###########################################################################

###########################################################################
###         Options that affect the behavior of the focus module.       ###
###########################################################################
AVOID_SKIP_TASKBAR = 1
"""Don't focus windows which have requested to not be displayed in taskbars.
   You will still be able to focus the windows, but not through cycling, and
   they won't be focused as a fallback if 'fallback' is enabled."""
FALLBACK = 0
"""Send focus somewhere when nothing is left with the focus, if possible."""
###########################################################################

###########################################################################
###########################################################################

###########################################################################
###      Internal stuff, should not be accessed outside the module.     ###
###########################################################################

import ob

# maintain a list of clients, stacked in focus order
_clients = []
_disable = 0

def _focusable(client, desktop):
    if not client.normal(): return 0
    if not (client.canFocus() or client.focusNotify()): return 0
    if AVOID_SKIP_TASKBAR and client.skipTaskbar(): return 0

    desk = client.desktop()
    if not (desk == 0xffffffff or desk == desktop): return 0

    return 1

def _remove(client):
    """This function exists because Swig pointers don't define a __eq__
       function, so list.remove(ptr) does not work."""
    win = client.window()
    for i in range(len(_clients)):
        if _clients[i].window() == win:
            _clients.pop(i)
            return
    raise ValueError("_remove(x): x not in _clients list.")

def _focused(data):
    global _clients

    if _disable: return

    if data.client:
        # move it to the top
        _remove(data.client)
        _clients.insert(0, data.client)
    elif FALLBACK:
        # pass around focus
        desktop = ob.openbox.screen(data.screen).desktop()
        for c in _clients:
            if _focusable(c, desktop) and c.focus():
                break

def _newwindow(data):
    _clients.append(data.client)
        
def _closewindow(data):
    _remove(data.client)

ob.ebind(ob.EventAction.NewWindow, _newwindow)
ob.ebind(ob.EventAction.CloseWindow, _closewindow)
ob.ebind(ob.EventAction.Focus, _focused)

print "Loaded focus.py"
