###########################################################################
### Functions that can be used as callbacks for mouse/keyboard bindings ###
###########################################################################

def state_above(data, add=2):
    """Toggles, adds or removes the 'above' state on a window."""
    if not data.client: return
    send_client_msg(display.screenInfo(data.screen).rootWindow(),
                    Property_atoms().net_wm_state, data.client.window(), add,
                    Property_atoms().net_wm_state_above)
    
def state_below(data, add=2):
    """Toggles, adds or removes the 'below' state on a window."""
    if not data.client: return
    send_client_msg(display.screenInfo(data.screen).rootWindow(),
                    Property_atoms().net_wm_state, data.client.window(), add,
                    Property_atoms().net_wm_state_below)
    
def state_shaded(data, add=2):
    """Toggles, adds or removes the 'shaded' state on a window."""
    if not data.client: return
    send_client_msg(display.screenInfo(data.screen).rootWindow(),
                    Property_atoms().net_wm_state, data.client.window(), add,
                    Property_atoms().net_wm_state_shaded)
    
def close(data):
    """Closes the window on which the event occured"""
    if not data.client: return
    send_client_msg(display.screenInfo(data.screen).rootWindow(),
                    Property_atoms().net_close_window, data.client.window(), 0)

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
        corner = Client.TopLeft
    else:
        x = px - data.press_clientx
        y = py - data.press_clienty
        if y < data.press_clientheight / 2:
            if x < data.press_clientwidth / 2:
                corner = Client.BottomRight
                dx *= -1
            else:
                corner = Client.BottomLeft
            dy *= -1
        else:
            if x < data.press_clientwidth / 2:
                corner = Client.TopRight
                dx *= -1
            else:
                corner = Client.TopLeft

    data.client.resize(corner,
                       data.press_clientwidth + dx,
                       data.press_clientheight + dy);

def restart(data, other = ""):
    """Restarts openbox, optionally starting another window manager."""
    openbox.restart(other)

def raise_win(data):
    """Raises the window on which the event occured"""
    if not data.client: return
    openbox.screen(data.screen).raiseWindow(data.client)

def lower_win(data):
    """Lowers the window on which the event occured"""
    if not data.client: return
    openbox.screen(data.screen).lowerWindow(data.client)

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
    root = display.screenInfo(data.screen).rootWindow()
    send_client_msg(root, Property_atoms().net_current_desktop, root, num)

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
    send_client_msg(display.screenInfo(data.screen).rootWindow(),
                    Property_atoms().net_wm_desktop, data.client.window(), num)

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

def setup_click_focus(click_raise = 1):
    """Sets up for focusing windows by clicking on or in the window.
       Optionally, clicking on or in a window can raise the window to the
       front of its stacking layer."""
    mbind("Left", MC_Titlebar, MousePress, focus)
    mbind("Left", MC_Handle, MousePress, focus)
    mbind("Left", MC_Grip, MousePress, focus)
    mbind("Left", MC_Window, MousePress, focus)
    if click_raise:
        mbind("Left", MC_Titlebar, MousePress, raise_win)
        mbind("Left", MC_Handle, MousePress, raise_win)
        mbind("Left", MC_Grip, MousePress, raise_win)
        mbind("Left", MC_Window, MousePress, raise_win)

def setup_sloppy_focus(click_focus = 1, click_raise = 0):
    """Sets up for focusing windows when the mouse pointer enters them.
       Optionally, clicking on or in a window can focus it if your pointer
       ends up inside a window without focus. Also, optionally, clicking on or
       in a window can raise the window to the front of its stacking layer."""
    ebind(EventEnterWindow, focus)
    if click_focus:
        setup_click_focus(click_raise)

def setup_window_clicks():
    """Sets up the default bindings for various mouse buttons for various
       contexts.
       This includes:
        * Alt-left drag anywhere on a window will move it
        * Alt-right drag anywhere on a window will resize it
        * Left drag on a window's titlebar/handle will move it
        * Left drag on a window's handle grips will resize it
        * Alt-left press anywhere on a window's will raise it to the front of
          its stacking layer.
        * Left press on a window's titlebar/handle will raise it to the front
          of its stacking layer.
        * Alt-middle click anywhere on a window's will lower it to the bottom
          of its stacking layer.
        * Middle click on a window's titlebar/handle will lower it to the
          bottom of its stacking layer.
        * Double-left click on a window's titlebar will toggle shading it
    """
    mbind("A-Left", MC_Frame, MouseMotion, move)
    mbind("Left", MC_Titlebar, MouseMotion, move)
    mbind("Left", MC_Handle, MouseMotion, move)

    mbind("A-Right", MC_Frame, MouseMotion, resize)
    mbind("Left", MC_Grip, MouseMotion, resize)

    mbind("Left", MC_Titlebar, MousePress, raise_win)
    mbind("Left", MC_Handle, MousePress, raise_win)
    mbind("A-Left", MC_Frame, MousePress, raise_win)
    mbind("A-Middle", MC_Frame, MouseClick, lower_win)
    mbind("Middle", MC_Titlebar, MouseClick, lower_win)
    mbind("Middle", MC_Handle, MouseClick, lower_win)

    mbind("Left", MC_Titlebar, MouseDoubleClick, toggle_shade)

def setup_window_buttons():
    """Sets up the default behaviors for the buttons in the window titlebar."""
    mbind("Left", MC_StickyButton, MouseClick, toggle_all_desktops)
    mbind("Left", MC_CloseButton, MouseClick, close)

def setup_scroll():
    """Sets up the default behaviors for the mouse scroll wheel.
       This includes:
        * scrolling on a window titlebar will shade/unshade it
        * alt-scrolling anywhere will switch to the next/previous desktop
        * control-alt-scrolling on a window will send it to the next/previous
          desktop, and switch to the desktop with the window
    """
    mbind("Up", MC_Titlebar, MouseClick, shade)
    mbind("Down", MC_Titlebar, MouseClick, unshade)

    mbind("A-Up", MC_Frame, MouseClick, next_desktop)
    mbind("A-Up", MC_Root, MouseClick, next_desktop)
    mbind("A-Down", MC_Frame, MouseClick, prev_desktop)
    mbind("A-Down", MC_Root, MouseClick, prev_desktop)

    mbind("C-A-Up", MC_Frame, MouseClick, send_to_next_desktop)
    mbind("C-A-Down", MC_Frame, MouseClick, send_to_prev_desktop)

def setup_fallback_focus():
    """Sets up a focus fallback routine so that when no windows are focused,
       the last window to have focus on the desktop will be focused."""
    focus_stack = []
    def focused(data):
        #global focus_stack
        if data.client:
            window = data.client.window()
            # add to front the stack
            if window in focus_stack:
                focus_stack.remove(window)
            focus_stack.insert(0, window)
        else:
            # pass around focus
            desktop = openbox.screen(data.screen).desktop()
            l = len(focus_stack)
            i = 0
            while i < l:
                w = focus_stack[i]
                client = openbox.findClient(w)
                if not client: # window is gone, remove it
                    focus_stack.pop(i)
                    l = l - 1
                elif client.desktop() == desktop and \
                         client.normal() and client.focus():
                    break
                else:
                    i = i + 1

    ebind(EventFocus, focused)

    
############################################################################
### Window placement algorithms, choose one of these and ebind it to the ###
### EventPlaceWindow action.                                             ###
############################################################################

ob_rand = None
import random
def placewindows_random(data):
    if not data.client: return
    client_area = data.client.area()
    frame_size = data.client.frame.size()
    screen = display.screenInfo(data.screen)
    width = screen.width() - (client_area.width() +
                              frame_size.left + frame_size.right)
    height = screen.height() - (client_area.height() + 
                                frame_size.top + frame_size.bottom)
    global ob_rand
    if not ob_rand: ob_rand = random.Random()
    x = ob_rand.randrange(0, width-1)
    y = ob_rand.randrange(0, height-1)
    data.client.move(x, y)


print "Loaded builtins.py"
