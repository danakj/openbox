############################################################################
###    Functions that provide callbacks for motion events to move and    ###
###    resize windows.                                                   ###
############################################################################

#############################################################################
###   Options that can be modified to change the functions' behaviors.    ###
#############################################################################
EDGE_RESISTANCE = 10
"""The amount of resistance to provide to moving a window past a screen
   boundary. Specify a value of 0 to disable edge resistance."""
MOVE_POPUP = 1
"""Display a coordinates popup when moving windows."""
MOVE_RUBBERBAND = 0
"""NOT IMPLEMENTED (yet?)
   Display an outline while moving instead of moving the actual window,
   until the move is completed. Good for slower systems."""
RESIZE_POPUP = 1
"""Display a size popup when resizing windows."""
RESIZE_RUBBERBAND = 0
"""NOT IMPLEMENTED (yet?)
   Display an outline while resizing instead of resizing the actual
   window, until the resize is completed. Good for slower systems."""
RESIZE_NEAREST = 1
"""Non-zero to resize from the corner nearest where the mouse is, 0 to
   resize always from the bottom right corner."""
#############################################################################

def move(data):
    """Moves the window interactively. This should only be used with
       MouseAction.Motion events. If MOVE_POPUP or MOVE_RUBBERBAND is enabled,
       then the end_move function needs to be bound as well."""
    _move(data)

def end_move(data):
    """Complete the interactive move of a window."""
    _end_move(data)

def resize(data):
    """Resizes the window interactively. This should only be used with
       MouseMotion events. If RESIZE_POPUP or RESIZE_RUBBERBAND is enabled,
       then the end_resize function needs to be bound as well."""
    _resize(data)

def end_resize(data):
    """Complete the interactive resize of a window."""
    _end_resize(data)

###########################################################################
###########################################################################

###########################################################################
###      Internal stuff, should not be accessed outside the module.     ###
###########################################################################

import ob
import otk

_popwidget = 0
_poplabel = 0

# motion state
_inmove = 0
_inresize = 0

# last motion data
_cx = 0
_cy = 0
_cw = 0
_ch = 0
_px = 0
_py = 0
_dx = 0
_dy = 0
_client = 0
_screen = 0

_motion_mask = 0

def _motion_grab(data):
    global _motion_mask, _inmove, _inresize;

    # are all the modifiers this started with still pressed?
    if not _motion_mask & data.state:
        if _inmove:
            _end_move(data)
        elif _inresize:
            _end_resize(data)
        else:
            raise RuntimeError

_last_x = 0
_last_y = 0

def _do_move():
    global _screen, _client, _cx, _cy, _dx, _dy

    # get destination x/y for the *frame*
    x = _cx + _dx + _client.frame.area().x() - _client.area().x()
    y = _cy + _dy + _client.frame.area().y() - _client.area().y()

    global _last_x, _last_y
    if EDGE_RESISTANCE:
        fs = _client.frame.size()
        w = _client.area().width() + fs.left + fs.right
        h = _client.area().height() + fs.top + fs.bottom
        # use the area based on the struts
        area = ob.openbox.screen(_screen).area(_client.desktop())
        l = area.left()
        r = area.right() - w + 1
        t = area.top()
        b = area.bottom() - h + 1
        # left screen edge
        if _last_x > x and x < l and x >= l - EDGE_RESISTANCE:
            x = l
        # right screen edge
        if _last_x < x and x > r and x <= r + EDGE_RESISTANCE:
            x = r
        # top screen edge
        if _last_y > y and y < t and y >= t - EDGE_RESISTANCE:
            y = t
        # right screen edge
        if _last_y < y and y > b and y <= b + EDGE_RESISTANCE:
            y = b

    global _inmove
    if not _inmove:
        _last_x = 0
        _last_y = 0
    else:
        _last_x = x
        _last_y = y

    if MOVE_RUBBERBAND:
        # draw the outline ...
        f=0
    else:
        _client.move(x, y)

    if MOVE_POPUP:
        global _popwidget, _poplabel
        text = "X: " + str(x) + " Y: " + str(y)
        if not _popwidget:
            _popwidget = otk.Widget(_screen, ob.openbox, 
                                    otk.Widget.Horizontal, 0, 1)
            _poplabel = otk.Label(_popwidget)
            _poplabel.setHighlighted(1)
        _poplabel.setText(text)
        scsize = otk.display.screenInfo(_screen).size()
        size = _poplabel.minSize()
        _popwidget.moveresize(otk.Rect((scsize.width() - size.width()) / 2,
                                       (scsize.height() - size.height()) / 2,
                                       size.width(), size.height()))
        _popwidget.show(1)

def _move(data):
    if not data.client: return

    # not-normal windows dont get moved
    if not data.client.normal(): return

    global _screen, _client, _cx, _cy, _dx, _dy, _motion_mask
    _screen = data.screen
    _client = data.client
    _cx = data.press_clientx
    _cy = data.press_clienty
    _dx = data.xroot - data.pressx
    _dy = data.yroot - data.pressy
    _motion_mask = data.state
    _do_move()
    global _inmove
    if not _inmove:
        ob.kgrab(_screen, _motion_grab)
        _inmove = 1

def _end_move(data):
    global MOVE_RUBBERBAND
    global _inmove, _popwidget, _poplabel
    if _inmove:
        r = MOVE_RUBBERBAND
        MOVE_RUBBERBAND = 0
        _do_move()
        MOVE_RUBBERBAND = r
        _inmove = 0
    _poplabel = 0
    _popwidget = 0
    ob.kungrab()

def _do_resize():
    global _screen, _client, _cx, _cy, _cw, _ch, _px, _py, _dx, _dy

    dx = _dx
    dy = _dy
    
    # pick a corner to anchor
    if not (RESIZE_NEAREST or _context == ob.MouseContext.Grip):
        corner = ob.Client.TopLeft
    else:
        x = _px - _cx
        y = _py - _cy
        if y < _ch / 2:
            if x < _cw / 2:
                corner = ob.Client.BottomRight
                dx *= -1
            else:
                corner = ob.Client.BottomLeft
            dy *= -1
        else:
            if x < _cw / 2:
                corner = ob.Client.TopRight
                dx *= -1
            else:
                corner = ob.Client.TopLeft

    w = _cw + dx
    h = _ch + dy

    if RESIZE_RUBBERBAND:
        # draw the outline ...
        f=0
    else:
        _client.resize(corner, w, h)

    if RESIZE_POPUP:
        global _popwidget, _poplabel
        ls = _client.logicalSize()
        text = "W: " + str(ls.width()) + " H: " + str(ls.height())
        if not _popwidget:
            _popwidget = otk.Widget(_screen, ob.openbox, 
                                    otk.Widget.Horizontal, 0, 1)
            _poplabel = otk.Label(_popwidget)
            _poplabel.setHighlighted(1)
        _poplabel.setText(text)
        scsize = otk.display.screenInfo(_screen).size()
        size = _poplabel.minSize()
        _popwidget.moveresize(otk.Rect((scsize.width() - size.width()) / 2,
                                       (scsize.height() - size.height()) / 2,
                                       size.width(), size.height()))
        _popwidget.show(1)

def _resize(data):
    if not data.client: return

    # not-normal windows dont get resized
    if not data.client.normal(): return

    global _screen, _client, _cx, _cy, _cw, _ch, _px, _py, _dx, _dy
    global _motion_mask
    _screen = data.screen
    _client = data.client
    _cx = data.press_clientx
    _cy = data.press_clienty
    _cw = data.press_clientwidth
    _ch = data.press_clientheight
    _px = data.pressx
    _py = data.pressy
    _dx = data.xroot - _px
    _dy = data.yroot - _py
    _motion_mask = data.state
    _do_resize()
    global _inresize
    if not _inresize:
        ob.kgrab(_screen, _motion_grab)
        _inresize = 1

def _end_resize(data):
    global RESIZE_RUBBERBAND, _inresize
    global _popwidget, _poplabel
    if _inresize:
        r = RESIZE_RUBBERBAND
        RESIZE_RUBBERBAND = 0
        _do_resize()
        RESIZE_RUBBERBAND = r
        _inresize = 0
    _poplabel = 0
    _popwidget = 0
    ob.kungrab()
