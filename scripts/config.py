#############################################################################
### Options that can be defined on startup that affect the behavior of    ###
### openbox.                                                              ###
#############################################################################

# client_buttons - a list of the modifier(s) and buttons which are grabbed on
#                  client windows (for interactive move/resize, etc).
#  examples: "A-2", "C-A-2", "W-1"
client_buttons = ["A-1", "A-2", "A-3"]

# theme - the theme used to decorate everything.
theme = "/usr/local/share/openbox/styles/nyz"


#############################################################################
### Options that can be modified by the user to change the default hooks' ###
### behaviors.                                                            ###
#############################################################################

# resize_nearest - 1 to resize from the corner nearest where the mouse is, 0
#                  to resize always from the bottom right corner
resize_nearest = 1



print "Loaded config.py"
