#############################################################################
### Variables defined for other scripts to use.                           ###
#############################################################################

# openbox - pointer to the current Openbox instance
openbox = Openbox_instance()

# screens - list of all screens in the current openbox instance
screens = []
for i in range(Openbox_screenCount(openbox)):
    screens.append(Openbox_screen(openbox, i))


print "Loaded globals.py"
