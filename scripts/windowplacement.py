############################################################################
### Window placement algorithms, choose one of these and ebind it to the ###
### ob.EventAction.PlaceWindow event.                                    ###
###                                                                      ###
### Also see historyplacement.py for the history placement module which  ###
### provides an algorithm that can be used in place of, or alongside,    ###
### these.                                                               ###
############################################################################

import otk, ob, random

_rand = random.Random()

def random(data):
    """Place windows randomly around the screen."""
    if not data.client: return
    if data.client.positionRequested(): return
    client_area = data.client.frame.area()
    screen_area = ob.openbox.screen(data.screen).area(data.client.desktop())
    width = screen_area.width() - client_area.width()
    height = screen_area.height() - client_area.height()
    global _rand
    x = _rand.randrange(screen_area.x(), width-1)
    y = _rand.randrange(screen_area.y(), height-1)
    data.client.move(x, y)

_cascade_x = 0
_cascade_y = 0

def cascade(data):
    """Place windows in a cascading order from top-left to bottom-right."""
    if not data.client: return
    if data.client.positionRequested(): return
    client_area = data.client.frame.area()
    screen_area = ob.openbox.screen(data.screen).area(data.client.desktop())
    width = screen_area.width() - client_area.width()
    height = screen_area.height() - client_area.height()
    global _cascade_x, _cascade_y
    if _cascade_x < screen_area.x() or _cascade_y < screen_area.y() or \
           _cascade_x >= width or _cascade_y >= height:
        _cascade_x = screen_area.x()
        _cascade_y = screen_area.y()
    data.client.move(_cascade_x, _cascade_y)
    frame_size = data.client.frame.size()
    _cascade_x += frame_size.top
    _cascade_y += frame_size.top

export_functions = random, cascade

print "Loaded windowplacement.py"
