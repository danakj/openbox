############################################################################
### Window placement algorithms, choose one of these and ebind it to the ###
### ob.EventAction.PlaceWindow event.                                    ###
############################################################################

import otk
import ob
import random

_rand = random.Random()

def random(data):
    if not data.client: return
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

print "Loaded windowplacement.py"
