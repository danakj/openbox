###########################################################################
###          Functions for helping out with your window focus.          ###
###########################################################################

import config, ob, hooks

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

def focusable(client, desktop):
    if not client.normal(): return False
    if not (client.canFocus() or client.focusNotify()): return False
    if client.iconic(): return False
    if config.get('focus', 'avoid_skip_taskbar') and \
       client.skipTaskbar(): return False

    desk = client.desktop()
    if not (desk == 0xffffffff or desk == desktop): return False

    return True

def _focused(client):
    global _clients, _skip

    if _skip:
        _skip -= 1
        return

    if client:
        # move it to the top
        _clients.remove(client)
        _clients.insert(0, client)
    elif config.get('focus', 'fallback'):
        # pass around focus
        desktop = ob.Openbox.desktop()
        for c in _clients:
            if focusable(c, desktop):
                c.focus()
                break

hooks.managed.append(lambda c: _clients.append(c))
hooks.closed.append(lambda c: _clients.remove(c))
hooks.focused.append(_focused)

print "Loaded focus.py"
