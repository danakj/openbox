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
    type = OBClient_type(client)
    # these types of windows dont get focus from window enter events
    if data.action() == EventEnterWindow:
        if (type == OBClient_Type_Dock or \
            type == OBClient_Type_Desktop):
            return
    OBClient_focus(client)

def move(data):
    """Moves the window interactively. This should only be used with
       MouseMotion events"""
    client = Openbox_findClient(openbox, data.window())
    if not client: return

    type = OBClient_type(client)
    # these types of windows dont get moved
    if type == OBClient_Type_Dock or \
       type == OBClient_Type_Desktop:
        return

    dx = data.xroot() - data.pressx()
    dy = data.yroot() - data.pressy()
    OBClient_move(client, data.press_clientx() + dx, data.press_clienty() + dy)

def resize(data):
    """Resizes the window interactively. This should only be used with
       MouseMotion events"""
    client = Openbox_findClient(openbox, data.window())
    if not client: return

    type = OBClient_type(client)
    # these types of windows dont get resized
    if type == OBClient_Type_Dock or \
       type == OBClient_Type_Desktop:
        return

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
    Openbox_restart(openbox, "")

def toggle_shade(data):
    print "toggle_shade"

def raise_win(data):
    client = Openbox_findClient(openbox, data.window())
    if not client: return
    screen = Openbox_screen(openbox, OBClient_screen(client))
    OBScreen_restack(screen, 1, client)

def lower_win(data):
    client = Openbox_findClient(openbox, data.window())
    if not client: return
    screen = Openbox_screen(openbox, OBClient_screen(client))
    OBScreen_restack(screen, 0, client)

def toggle_shade(data):
    client = Openbox_findClient(openbox, data.window())
    if not client: return
    print "toggle_shade"
    OBClient_shade(client, not OBClient_shaded(client))

def shade(data):
    client = Openbox_findClient(openbox, data.window())
    if not client: return
    OBClient_shade(client, 1)

def unshade(data):
    client = Openbox_findClient(openbox, data.window())
    if not client: return
    OBClient_shade(client, 0)
    
#########################################
### Convenience functions for scripts ###
#########################################

def execute(bin, screen = 0):
    Openbox_execute(openbox, screen, bin)

print "Loaded builtins.py"
