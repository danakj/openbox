#############################################################################
### Options that can be defined on startup that affect the behavior of    ###
### openbox.                                                              ###
#############################################################################

# client_buttons - a list of the modifier(s) and buttons which are grabbed on
#                  client windows (for interactive move/resize, etc)
#  examples: "A-2", "C-A-2", "W-1"
client_buttons = ["A-1", "A-2", "A-3"]



#############################################################################
### Options that can be modified by the user to change the default hooks' ###
### behaviors.                                                            ###
#############################################################################

# click_focus - true if clicking in a client will cause it to focus in the
#               default hook functions
click_focus = 0
# click_raise - true if clicking in a client will cause it to raise to the
#               top of its stacking layer
click_raise = 0
# enter_focus - true if entering a client window will cause it to focus in the
#               default hook functions
enter_focus = 1
# leave_unfocus - true if leaving a client window will cause it to unfocus in
#                 the default hook functions
leave_unfocus = 1



#############################################################################
### Variables defined for other scripts to use.                           ###
#############################################################################

# openbox - pointer to the current Openbox instance
openbox = Openbox_instance()

# screen - list of all screens in the current openbox instance
screen = []
for i in range(Openbox_screenCount(openbox)):
    screen.append(Openbox_screen(openbox, i))



print "Loaded globals.py"
