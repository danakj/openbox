###############################################################################
### Functions for setting up some default behaviors. This includes the      ###
### default bindings for clicking on various parts of a window, the         ###
### titlebar buttons, and bindings for the scroll wheel on your mouse.      ###
###############################################################################

import ob
import callbacks
import motion

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
    ob.mbind("A-Left", ob.MouseContext.Frame,
             ob.MouseAction.Motion, motion.move)
    ob.mbind("A-Left", ob.MouseContext.Frame,
             ob.MouseAction.Release, motion.end_move)
    ob.mbind("Left", ob.MouseContext.Titlebar,
             ob.MouseAction.Motion, motion.move)
    ob.mbind("Left", ob.MouseContext.Titlebar,
             ob.MouseAction.Release, motion.end_move)
    ob.mbind("Left", ob.MouseContext.Handle,
             ob.MouseAction.Motion, motion.move)
    ob.mbind("Left", ob.MouseContext.Handle,
             ob.MouseAction.Release, motion.end_move)

    ob.mbind("A-Right", ob.MouseContext.Frame,
             ob.MouseAction.Motion, motion.resize)
    ob.mbind("A-Right", ob.MouseContext.Frame,
             ob.MouseAction.Release, motion.end_resize)
    ob.mbind("Left", ob.MouseContext.Grip,
             ob.MouseAction.Motion, motion.resize)
    ob.mbind("Left", ob.MouseContext.Grip,
             ob.MouseAction.Release, motion.end_resize)

    ob.mbind("Left", ob.MouseContext.Titlebar,
             ob.MouseAction.Press, callbacks.raise_win)
    ob.mbind("Left", ob.MouseContext.Handle,
             ob.MouseAction.Press, callbacks.raise_win)
    ob.mbind("A-Left", ob.MouseContext.Frame,
             ob.MouseAction.Press, callbacks.raise_win)
    ob.mbind("A-Middle", ob.MouseContext.Frame,
             ob.MouseAction.Click, callbacks.lower_win)
    ob.mbind("Middle", ob.MouseContext.Titlebar,
             ob.MouseAction.Click, callbacks.lower_win)
    ob.mbind("Middle", ob.MouseContext.Handle,
             ob.MouseAction.Click, callbacks.lower_win)

    ob.mbind("Left", ob.MouseContext.Titlebar,
             ob.MouseAction.DoubleClick, callbacks.toggle_shade)

def setup_window_buttons():
    """Sets up the default behaviors for the buttons in the window titlebar."""
    ob.mbind("Left", ob.MouseContext.AllDesktopsButton,
             ob.MouseAction.Click, callbacks.toggle_all_desktops)
    ob.mbind("Left", ob.MouseContext.CloseButton,
             ob.MouseAction.Click, callbacks.close)
    ob.mbind("Left", ob.MouseContext.IconifyButton,
             ob.MouseAction.Click, callbacks.iconify)
    ob.mbind("Left", ob.MouseContext.MaximizeButton,
             ob.MouseAction.Click, callbacks.toggle_maximize)
    ob.mbind("Middle", ob.MouseContext.MaximizeButton,
             ob.MouseAction.Click, callbacks.toggle_maximize_vert)
    ob.mbind("Right", ob.MouseContext.MaximizeButton,
             ob.MouseAction.Click, callbacks.toggle_maximize_horz)

def setup_scroll():
    """Sets up the default behaviors for the mouse scroll wheel.
       This includes:
        * scrolling on a window titlebar will shade/unshade it
        * alt-scrolling anywhere will switch to the next/previous desktop
        * control-alt-scrolling on a window will send it to the next/previous
          desktop, and switch to the desktop with the window
    """
    ob.mbind("Up", ob.MouseContext.Titlebar,
             ob.MouseAction.Click, callbacks.shade)
    ob.mbind("Down", ob.MouseContext.Titlebar,
             ob.MouseAction.Click, callbacks.unshade)

    ob.mbind("A-Up", ob.MouseContext.Frame,
             ob.MouseAction.Click, callbacks.next_desktop)
    ob.mbind("A-Up", ob.MouseContext.Root,
             ob.MouseAction.Click, callbacks.next_desktop)
    ob.mbind("Up", ob.MouseContext.Root,
             ob.MouseAction.Click, callbacks.next_desktop)
    ob.mbind("A-Down", ob.MouseContext.Frame,
             ob.MouseAction.Click, callbacks.prev_desktop)
    ob.mbind("A-Down", ob.MouseContext.Root,
             ob.MouseAction.Click, callbacks.prev_desktop)
    ob.mbind("Down", ob.MouseContext.Root,
             ob.MouseAction.Click, callbacks.prev_desktop)

    ob.mbind("C-A-Up", ob.MouseContext.Frame,
             ob.MouseAction.Click, callbacks.send_to_next_desktop)
    ob.mbind("C-A-Down", ob.MouseContext.Frame,
             ob.MouseAction.Click, callbacks.send_to_prev_desktop)

print "Loaded behavior.py"
