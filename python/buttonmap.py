from input import Pointer

def set(map):
    """Set your buttonmap. Functions in the button map should all take a single
    argument, a Client object, except for functions for Action_Motion events,
    who should take 2 arguments, a PointerData object and a Client object."""
    global _press_map, _release_map, _click_map, _doubleclick_map, _motion_map
    Pointer.clearBinds()
    _press_map = []
    _release_map = []
    _click_map = []
    _doubleclick_map = []
    _motion_map = []
    for button, context, action, func in map:
        if (action == Pointer.Action_Press):
            _press_map.append((button, context, func))
            mapfunc = press_run
        if (action == Pointer.Action_Release):
            _release_map.append((button, context, func))
            mapfunc = release_run
        if (action == Pointer.Action_Click):
            _click_map.append((button, context, func))
            mapfunc = click_run
        if (action == Pointer.Action_DoubleClick):
            _doubleclick_map.append((button, context, func))
            mapfunc = doubleclick_run
        if (action == Pointer.Action_Motion):
            _motion_map.append((button, context, func))
            mapfunc = motion_run
        Pointer.bind(button, context, action, mapfunc)

def press_run(ptrdata, client):
    """Run a button press event through the buttonmap"""
    button = ptrdata.button
    context = ptrdata.context
    for but, cont, func in _press_map:
        if (but == button and cont == context):
            func(client)

def release_run(ptrdata, client):
    """Run a button release event through the buttonmap"""
    button = ptrdata.button
    context = ptrdata.context
    for but, cont, func in _release_map:
        if (but == button and cont == context):
            func(client)

def click_run(ptrdata, client):
    """Run a button click event through the buttonmap"""
    button = ptrdata.button
    context = ptrdata.context
    for but, cont, func in _click_map:
        if (but == button and cont == context):
            func(client)

def doubleclick_run(ptrdata, client):
    """Run a button doubleclick event through the buttonmap"""
    button = ptrdata.button
    context = ptrdata.context
    for but, cont, func in _doubleclick_map:
        if (but == button and cont == context):
            func(client)

def motion_run(ptrdata, client):
    """Run a pointer motion event through the buttonmap"""
    button = ptrdata.button
    context = ptrdata.context
    for but, cont, func in _motion_map:
        if (but == button and cont == context):
            func(ptrdata, client)

_press_map = ()
_release_map = ()
_click_map = ()
_doubleclick_map = ()
_motion_map = ()
