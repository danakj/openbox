###########################################################################
###          Functions for helping out with your window focus.          ###
###########################################################################

ob_focus_raise = 1
ob_focus_fallback = 0
ob_focus_stack = []

def ob_focused(data):
    global ob_focus_raise
    global ob_focus_fallback
    global ob_focus_stack
    if data.client:
        window = data.client.window()
        # add/move to front the stack
        if window in ob_focus_stack:
            ob_focus_stack.remove(window)
        ob_focus_stack.insert(0, window)
    elif ob_focus_fallback:
        # pass around focus
        desktop = openbox.screen(data.screen).desktop()
        l = len(ob_focus_stack)
        i = 0
        while i < l:
            w = ob_focus_stack[i]
            client = openbox.findClient(w)
            if not client: # window is gone, remove it
                ob_focus_stack.pop(i)
                l = l - 1
            elif client.desktop() == desktop and \
                     client.normal() and client.focus():
                break
            else:
                i = i + 1

ebind(EventFocus, ob_focused)

def focus_next(data, num=1, forward=1):
    """Focus the next (or previous, with forward=0) window in a linear
       order."""
    screen = openbox.screen(data.screen)
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
                break
            elif screen.client(i).window() == client_win:
                found = 1
        if not found: # wraparound
            if forward: target = 0
            else: target = count - 1

    t = target
    curdesk = screen.desktop()
    while 1:
        client = screen.client(t)
        if client.normal() and \
               (client.desktop() == curdesk or client.desktop() == 0xffffffff)\
               and client.focus():
            if ob_focus_raise:
                screen.raiseWindow(client)
            return
        if forward:
            t += 1
            if t == count: t = 0
        else:
            t -= 1
            if t < 0: t = count - 1
        if t == target: return # nothing to focus

def focus_prev(data, num=1):
    """Focus the previous window in a linear order."""
    focus_next(data, num, forward=0)


print "Loaded focus.py"
