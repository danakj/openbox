#############################################################################
### Options that can be changed to adjust the behavior of Openbox.        ###
#############################################################################

THEME = "/usr/local/share/openbox/styles/fieron2"
"""The theme used to decorate everything."""

TITLEBAR_LAYOUT = "DILMC"
"""The layout of the buttons/label on client titlebars, can be made up of the
following:
    I - iconify button
    L - text label
    M - maximize button,
    D - all-desktops button
    C - close button
If no 'L' is included in the string, one will be added to the end by
Openbox."""

DOUBLE_CLICK_DELAY = 300
"""The number of milliseconds in which 2 clicks are perceived as a
double-click."""

DRAG_THRESHOLD = 3
"""The amount of pixels that you have to drag the mouse before motion events
will start occuring."""

DESKTOP_NAMES = ["one", "two", "three", "four", "five", "six", "seven", \
                 "eight", "nine", "ten", "eleven", "twelve"]
"""The name of each desktop."""

NUMBER_OF_DESKTOPS = 4
"""The number of desktops/workspaces which can be scrolled between."""

#############################################################################

print "Loaded config.py"
