############################################################################
### Window placement algorithms, choose one of these and ebind it to the ###
### ob.EventAction.PlaceWindow event.                                    ###
###                                                                      ###
### Also see historyplacement.py for the history placement module which  ###
### provides an algorithm that can be used in place of, or alongside,    ###
### these.                                                               ###
############################################################################

import ob
from random import Random

def random(client):
    """Place windows randomly around the screen."""
    if ob.Openbox.state() == ob.State.Starting: return
    #if data.client.positionRequested(): return
    cx, cy, cw, ch = client.area()
    sx, sy, sw, sh = ob.Openbox.screenArea(client.desktop())
    global _rand
    x = Random().randrange(sx, sw - cw - 1)
    y = Random().randrange(sy, sh - ch - 1)
    client.setArea((x, y, cw, ch))

def cascade(client):
    """Place windows in a cascading order from top-left to bottom-right."""
    if ob.Openbox.state() == ob.State.Starting: return
    #if data.client.positionRequested(): return
    cx, cy, cw, ch = client.area()
    sx, sy, sw, sh = ob.Openbox.screenArea(client.desktop())
    width = sw - cw
    height = sh - ch
    global _cascade_x, _cascade_y
    if _cascade_x < sx or _cascade_y < sy or \
           _cascade_x >= width or _cascade_y >= height:
        _cascade_x = sx
        _cascade_y = sy
    client.setArea((_cascade_x, _cascade_y, cw, ch))
    frame_size = client.frameSize()
    _cascade_x += frame_size[1]
    _cascade_y += frame_size[1]

_cascade_x = 0
_cascade_y = 0

export_functions = random, cascade

print "Loaded windowplacement.py"
