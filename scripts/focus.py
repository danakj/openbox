###########################################################################
###          Functions for helping out with your window focus.          ###
###########################################################################

# raise the window also when it is focused
ob_focus_raise = 1
# send focus somewhere when nothing is left with the focus if possible
ob_focus_fallback = 0

# maintain a list of clients, stacked in focus order
ob_clients = []
# maintaint he current focused window
ob_doing_stacked = 0

def ob_new_win(data):
    global ob_clients
    global ob_doing_stacked
    global ob_cyc_w;

    if ob_doing_stacked:
        ob_clients.insert(ob_clients.index(ob_cyc_w), data.client.window())
    else:
        if not len(ob_clients):
            ob_clients.append(data.client.window())
        else:
            ob_clients.insert(1, data.client.window()) # insert in 2nd slot

def ob_close_win(data):
    global ob_clients
    global ob_cyc_w;
    global ob_doing_stacked

    if not ob_doing_stacked:
        # not in the middle of stacked cycling, so who cares
        ob_clients.remove(data.client.window())
    else:
        # have to fix the cycling if we remove anything
        win = data.client.window()
        if ob_cyc_w == win:
            do_stacked_cycle(data) # cycle off the window first
        ob_clients.remove(win)

def ob_focused(data):
    global ob_clients
    global ob_doing_stacked
    global ob_cyc_w
    
    if data.client:
        if not ob_doing_stacked: # only move the window when we're not cycling
            win = data.client.window()
            # move it to the top
            ob_clients.remove(win)
            ob_clients.insert(0, win)
        else: # if we are cycling, then update our pointer
            ob_cyc_w = data.client.window()
    elif ob_focus_fallback: 
        # pass around focus
        desktop = openbox.screen(ob_cyc_screen).desktop()
        for w in ob_clients:
            client = openbox.findClient(w)
            if client and (client.desktop() == desktop and \
                           client.normal() and client.focus()):
                break

ebind(EventNewWindow, ob_new_win)
ebind(EventCloseWindow, ob_close_win)
ebind(EventFocus, ob_focused)

ob_cyc_mask = 0
ob_cyc_key = 0
ob_cyc_w = 0 # last window cycled to
ob_cyc_screen = 0

def do_stacked_cycle(data):
    global ob_cyc_w

    try:
        i = ob_clients.index(ob_cyc_w) + 1
    except ValueError:
        i = 0
        
    clients = ob_clients[i:] + ob_clients[:i]
    for w in clients:
        client = openbox.findClient(w)
        if client and (client.desktop() == desktop and \
                       client.normal() and client.focus()):
            return

def focus_next_stacked_grab(data):
    global ob_cyc_mask;
    global ob_cyc_key;
    global ob_cyc_w;
    global ob_doing_stacked;

    if data.action == EventKeyRelease:
        # have all the modifiers this started with been released?
        if not ob_cyc_mask & data.state:
            kungrab() # ungrab ourself
            ob_doing_stacked = 0;
            print "UNGRABBED!"
    else:
        if ob_cyc_key == data.key:
            # the next window to try focusing in ob_clients[ob_cyc_i]
            print "CYCLING!!"
            do_stacked_cycle(data)

def focus_next_stacked(data, forward=1):
    global ob_cyc_mask
    global ob_cyc_key
    global ob_cyc_w
    global ob_cyc_screen
    global ob_doing_stacked
    ob_cyc_mask = data.state
    ob_cyc_key = data.key
    ob_cyc_w = 0
    ob_cyc_screen = data.screen
    ob_doing_stacked = 1

    kgrab(data.screen, focus_next_stacked_grab)
    print "GRABBED!"
    focus_next_stacked_grab(data) # start with the first press

def focus_prev_stacked(data):
    return

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
                found = 2
                break
            elif screen.client(i).window() == client_win:
                found = 1
        if found == 1: # wraparound
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
            t += num
            if t >= count: t -= count
        else:
            t -= num
            if t < 0: t += count
        if t == target: return # nothing to focus

def focus_prev(data, num=1):
    """Focus the previous window in a linear order."""
    focus_next(data, num, forward=0)


print "Loaded focus.py"
