import config, hooks, ob
from input import Pointer

config.add('motion',
           'edge_resistance',
           'Edge Resistance',
           "The amount of resistance to provide to moving a window past a " + \
           "screen boundary. Specify a value of 0 to disable edge resistance.",
           'integer',
           10,
           min = 0)

def move(ptrdata, client):
    def mymove(ptrdata, client):
        global _moving, _last_pos
        if ptrdata.action == Pointer.Action_Release:
            _moveclient.setArea(_moveclient.area(), True) # finalize the move
            _moving = False
            Pointer.ungrab()
        elif ptrdata.action == Pointer.Action_Motion:
            pos = ptrdata.pos

            x = _pcarea[0] + pos[0] - _presspos[0]
            y = _pcarea[1] + pos[1] - _presspos[1]

            resist = config.get('motion', 'edge_resistance')
            if resist:
                ca = _moveclient.area()
                w, h = ca[2], ca[3]
                # use the area based on the struts
                sa = ob.Openbox.screenArea(_moveclient.desktop())
                l, t = sa[0], sa[1]
                r = l+ sa[2] - w
                b = t+ sa[3] - h
                # left screen edge
                if _last_pos[0] >= pos[0] and x < l and x >= l - resist:
                    x = l
                # right screen edge
                if _last_pos[0] <= pos[0] and x > r and x <= r + resist:
                    x = r
                # top screen edge
                if _last_pos[1] >= pos[1] and y < t and y >= t - resist:
                    y = t
                # right screen edge
                if _last_pos[1] <= pos[1] and y > b and y <= b + resist:
                    y = b

            _moveclient.setArea((x, y, _pcarea[2], _pcarea[3]), False)
            _last_pos = pos

    global _last_pos, _moving, _pcarea, _presspos, _moveclient
    if not _moving:
        _moving = True
        _pcarea = ptrdata.pressclientarea
        _presspos = ptrdata.presspos
        _last_pos = _presspos
        _moveclient = client
        Pointer.grab(mymove)
        mymove(ptrdata, client)

def resize(ptrdata, client):
    x, y = ptrdata.pos
    px, py = ptrdata.presspos
    cx, cy, cw, ch = ptrdata.pressclientarea
    dx = x - px
    dy = y - py
    if px < cx + cw / 2: # left side
        dx *= -1
        cx -= dx
    if py < cy + ch / 2: # top side
        dy *= -1
        cy -= dy
    cw += dx
    ch += dy
    client.setArea((cx, cy, cw, ch))

_moving = False
_moveclient = 0
_last_pos = ()
_pcarea = ()
_presspos = ()
