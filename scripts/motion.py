############################################################################
###    Functions that provide callbacks for motion events to move and    ###
###    resize windows.                                                   ###
############################################################################

def move(data):
    """Moves the window interactively. This should only be used with
       MouseAction.Motion events. If 'Coords Popup for Moving' or 'Rubberband
       Mode for Moving' is enabled, then the end_move function needs to be
       bound as well."""
    _move(data)

def end_move(data):
    """Complete the interactive move of a window."""
    _end_move(data)

def resize(data):
    """Resizes the window interactively. This should only be used with
       MouseMotion events. If 'Coords Popup for Resizing' or 'Rubberband Mode
       for Resizing' is enabled, then the end_resize function needs to be
       bound as well."""
    _resize(data)

def end_resize(data):
    """Complete the interactive resize of a window."""
    _end_resize(data)

export_functions = move, end_move, resize, end_resize

#############################################################################

import config

config.add('motion',
           'edge_resistance',
           'Edge Resistance',
           "The amount of resistance to provide to moving a window past a " + \
           "screen boundary. Specify a value of 0 to disable edge resistance.",
           'integer',
           10,
           min = 0)
config.add('motion',
           'popup_in_window',
           'Coords Popup In Window',
           "When this is true, the coordinates popups will be placed " + \
           "relative to the window being moved/resized. When false, they " + \
           "will appear relative to the entire screen.",
           'boolean',
           0)
config.add('motion',
           'popup_centered',
           'Coords Popup Centered',
           "When this is true, the coordinates popups will be centered " + \
           "relative to the window or screen (see 'Coords Popup In " + \
           "Window'). When false, they will be placed based upon the " + \
           "'Coords Popup Position' options.",
           'boolean',
           1)
config.add('motion',
           'popup_coords_x',
           'Coords Popup Position - X',
           "When 'Coords Popup Centered' is false, this position will be " + \
           "used to place the coordinates popups. The popups will be " + \
           "placed relative to the window or the screen (see 'Coords " + \
           "Popup In Window'). A value of 0 would place it at the left " + \
           "edge, while a value of -1 would place it at the right edge. " + \
           "This value behaves similarly to those passed to the -geometry " + \
           "flag of many applications.",
           'integer',
           0)
config.add('motion',
           'popup_coords_y',
           'Coords Popup Position - Y',
           "When 'Coords Popup Centered' is false, this position will be " + \
           "used to place the coordinates popups. The popups will be " + \
           "placed relative to the window or the screen (see 'Coords Popup " +\
           "In Window'). A value of 0 would place it at the top edge, " + \
           "while a value of -1 would place it at the bottom edge. This " + \
           "value behaves similarly to those passed to the -geometry flag " + \
           "of many applications.",
           'integer',
           0)
config.add('motion',
           'move_popup',
           'Coords Popup for Moving',
           "Option to display a coordinates popup when moving windows.",
           'boolean',
           1)
config.add('motion',
           'move_rubberband',
           'Rubberband Mode for Moving',
           "NOT IMPLEMENTED (yet?)\n"+\
           "Display an outline while moving instead of moving the actual " + \
           "window, until the move is completed. Good for slower systems.",
           'boolean',
           0)
config.add('motion',
           'resize_popup',
           'Coords Popup for Resizing',
           "Option to display a coordinates popup when resizing windows.",
           'boolean',
           1)
config.add('motion',
           'resize_rubberband',
           'Rubberband Mode for Resizing',
           "NOT IMPLEMENTED (yet?)\n"+\
           "Display an outline while resizing instead of resizing the " + \
           "actual window, until the resize is completed. Good for slower " + \
           "systems.",
           'boolean',
           0)
config.add('motion',
           'resize_nearest',
           'Resize Nearest Corner',
           "When true, resizing will occur from the corner nearest where " + \
           "the mouse is. When false resizing will always occur from the " + \
           "bottom right corner.",
           'boolean',
           1)

###########################################################################
###      Internal stuff, should not be accessed outside the module.     ###
###########################################################################

import ob
import otk

_popwidget = 0

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

def _place_popup():
    if config.get('motion', 'popup_in_window'):
        # use the actual client's area, not the frame's
        area = _client.frame.area()
        size = _client.frame.size()
        area = otk.Rect(area.x() + size.left, area.y() + size.top,
                        area.width() - size.left - size.right,
                        area.height() - size.top - size.bottom)
    else:
        area = otk.Rect(otk.Point(0, 0), ob.openbox.screen(_screen).size())
    size = _popwidget.minSize()
    if config.get('motion', 'popup_centered'):
        x = area.position().x() + (area.size().width() - size.width()) / 2
        y = area.position().y() + (area.size().height() - size.height()) / 2
    else:
        x = config.get('motion', 'popup_coords_x')
        y = config.get('motion', 'popup_coords_y')
        if x < 0: x += area.width() - size.width() + 1
        if y < 0: y += area.width() - size.height() + 1
        x += area.position().x()
        y += area.position().y()
    _popwidget.moveresize(otk.Rect(x, y, size.width(), size.height()))

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

def _do_move(final):
    global _screen, _client, _cx, _cy, _dx, _dy

    # get destination x/y for the *frame*
    x = _cx + _dx + _client.frame.area().x() - _client.area().x()
    y = _cy + _dy + _client.frame.area().y() - _client.area().y()

    global _last_x, _last_y
    resist = config.get('motion', 'edge_resistance')
    if resist:
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
        if _last_x > x and x < l and x >= l - resist:
            x = l
        # right screen edge
        if _last_x < x and x > r and x <= r + resist:
            x = r
        # top screen edge
        if _last_y > y and y < t and y >= t - resist:
            y = t
        # right screen edge
        if _last_y < y and y > b and y <= b + resist:
            y = b

    global _inmove
    if not _inmove:
        _last_x = 0
        _last_y = 0
    else:
        _last_x = x
        _last_y = y

    if not final and config.get('motion', 'move_rubberband'):
        # XXX draw the outline ...
        pass
    else:
        _client.move(x, y, final)

    if config.get('motion', 'move_popup'):
        global _popwidget
        text = "X: " + str(x) + " Y: " + str(y)
        if not _popwidget:
            _popwidget = otk.Label(_screen, ob.openbox)
            _popwidget.setHighlighted(1)
        _popwidget.setText(text)
        _place_popup()
        _popwidget.show()

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
    _do_move(0)
    global _inmove
    if not _inmove:
        ob.kgrab(_screen, _motion_grab)
        _inmove = 1

def _end_move(data):
    global _inmove, _popwidget
    if _inmove:
        _do_move(1)
        _inmove = 0
    _popwidget = 0
    ob.kungrab()

def _do_resize(final):
    global _screen, _client, _cx, _cy, _cw, _ch, _px, _py, _dx, _dy

    dx = _dx
    dy = _dy
    
    # pick a corner to anchor
    if not (config.get('motion', 'resize_nearest') or
            _context == ob.MouseContext.Grip):
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

    if not final and config.get('motion', 'resize_rubberband'):
        # XXX draw the outline ...
        pass
    else:
        _client.resize(corner, w, h)

    if config.get('motion', 'resize_popup'):
        global _popwidget
        ls = _client.logicalSize()
        text = "W: " + str(ls.width()) + " H: " + str(ls.height())
        if not _popwidget:
            _popwidget = otk.Label(_screen, ob.openbox)
            _popwidget.setHighlighted(1)
        _popwidget.setText(text)
        _place_popup()
        _popwidget.show()

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
    _do_resize(0)
    global _inresize
    if not _inresize:
        ob.kgrab(_screen, _motion_grab)
        _inresize = 1

def _end_resize(data):
    global _inresize, _popwidget
    if _inresize:
        _do_resize(1)
        _inresize = 0
    _popwidget = 0
    ob.kungrab()
