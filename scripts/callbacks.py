############################################################################
### Functions that can be used as callbacks for mouse/keyboard bindings  ###
############################################################################

import ob
import otk

def iconify(data):
    """Iconifies the window on which the event occured"""
    if not data.client: return
    data.client.iconify(1)
    
def restore(data):
    """Un-iconifies the window on which the event occured, but does not focus
       if. If you want to focus the window too, it is recommended that you
       use the activate() function."""
    if not data.client: return
    data.client.iconify(0)
    
def close(data):
    """Closes the window on which the event occured"""
    if not data.client: return
    data.client.close()

def focus(data):
    """Focuses the window on which the event occured"""
    if not data.client: return
    # !normal windows dont get focus from window enter events
    if data.action == ob.EventAction.EnterWindow and not data.client.normal():
        return
    data.client.focus()

def raise_win(data):
    """Raises the window on which the event occured"""
    if not data.client: return
    data.client.raiseWindow()

def lower_win(data):
    """Lowers the window on which the event occured"""
    if not data.client: return
    data.client.lowerWindow()

def toggle_maximize(data):
    """Toggles the maximized status of the window on which the event occured"""
    if not data.client: return
    data.client.maximize(not (data.client.maxHorz() or data.client.maxVert()))

def toggle_maximize_horz(data):
    """Toggles the horizontal maximized status of the window on which the event
       occured"""
    if not data.client: return
    data.client.maximizeHorizontal(not data.client.maxHorz())

def toggle_maximize_vert(data):
    """Toggles the vertical maximized status of the window on which the event
       occured"""
    if not data.client: return
    data.client.maximizeVertical(not data.client.maxVert())

def maximize(data):
    """Maximizes the window on which the event occured"""
    if not data.client: return
    data.client.maximize(1)

def maximize_horz(data):
    """Horizontally maximizes the window on which the event occured"""
    if not data.client: return
    data.client.maximizeHorizontal(1)

def maximize_vert(data):
    """Vertically maximizes the window on which the event occured"""
    if not data.client: return
    data.client.maximizeVertical(1)

def unmaximize(data):
    """Unmaximizes the window on which the event occured"""
    if not data.client: return
    data.client.maximize(0)

def unmaximize_horz(data):
    """Horizontally unmaximizes the window on which the event occured"""
    if not data.client: return
    data.client.maximizeHorizontal(0)

def unmaximize_vert(data):
    """Vertically unmaximizes the window on which the event occured"""
    if not data.client: return
    data.client.maximizeVertical(0)

def toggle_shade(data):
    """Toggles the shade status of the window on which the event occured"""
    if not data.client: return
    data.client.shade(not data.client.shaded())

def shade(data):
    """Shades the window on which the event occured"""
    if not data.client: return
    data.client.shade(1)

def unshade(data):
    """Unshades the window on which the event occured"""
    if not data.client: return
    data.client.shade(0)

def change_desktop(data, num):
    """Switches to a specified desktop"""
    ob.openbox.screen(data.screen).changeDesktop(num)

def show_desktop(data, show=1):
    """Shows and focuses the desktop, hiding any client windows. Optionally,
       if show is zero, this will hide the desktop, leaving show-desktop
       mode."""
    ob.openbox.screen(data.screen).showDesktop(show)

def hide_desktop(data):
    """Hides the desktop, re-showing the client windows. Leaves show-desktop
       mode."""
    show_desktop(data, 0)

def toggle_show_desktop(data):
    """Requests the Openbox to show the desktop, hiding the client windows, or
       redisplay the clients."""
    screen = ob.openbox.screen(data.screen)
    screen.showDesktop(not screen.showingDesktop())

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

def up_desktop(data, num=1):
    """Switches to the desktop vertically above the current one. This is based
       on the desktop layout chosen by an EWMH compliant pager. Optionally, num
       can be specified to move more than one row at a time."""
    screen = ob.openbox.screen(data.screen)
    d = screen.desktop()
    n = screen.numDesktops()
    l = screen.desktopLayout()

    target = d - num * l.columns
    if target < 0:
        target += l.rows * l.columns
    while target >= n:
        target -= l.columns
    change_desktop(data, target)

def down_desktop(data, num=1):
    """Switches to the desktop vertically below the current one. This is based
       on the desktop layout chosen by an EWMH compliant pager. Optionally, num
       can be specified to move more than one row at a time."""
    screen = ob.openbox.screen(data.screen)
    d = screen.desktop()
    n = screen.numDesktops()
    l = screen.desktopLayout()

    target = d + num * l.columns
    if target >= n:
        target -= l.rows * l.columns
    while target < 0:
        target += l.columns
    change_desktop(data, target)

def left_desktop(data, num=1):
    """Switches to the desktop horizotally left of the current one. This is
       based on the desktop layout chosen by an EWMH compliant pager.
       Optionally, num can be specified to move more than one column at a
       time."""
    screen = ob.openbox.screen(data.screen)
    d = screen.desktop()
    n = screen.numDesktops()
    l = screen.desktopLayout()

    rowstart = d - d % l.columns
    target = d - num
    while target < rowstart:
        target += l.columns
    change_desktop(data, target)

def right_desktop(data, num=1):
    """Switches to the desktop horizotally right of the current one. This is
       based on the desktop layout chosen by an EWMH compliant pager.
       Optionally, num can be specified to move more than one column at a
       time."""
    screen = ob.openbox.screen(data.screen)
    d = screen.desktop()
    n = screen.numDesktops()
    l = screen.desktopLayout()

    rowstart = d - d % l.columns
    target = d + num
    while target >= rowstart + l.columns:
        target -= l.columns
    change_desktop(data, target)

def send_to_desktop(data, num):
    """Sends a client to a specified desktop"""
    if not data.client: return
    data.client.setDesktop(num)

def toggle_all_desktops(data):
    """Toggles between sending a client to all desktops and to the current
       desktop."""
    if not data.client: return
    if not data.client.desktop() == 0xffffffff:
        send_to_desktop(data, 0xffffffff)
    else:
        send_to_desktop(data, ob.openbox.screen(data.screen).desktop())
    
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

def restart(data=0, other = ""):
    """Restarts Openbox, optionally starting another window manager."""
    ob.openbox.restart(other)

def exit(data=0):
    """Exits Openbox."""
    ob.openbox.shutdown()

print "Loaded callbacks.py"
