###########################################################################
###          Functions for helping out with your window focus.          ###
###########################################################################

ob_focus_raise = 1
ob_focus_fallback = 0

# maintain a list of clients, stacked in focus order
ob_clients = []
# maintaint he current focused window
ob_focused = 0
ob_hold_client_list = 0

def ob_new_win(data):
    global ob_clients
    if not len(ob_clients): ob_clients.append(data.client.window())
    else: ob_clients.insert(1, data.client.window()) # insert in 2nd slot

def ob_close_win(data):
    global ob_clients
    ob_clients.remove(data.client.window())

def ob_focused(data):
    global ob_clients
    if data.client:
        if not ob_hold_client_list:
            win = data.client.window()
            ob_focused = win
            # move it to the top
            ob_clients.remove(win)
            ob_clients.insert(0, win)
    elif ob_focus_fallback:
        ob_old_client_list = 0 # something is wrong.. stop holding
        # pass around focus
        ob_focused = 0
        desktop = openbox.screen(data.screen).desktop()
        for w in ob_clients:
            client = openbox.findClient(w)
            if client and (client.desktop() == desktop and \
                           client.normal() and client.focus()):
                break

ebind(EventNewWindow, ob_new_win)
ebind(EventCloseWindow, ob_close_win)
ebind(EventFocus, ob_focused)

ob_cyc_mask = 0
ob_cyc_key = 0;

def focus_next_stacked_grab(data):
    global ob_cyc_mask;
    global ob_cyc_key;

    if data.action == EventKeyRelease:
        print "release: " + str(ob_cyc_mask) + "..." + str(data.state)
        # have all the modifiers this started with been released?
        if not ob_cyc_mask & data.state:
            kungrab() # ungrab ourself
            print "UNGRABBED!"
    else:
        print "press: " + str(ob_cyc_mask) + "..." + str(data.state) + \
              "..." + data.key
        if ob_cyc_key == data.key:
            print "CYCLING!!"

def focus_next_stacked(data, forward=1):
    global ob_cyc_mask;
    global ob_cyc_key;
    ob_cyc_mask = data.state
    ob_cyc_key = data.key

    kgrab(focus_next_stacked_grab)
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
