###########################################################################
### Functions that can be used as callbacks for mouse/keyboard bindings ###
###########################################################################

def close(data):
    """Closes the window on which the event occured"""
    client = Openbox_findClient(openbox, data.window())
    if client: OBClient_close(client)

def focus(data):
    """Focuses the window on which the event occured"""
    client = Openbox_findClient(openbox, data.window())
    if not client: return
    OBClient_focus(client)

def move(data):
    """Moves the window interactively. This should only be used with
       MouseMotion events"""
    client = Openbox_findClient(openbox, data.window())
    if not client: return

    dx = data.xroot() - data.pressx()
    dy = data.yroot() - data.pressy()
    OBClient_move(client, data.press_clientx() + dx, data.press_clienty() + dy)

def resize(data):
    """Resizes the window interactively. This should only be used with
       MouseMotion events"""
    client = Openbox_findClient(openbox, data.window())
    if not client: return

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

def execute(bin, screen = 0):
    Openbox_execute(openbox, screen, bin)
