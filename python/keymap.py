from input import Keyboard

def set(map):
    """Set your keymap"""
    global _map
    Keyboard.clearBinds()
    for key, func in map:
        Keyboard.bind(key, run)
    _map = map

def run(keydata, client):
    """Run a key press event through the keymap"""
    for key, func in _map:
        if (keydata.keychain == key):
            func(keydata, client)

_map = ()
