# set up the mouse buttons
setup_sloppy_focus()
setup_window_clicks()
setup_window_buttons()
setup_scroll()
# set up focus fallback so im not left with nothing focused all the time
setup_fallback_focus()

# my window placement algorithm
ebind(EventPlaceWindow, placewindows_random)

# run xterm from root clicks
mbind("1", MC_Root, MouseClick, execute("xterm"))

kbind(["A-F4"], KC_All, close)

# desktop changing bindings
kbind(["C-1"], KC_All, lambda(d): change_desktop(d, 0))
kbind(["C-2"], KC_All, lambda(d): change_desktop(d, 1))
kbind(["C-3"], KC_All, lambda(d): change_desktop(d, 2))
kbind(["C-4"], KC_All, lambda(d): change_desktop(d, 3))
kbind(["C-A-Right"], KC_All, lambda(d): next_desktop(d))
kbind(["C-A-Left"], KC_All, lambda(d): prev_desktop(d))

kbind(["C-S-A-Right"], KC_All, lambda(d): send_to_next_desktop(d))
kbind(["C-S-A-Left"], KC_All, lambda(d): send_to_prev_desktop(d))

# focus new windows
def focusnew(data):
    if not data.client: return
    if data.client.normal():
        focus(data)

ebind(EventNewWindow, focusnew)

print "Loaded defaults.py"
