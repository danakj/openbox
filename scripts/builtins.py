###########################################################################
### Functions that can be used as callbacks for mouse/keyboard bindings ###
###########################################################################

def state_above(data, add=2):
    """Toggles, adds or removes the 'above' state on a window."""
    if not data.client: return
    send_client_msg(OBDisplay_screenInfo(data.screen).rootWindow(),
                    OBProperty.net_wm_state, data.client.window(), add,
                    openbox.property().atom(OBProperty.net_wm_state_above))
    
def state_below(data, add=2):
    """Toggles, adds or removes the 'below' state on a window."""
    if not data.client: return
    send_client_msg(OBDisplay_screenInfo(data.screen).rootWindow(),
                    OBProperty.net_wm_state, data.client.window(), add,
                    openbox.property().atom(OBProperty.net_wm_state_below))
    
def state_shaded(data, add=2):
    """Toggles, adds or removes the 'shaded' state on a window."""
    if not data.client: return
    send_client_msg(OBDisplay_screenInfo(data.screen).rootWindow(),
                    OBProperty.net_wm_state, data.client,window(), add,
                    openbox.property().atom(OBProperty.net_wm_state_shaded))
    
def close(data):
    """Closes the window on which the event occured"""
    if not data.client: return
    send_client_msg(OBDisplay_screenInfo(data.screen).rootWindow(),
                    OBProperty.net_close_window, data.client.window(), 0)

def focus(data):
    """Focuses the window on which the event occured"""
    if not data.client: return
    # !normal windows dont get focus from window enter events
    if data.action == EventEnterWindow and not data.client.normal():
        return
    data.client.focus()

def move(data):
    """Moves the window interactively. This should only be used with
       MouseMotion events"""
    if not data.client: return

    # !normal windows dont get moved
    if not data.client.normal(): return

    dx = data.xroot - data.pressx
    dy = data.yroot - data.pressy
    data.client.move(data.press_clientx + dx, data.press_clienty + dy)

def resize(data):
    """Resizes the window interactively. This should only be used with
       MouseMotion events"""
    if not data.client: return

    # !normal windows dont get moved
    if not data.client.normal(): return

    px = data.pressx
    py = data.pressy
    dx = data.xroot - px
    dy = data.yroot - py

    # pick a corner to anchor
    if not (resize_nearest or data.context == MC_Grip):
        corner = OBClient.TopLeft
    else:
        x = px - data.press_clientx
        y = py - data.press_clienty
        if y < data.press_clientheight / 2:
            if x < data.press_clientwidth / 2:
                corner = OBClient.BottomRight
                dx *= -1
            else:
                corner = OBClient.BottomLeft
            dy *= -1
        else:
            if x < data.press_clientwidth / 2:
                corner = OBClient.TopRight
                dx *= -1
            else:
                corner = OBClient.TopLeft

    data.client.resize(corner,
                       data.press_clientwidth + dx,
                       data.press_clientheight + dy);

def restart(data):
    """Restarts openbox"""
    openbox.restart("")

def raise_win(data):
    """Raises the window on which the event occured"""
    if not data.client: return
    openbox.screen(data.screen).restack(1, data.client)

def lower_win(data):
    """Lowers the window on which the event occured"""
    if not data.client: return
    openbox.screen(data.screen).restack(0, data.client)

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
    root = OBDisplay_screenInfo(data.screen).rootWindow()
    send_client_msg(root, OBProperty.net_current_desktop, root, num)

def next_desktop(data, no_wrap=0):
    """Switches to the next desktop, optionally (by default) cycling around to
       the first when going past the last."""
    screen = openbox.screen(data.screen)
    d = screen.desktop()
    n = screen.numDesktops()
    if (d < (n-1)):
        d = d + 1
    elif not no_wrap:
        d = 0
    change_desktop(data, d)
    
def prev_desktop(data, no_wrap=0):
    """Switches to the previous desktop, optionally (by default) cycling around
       to the last when going past the first."""
    screen = openbox.screen(data.screen)
    d = screen.desktop()
    n = screen.numDesktops()
    if (d > 0):
        d = d - 1
    elif not no_wrap:
        d = n - 1
    change_desktop(data, d)

def send_to_desktop(data, num):
    """Sends a client to a specified desktop"""
    if not data.client: return
    send_client_msg(OBDisplay_screenInfo(data.screen).rootWindow(),
                    OBProperty.net_wm_desktop, data.client.window(), num)

def send_to_next_desktop(data, no_wrap=0, follow=1):
    """Sends a window to the next desktop, optionally (by default) cycling
       around to the first when going past the last. Also optionally moving to
       the new desktop after sending the window."""
    if not data.client: return
    screen = openbox.screen(data.screen)
    d = screen.desktop()
    n = screen.numDesktops()
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
    if not data.client: return
    screen = openbox.screen(data.screen)
    d = screen.desktop()
    n = screen.numDesktops()
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
    openbox.execute(screen, bin)

print "Loaded builtins.py"
