###########################################################################
### Functions that can be used as callbacks for mouse/keyboard bindings ###
###########################################################################

def state_above(data, add=2):
    """Toggles, adds or removes the 'above' state on a window."""
    client = Openbox_findClient(openbox, data.window())
    if not client: return
    root = ScreenInfo_rootWindow(OBDisplay_screenInfo(data.screen()))
    window = OBClient_window(client)
    above = OBProperty_atom(Openbox_property(openbox),
                            OBProperty_net_wm_state_above)
    send_client_msg(root, OBProperty_net_wm_state, window, add,
                    above)
    
def state_below(data, add=2):
    """Toggles, adds or removes the 'below' state on a window."""
    client = Openbox_findClient(openbox, data.window())
    if not client: return
    root = ScreenInfo_rootWindow(OBDisplay_screenInfo(data.screen()))
    window = OBClient_window(client)
    below = OBProperty_atom(Openbox_property(openbox),
                            OBProperty_net_wm_state_below)
    send_client_msg(root, OBProperty_net_wm_state, window, add,
                    below)
    
def state_shaded(data, add=2):
    """Toggles, adds or removes the 'shaded' state on a window."""
    client = Openbox_findClient(openbox, data.window())
    if not client: return
    root = ScreenInfo_rootWindow(OBDisplay_screenInfo(data.screen()))
    window = OBClient_window(client)
    shaded = OBProperty_atom(Openbox_property(openbox),
                            OBProperty_net_wm_state_shaded)
    send_client_msg(root, OBProperty_net_wm_state, window, add,
                    shaded)
    
def close(data):
    """Closes the window on which the event occured"""
    client = Openbox_findClient(openbox, data.window())
    if not client: return
    root = ScreenInfo_rootWindow(OBDisplay_screenInfo(data.screen()))
    window = OBClient_window(client)
    send_client_msg(root, OBProperty_net_close_window, window)

def focus(data):
    """Focuses the window on which the event occured"""
    client = Openbox_findClient(openbox, data.window())
    if not client: return
    type = OBClient_type(client)
    # !normal windows dont get focus from window enter events
    if data.action() == EventEnterWindow and not OBClient_normal(client):
        return
    OBClient_focus(client)

def move(data):
    """Moves the window interactively. This should only be used with
       MouseMotion events"""
    client = Openbox_findClient(openbox, data.window())
    if not client: return

    # !normal windows dont get moved
    if not OBClient_normal(client): return

    dx = data.xroot() - data.pressx()
    dy = data.yroot() - data.pressy()
    OBClient_move(client, data.press_clientx() + dx, data.press_clienty() + dy)

def resize(data):
    """Resizes the window interactively. This should only be used with
       MouseMotion events"""
    client = Openbox_findClient(openbox, data.window())
    if not client: return

    # !normal windows dont get moved
    if not OBClient_normal(client): return

    px = data.pressx()
    py = data.pressy()
    dx = data.xroot() - px
    dy = data.yroot() - py

    # pick a corner to anchor
    if not (resize_nearest or data.context() == MC_Grip):
        corner = OBClient_TopLeft
    else:
        x = px - data.press_clientx()
        y = py - data.press_clienty()
        if y < data.press_clientheight() / 2:
            if x < data.press_clientwidth() / 2:
                corner = OBClient_BottomRight
                dx *= -1
            else:
                corner = OBClient_BottomLeft
            dy *= -1
        else:
            if x < data.press_clientwidth() / 2:
                corner = OBClient_TopRight
                dx *= -1
            else:
                corner = OBClient_TopLeft

    OBClient_resize(client, corner,
                    data.press_clientwidth() + dx,
                    data.press_clientheight() + dy);

def restart(data):
    """Restarts openbox"""
    Openbox_restart(openbox, "")

def raise_win(data):
    """Raises the window on which the event occured"""
    client = Openbox_findClient(openbox, data.window())
    if not client: return
    screen = Openbox_screen(openbox, OBClient_screen(client))
    OBScreen_restack(screen, 1, client)

def lower_win(data):
    """Lowers the window on which the event occured"""
    client = Openbox_findClient(openbox, data.window())
    if not client: return
    screen = Openbox_screen(openbox, OBClient_screen(client))
    OBScreen_restack(screen, 0, client)

def toggle_shade(data):
    """Toggles the shade status of the window on which the event occured"""
    state_shaded(data)

def shade(data):
    """Shades the window on which the event occured"""
    state_shaded(data, 1)

def unshade(data):
    """Unshades the window on which the event occured"""
    state_shaded(data, 0)

def change_desktop(data, num):
    """Switches to a specified desktop"""
    root = ScreenInfo_rootWindow(OBDisplay_screenInfo(data.screen()))
    send_client_msg(root, OBProperty_net_current_desktop, root, num)

def next_desktop(data, no_wrap=0):
    """Switches to the next desktop, optionally (by default) cycling around to
       the first when going past the last."""
    screen = Openbox_screen(openbox, data.screen())
    d = OBScreen_desktop(screen)
    n = OBScreen_numDesktops(screen)
    if (d < (n-1)):
        d = d + 1
    elif not no_wrap:
        d = 0
    change_desktop(data, d)
    
def prev_desktop(data, no_wrap=0):
    """Switches to the previous desktop, optionally (by default) cycling around
       to the last when going past the first."""
    screen = Openbox_screen(openbox, data.screen())
    d = OBScreen_desktop(screen)
    n = OBScreen_numDesktops(screen)
    if (d > 0):
        d = d - 1
    elif not no_wrap:
        d = n - 1
    change_desktop(data, d)

def send_to_desktop(data, num):
    """Sends a client to a specified desktop"""
    root = ScreenInfo_rootWindow(OBDisplay_screenInfo(data.screen()))
    client = Openbox_findClient(openbox, data.window())
    if client:
        window = OBClient_window(client)
        send_client_msg(root, OBProperty_net_wm_desktop, window, num)

def send_to_next_desktop(data, no_wrap=0, follow=1):
    """Sends a window to the next desktop, optionally (by default) cycling
       around to the first when going past the last. Also optionally moving to
       the new desktop after sending the window."""
    client = Openbox_findClient(openbox, data.window())
    if not client: return
    screen = Openbox_screen(openbox, data.screen())
    d = OBScreen_desktop(screen)
    n = OBScreen_numDesktops(screen)
    if (d < (n-1)):
        d = d + 1
    elif not no_wrap:
        d = 0
    send_to_desktop(data, d)
    if follow:
        change_desktop(data, d)
    
def send_to_prev_desktop(data, no_wrap=0, follow=1):
    """Sends a window to the previous desktop, optionally (by default) cycling
       around to the last when going past the first. Also optionally moving to
       the new desktop after sending the window."""
    client = Openbox_findClient(openbox, data.window())
    if not client: return
    screen = Openbox_screen(openbox, data.screen())
    d = OBScreen_desktop(screen)
    n = OBScreen_numDesktops(screen)
    if (d > 0):
        d = d - 1
    elif not no_wrap:
        d = n - 1
    send_to_desktop(data, d)
    if follow:
        change_desktop(data, d)

#########################################
### Convenience functions for scripts ###
#########################################

def execute(bin, screen = 0):
    """Executes a command on the specified screen. It is recommended that you
       use this call instead of a python system call. If the specified screen
       is beyond your range of screens, the default is used instead."""
    Openbox_execute(openbox, screen, bin)

print "Loaded builtins.py"
