###############################################################################
###           Functions for setting up some default focus models.           ###
###############################################################################

import ob
import callbacks

def setup_click_focus(click_raise = 1):
    """Sets up for focusing windows by clicking on or in the window.
       Optionally, clicking on or in a window can raise the window to the
       front of its stacking layer."""
    ob.mbind("Left", ob.MouseContext.Titlebar,
             ob.MouseAction.Press, callbacks.focus)
    ob.mbind("Left", ob.MouseContext.Handle,
             ob.MouseAction.Press, callbacks.focus)
    ob.mbind("Left", ob.MouseContext.Grip,
             ob.MouseAction.Press, callbacks.focus)
    ob.mbind("Left", ob.MouseContext.Window,
             ob.MouseAction.Press, callbacks.focus)
    ob.mbind("Middle", ob.MouseContext.Window,
             ob.MouseAction.Press, callbacks.focus)
    ob.mbind("A-Left", ob.MouseContext.Frame,
             ob.MouseAction.Press, callbacks.focus)
    if click_raise:
        ob.mbind("Left", ob.MouseContext.Titlebar,
                 ob.MouseAction.Press, callbacks.raise_win)
        ob.mbind("Left", ob.MouseContext.Handle,
                 ob.MouseAction.Press, callbacks.raise_win)
        ob.mbind("Left", ob.MouseContext.Grip,
                 ob.MouseAction.Press, callbacks.raise_win)
        ob.mbind("Left", ob.MouseContext.Window,
                 ob.MouseAction.Press, callbacks.raise_win)    

def setup_sloppy_focus(click_focus = 1, click_raise = 0):
    """Sets up for focusing windows when the mouse pointer enters them.
       Optionally, clicking on or in a window can focus it if your pointer
       ends up inside a window without focus. Also, optionally, clicking on or
       in a window can raise the window to the front of its stacking layer."""
    ob.ebind(ob.EventAction.EnterWindow, callbacks.focus)
    if click_focus:
        ob.mbind("Left", ob.MouseContext.Titlebar,
                 ob.MouseAction.Press, callbacks.focus)
        ob.mbind("Left", ob.MouseContext.Handle,
                 ob.MouseAction.Press, callbacks.focus)
        ob.mbind("Left", ob.MouseContext.Grip,
                 ob.MouseAction.Press, callbacks.focus)
        ob.mbind("Left", ob.MouseContext.Window,
                 ob.MouseAction.Press, callbacks.focus)
        if click_raise:
            ob.mbind("Left", ob.MouseContext.Titlebar,
                     ob.MouseAction.Press, callbacks.raise_win)
            ob.mbind("Left", ob.MouseContext.Handle,
                     ob.MouseAction.Press, callbacks.raise_win)
            ob.mbind("Left", ob.MouseContext.Grip,
                     ob.MouseAction.Press, callbacks.raise_win)
            ob.mbind("Left", ob.MouseContext.Window,
                     ob.MouseAction.Press, callbacks.raise_win)    

print "Loaded focusmodel.py"
