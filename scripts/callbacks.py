############################################################################
### Functions that can be used as callbacks for mouse/keyboard bindings  ###
############################################################################

#############################################################################
### Options that can be modified to change the default hooks' behaviors.  ###
###                                                                       ###
#############################################################################

import ob
import otk

def state_above(data, add=2):
    """Toggles, adds or removes the 'above' state on a window."""
    if not data.client: return
    ob.send_client_msg(otk.display.screenInfo(data.screen).rootWindow(),
                       otk.Property_atoms().net_wm_state, data.client.window(),
                       add, otk.Property_atoms().net_wm_state_above)
    
def state_below(data, add=2):
    """Toggles, adds or removes the 'below' state on a window."""
    if not data.client: return
    ob.send_client_msg(otk.display.screenInfo(data.screen).rootWindow(),
                       otk.Property_atoms().net_wm_state, data.client.window(),
                       add, otk.Property_atoms().net_wm_state_below)
    
def state_shaded(data, add=2):
    """Toggles, adds or removes the 'shaded' state on a window."""
    if not data.client: return
    ob.send_client_msg(otk.display.screenInfo(data.screen).rootWindow(),
                       otk.Property_atoms().net_wm_state, data.client.window(),
                       add, otk.Property_atoms().net_wm_state_shaded)

def state_skip_taskbar(data, add=2):
    """Toggles, adds or removes the 'skip_taskbar' state on a window."""
    if not data.client: return
    ob.send_client_msg(otk.display.screenInfo(data.screen).rootWindow(),
                       otk.Property_atoms().net_wm_state, data.client.window(),
                       add, otk.Property_atoms().net_wm_state_skip_taskbar)
    
def state_skip_pager(data, add=2):
    """Toggles, adds or removes the 'skip_pager' state on a window."""
    if not data.client: return
    ob.send_client_msg(otk.display.screenInfo(data.screen).rootWindow(),
                       otk.Property_atoms().net_wm_state, data.client.window(),
                       add, otk.Property_atoms().net_wm_state_skip_pager)
    
def iconify(data):
    """Iconifies the window on which the event occured"""
    if not data.client: return
    ob.send_client_msg(otk.display.screenInfo(data.screen).rootWindow(),
                       otk.Property_atoms().wm_change_state,
                       data.client.window(), 3) # IconicState
    
def restore(data):
    """Un-iconifies the window on which the event occured, but does not focus
       if. If you want to focus the window too, it is recommended that you
       use the activate() function."""
    if not data.client: return
    ob.send_client_msg(otk.display.screenInfo(data.screen).rootWindow(),
                       otk.Property_atoms().wm_change_state,
                       data.client.window(), 1) # NormalState
    
def close(data):
    """Closes the window on which the event occured"""
    if not data.client: return
    ob.send_client_msg(otk.display.screenInfo(data.screen).rootWindow(),
                       otk.Property_atoms().net_close_window,
                       data.client.window(), 0)

def focus(data):
    """Focuses the window on which the event occured"""
    if not data.client: return
    # !normal windows dont get focus from window enter events
    if data.action == ob.EventAction.EnterWindow and not data.client.normal():
        return
    data.client.focus()

def restart(data, other = ""):
    """Restarts openbox, optionally starting another window manager."""
    ob.openbox.restart(other)

def raise_win(data):
    """Raises the window on which the event occured"""
    if not data.client: return
    ob.openbox.screen(data.screen).raiseWindow(data.client)

def lower_win(data):
    """Lowers the window on which the event occured"""
    if not data.client: return
    ob.openbox.screen(data.screen).lowerWindow(data.client)

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
    root = otk.display.screenInfo(data.screen).rootWindow()
    ob.send_client_msg(root, otk.Property_atoms().net_current_desktop,
                       root, num)

def next_desktop(data, no_wrap=0):
    """Switches to the next desktop, optionally (by default) cycling around to
       the first when going past the last."""
    screen = ob.openbox.screen(data.screen)
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
    screen = ob.openbox.screen(data.screen)
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
    ob.send_client_msg(otk.display.screenInfo(data.screen).rootWindow(),
                       otk.Property_atoms().net_wm_desktop,
                       data.client.window(),num)

def toggle_all_desktops(data):
    """Toggles between sending a client to all desktops and to the current
       desktop."""
    if not data.client: return
    if not data.client.desktop() == 0xffffffff:
        send_to_desktop(data, 0xffffffff)
    else:
        send_to_desktop(data, openbox.screen(data.screen).desktop())
    
def send_to_all_desktops(data):
    """Sends a client to all desktops"""
    if not data.client: return
    send_to_desktop(data, 0xffffffff)
    
def send_to_next_desktop(data, no_wrap=0, follow=1):
    """Sends a window to the next desktop, optionally (by default) cycling
       around to the first when going past the last. Also optionally moving to
       the new desktop after sending the window."""
    if not data.client: return
    screen = ob.openbox.screen(data.screen)
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
    screen = ob.openbox.screen(data.screen)
    d = screen.desktop()
    n = screen.numDesktops()
    if (d > 0):
        d = d - 1
    elif not no_wrap:
        d = n - 1
    send_to_desktop(data, d)
    if follow:
        change_desktop(data, d)

print "Loaded callbacks.py"
