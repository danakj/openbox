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

# click_focus - '1' if clicking in a client will cause it to focus in the
#               default hook functions; else '0'.
click_focus = 0
# click_raise - '1' if clicking in a client will cause it to raise to the
#               top of its stacking layer; else '0'.
click_raise = 0
# enter_focus - '1' if entering a client window will cause it to focus in the
#               default hook functions; else '0'.
enter_focus = 1
# leave_unfocus - '1' if leaving a client window will cause it to unfocus in
#                 the default hook functions; else '0'.
leave_unfocus = 1


print "Loaded config.py"
