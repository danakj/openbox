###########################################################################
###          Functions for helping out with your window focus.          ###
###########################################################################

###########################################################################
###         Options that affect the behavior of the focus module.       ###
###########################################################################
avoid_skip_taskbar = 1
"""Don't focus windows which have requested to not be displayed in taskbars.
   You will still be able to focus the windows, but not through cycling, and
   they won't be focused as a fallback if 'fallback' is enabled."""
raise_window = 1
"""When cycling focus, raise the window chosen as well as focusing it. This
   does not affect fallback focusing behavior."""
fallback = 0
"""Send focus somewhere when nothing is left with the focus, if possible."""
###########################################################################

def next(data, num=1):
    """Focus the next window."""
    _cycle(data, num, 1)

def previous(data, num=1):
    """Focus the previous window."""
    _cycle(data, num, 0)

###########################################################################
###########################################################################

###########################################################################
###      Internal stuff, should not be accessed outside the module.     ###
###########################################################################

import otk
import ob

# maintain a list of clients, stacked in focus order
_clients = []
_disable = 0

def _focusable(client, desktop):
    if not client.normal(): return 0
    if not (client.canFocus() or client.focusNotify()): return 0
    if avoid_skip_taskbar and client.skipTaskbar(): return 0

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
    elif fallback:
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
        focus._clients.remove(data.client.window())
    except ValueError: pass

ob.ebind(ob.EventAction.NewWindow, _newwindow)
ob.ebind(ob.EventAction.CloseWindow, _closewindow)
ob.ebind(ob.EventAction.Focus, _focused)

def _cycle(data, num, forward):
    global avoid_skip_taskbar
    
    screen = ob.openbox.screen(data.screen)
    count = screen.clientCount()

    if not count: return # no clients
    
    target = 0
    if data.client:
        client_win = data.client.window()
        found = 0
        r = range(count)
        if not forward:
            r.reverse()
        for i in r:
            if found:
                target = i
                found = 2
                break
            elif screen.client(i).window() == client_win:
                found = 1
        if found == 1: # wraparound
            if forward: target = 0
            else: target = count - 1

        t = target
        desktop = screen.desktop()
        while 1:
            client = screen.client(t)
            if client and _focusable(client, desktop) and client.focus():
                if cycle_raise:
                    screen.raiseWindow(client)
                return
            if forward:
                t += num
                if t >= count: t -= count
            else:
                t -= num
                if t < 0: t += count
            if t == target: return # nothing to focus
            
print "Loaded focus.py"
