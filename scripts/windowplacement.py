############################################################################
### Window placement algorithms, choose one of these and ebind it to the ###
### ob.EventAction.PlaceWindow event.                                    ###
###                                                                      ###
### Also see historyplacement.py for the history placement module which  ###
### provides an algorithm that can be used in place of, or alongside,    ###
### these.                                                               ###
############################################################################

##############################################################################
### Options for the windowplacement module:                                ###
###                                                                        ###
# ignore_requested_positions - When true, the placement algorithm will     ###
###                            attempt to place windows even when they     ###
###                            request a position (like XMMS).             ###
###                            Note this only applies to normal windows,   ###
###                            not to special cases like desktops and      ###
###                            docks.                                      ###
ignore_requested_positions = 0                                             ###
###                                                                        ###
##############################################################################

import otk
import ob
import random

_rand = random.Random()

def random(data):
    """Place windows randomly around the screen."""
    if not data.client: return
    if not (ignore_requested_positions and data.client.normal()):
        if data.client.positionRequested(): return
    client_area = data.client.area()
    frame_size = data.client.frame.size()
    screen_area = ob.openbox.screen(data.screen).area()
    width = screen_area.width() - (client_area.width() +
                                   frame_size.left + frame_size.right)
    height = screen_area.height() - (client_area.height() + 
                                     frame_size.top + frame_size.bottom)
    global _rand
    x = _rand.randrange(screen_area.x(), width-1)
    y = _rand.randrange(screen_area.y(), height-1)
    data.client.move(x, y)

_cascade_x = 0
_cascade_y = 0

def cascade(data):
    """Place windows in a cascading order from top-left to bottom-right."""
    if not data.client: return
    if not (ignore_requested_positions and data.client.normal()):
        if data.client.positionRequested(): return
    client_area = data.client.area()
    frame_size = data.client.frame.size()
    screen_area = ob.openbox.screen(data.screen).area()
    width = screen_area.width() - (client_area.width() +
                                   frame_size.left + frame_size.right)
    height = screen_area.height() - (client_area.height() + 
                                     frame_size.top + frame_size.bottom)
    global _cascade_x, _cascade_y
    if _cascade_x < screen_area.x() or _cascade_y < screen_area.y() or \
           _cascade_x >= width or _cascade_y >= height:
        _cascade_x = screen_area.x()
        _cascade_y = screen_area.y()
    data.client.move(_cascade_x, _cascade_y)
    _cascade_x += frame_size.top
    _cascade_y += frame_size.top

print "Loaded windowplacement.py"
