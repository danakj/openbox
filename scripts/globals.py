# openbox - pointer to the current Openbox instance
openbox = Openbox_instance()

# screen - list of all screens in the current openbox instance
screen = []
for i in range(Openbox_screenCount(openbox)):
    screen.append(Openbox_screen(openbox, i))

print "Loaded globals.py"
