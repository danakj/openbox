###########################################################################
###          Functions for helping out with your window focus.          ###
###########################################################################

import config, ob

export_functions = ()

config.add('focus',
           'avoid_skip_taskbar',
           'Avoid SkipTaskbar Windows',
           "Don't focus windows which have requested to not be displayed " + \
           "in taskbars. You will still be able to focus the windows, but " + \
           "not through cycling, and they won't be focused as a fallback " + \
           "if 'Focus Fallback' is enabled.",
           'boolean',
           1)

config.add('focus',
           'fallback',
           'Focus Fallback',
           "Send focus somewhere when nothing is left with the focus, if " + \
           "possible.",
           'boolean',
           1)
                    
# maintain a list of clients, stacked in focus order
_clients = []
_skip = 0

def _focusable(client, desktop):
    if not client.normal(): return 0
    if not (client.canFocus() or client.focusNotify()): return 0
    if client.iconic(): return 0
    if config.get('focus', 'avoid_skip_taskbar') and \
       client.skipTaskbar(): return 0

    desk = client.desktop()
    if not (desk == 0xffffffff or desk == desktop): return 0

    return 1

def _remove(client):
    """This function exists because Swig pointers don't define a __eq__
       function, so list.remove(ptr) does not work."""
    win = client.window()
    for i in range(len(_clients)):
        if _clients[i].window() == win:
            _clients.pop(i)
            return
    raise ValueError("_remove(x): x not in _clients list.")

def _focused(data):
    global _clients, _skip

    if _skip:
        _skip -= 1
        return

    if data.client:
        # move it to the top
        try:
            _remove(data.client)
        except ValueError: pass # happens if _focused comes before _newwindow
        _clients.insert(0, data.client)
    elif config.get('focus', 'fallback'):
        # pass around focus
        desktop = ob.openbox.screen(data.screen).desktop()
        for c in _clients:
            if _focusable(c, desktop):
                c.focus()
                break

def _newwindow(data):
    # make sure its not already in the list
    win = data.client.window()
    for i in range(len(_clients)):
        if _clients[i].window() == win:
            return
    _clients.append(data.client)
        
def _closewindow(data):
    _remove(data.client)

ob.ebind(ob.EventAction.NewWindow, _newwindow)
ob.ebind(ob.EventAction.CloseWindow, _closewindow)
ob.ebind(ob.EventAction.Focus, _focused)

print "Loaded focus.py"
