#############################################################################
### Options that can be defined on startup that affect the behavior of    ###
### openbox.                                                              ###
#############################################################################

# client_buttons - a list of the modifier(s) and buttons which are grabbed on
#                  client windows (for interactive move/resize, etc).
#  examples: "A-2", "C-A-2", "W-1"
client_buttons = ["A-1", "A-2", "A-3"]

# theme - the theme used to decorate everything.
theme = "/usr/local/share/openbox/styles/fieron2"

# titlebar_layout - the layout of the buttons/label on client titlebars, can be
#                   made up of the following:
#                   I - iconify button, L - text label, M - maximize button,
#                   D - all-desktops button, C - close button
#                   If no 'L' is included in the string, one will be added to
#                   the end by Openbox.
titlebar_layout = "DILMC"

# double_click_delay - the number of milliseconds in which 2 clicks are
#                      perceived as a double-click.
double_click_delay = 300

# drag_threshold - the amount of pixels that you have to drag the mouse before
#                  motion events will start occuring.
drag_threshold = 3

# desktop_names - the name of each desktop
desktop_names = ["one", "two", "three", "four", "five", "six", "seven", \
                 "eight", "nine", "ten", "eleven", "twelve"]

# number_of_desktops - the number of desktops/workspaces which can be scrolled
#                      between.
number_of_desktops = 4

#############################################################################
### Options that can be modified by the user to change the default hooks' ###
### behaviors.                                                            ###
#############################################################################

# resize_nearest - 1 to resize from the corner nearest where the mouse is, 0
#                  to resize always from the bottom right corner.
resize_nearest = 1



print "Loaded config.py"
