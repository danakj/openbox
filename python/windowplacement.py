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
    xr = sw - cw - 1 # x range
    yr = sh - ch - 1 # y range
    if xr <= 0: x = 0
    else: x = Random().randrange(sx, xr)
    if yr <= 0: y = 0
    else: y = Random().randrange(sy, yr)
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
