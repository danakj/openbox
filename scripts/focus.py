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

def _focused(data):
    global _clients

    if _disable: return

    if data.client:
        win = data.client.window()
        # move it to the top
        _clients.remove(win)
        _clients.insert(0, win)
    elif FALLBACK:
        # pass around focus
        desktop = ob.openbox.screen(data.screen).desktop()
        for w in _clients:
            client = ob.openbox.findClient(w)
            if client and _focusable(client, desktop) and client.focus():
                break

def _newwindow(data):
    _clients.append(data.client.window())
        
def _closewindow(data):
    try:
        _clients.remove(data.client.window())
    except ValueError: pass

ob.ebind(ob.EventAction.NewWindow, _newwindow)
ob.ebind(ob.EventAction.CloseWindow, _closewindow)
ob.ebind(ob.EventAction.Focus, _focused)

print "Loaded focus.py"
