###########################################################################
### Functions for cycling focus (in a 'linear' order) between windows.  ###
###########################################################################

###########################################################################
###     Options that affect the behavior of the focuscycle module.      ###
###########################################################################
RAISE_WINDOW = 1
"""When cycling focus, raise the window chosen as well as focusing it. This
   does not affect fallback focusing behavior."""
# See focus.AVOID_SKIP_TASKBAR
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

import ob
import focus

def _cycle(data, num, forward):
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
            if client and focus._focusable(client, desktop) and client.focus():
                if RAISE_WINDOW:
                    screen.raiseWindow(client)
                return
            if forward:
                t += num
                if t >= count: t -= count
            else:
                t -= num
                if t < 0: t += count
            if t == target: return # nothing to focus
            
print "Loaded focuscycle.py"
