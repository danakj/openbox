##############################################################################
### The history window placement algorithm. ebind historyplacement.place   ###
### to the ob.EventAction.PlaceWindow event to use it.                     ###
##############################################################################

import windowplacement # fallback routines

##############################################################################
### Options for the historyplacement module:                               ###
###                                                                        ###
# fallback - The window placement algorithm that will be used when history ###
###          placement does not have a place for the window.               ###
fallback = windowplacement.random                                          ###
###                                                                        ###
# filename - The name of the file where history data will be stored. The   ###
###          number of the screen is appended onto this filename.          ###
filename = 'historydb'                                                     ###
###                                                                        ###
##############################################################################

import otk
import ob
import os
import string

_data = []

class _state:
    def __init__(self, appname, appclass, role, x, y):
        self.appname = appname
        self.appclass = appclass
        self.role = role
        self.x = x
        self.y = y
    def __eq__(self, other):
        if self.appname == other.appname and \
           self.appclass == other.appclass and \
           self.role == other.role:
            return 1
        return 0

def _load(data):
    file = open(os.environ['HOME']+'/.openbox/'+filename+"."+str(data.screen),
                'r')
    if file:
        print "loading: "
        # read data
        for line in file.readlines():
            line = line[:-1] # drop the '\n'
            try:
                print string.split(line, '\0')
                print line.count('\0')
                s = string.split(line, '\0')
                state = _state(s[0], s[1], s[2],
                               string.atoi(s[3]), string.atoi(s[4]))

                while len(_data)-1 < data.screen:
                    _data.append([])
                _data[data.screen].append(state)
                
                print "  "+s[0]+" "+s[1]+" "+s[2]
                print "     " + str(s[3]) + "," + str(s[4])
            except ValueError:
                print "ValueError"
                pass
            except IndexError:
                print "IndexError"
                pass
        print "DONE loading."
        file.close()

def _save(data):
    file = open(os.environ['HOME']+'/.openbox/'+filename+"."+str(data.screen),
                'w')
    if file:
        print "saving: "
        while len(_data)-1 < data.screen:
            _data.append([])
        for i in _data[data.screen]:
            file.write(i.appname + '\0' +
                       i.appclass + '\0' +
                       i.role + '\0' +
                       str(i.x) + '\0' +
                       str(i.y) + '\n')
            print "  "+i.appname+" "+i.appclass+" "+i.role
            print "     " + str(i.x) + "," + str(i.y)
        print "DONE saving."
        file.close()

def place(data):
    print "placing"
    if data.client:
        state = _state(data.client.appName(), data.client.appClass(),
                       data.client.role(), 0, 0)
        while len(_data)-1 < data.screen:
            _data.append([])
        print "looking for :"
        print "  " + state.appname
        print "  " + state.appclass
        print "  " + state.role
        try:
            i = _data[data.screen].index(state)
            print "got it"
            coords = _data[data.screen][i]
            print "Found in history ("+str(coords.x)+","+str(coords.y)+")"
            data.client.move(coords.x, coords.y)
        except ValueError:
            print "No match in history"
            fallback(data)

def _save_window(data):
    print "saving"
    if data.client:
        area = data.client.area()
        state = _state(data.client.appName(), data.client.appClass(),
                       data.client.role(), area.x(), area.y())
        while len(_data)-1 < data.screen:
            _data.append([])
        print "looking for :"
        print "  " + state.appname
        print "  " + state.appclass
        print "  " + state.role
        try:
            i = _data[data.screen].index(state)
            print "replacing"
            _data[data.screen][i] = state # replace it
        except ValueError:
            print "appending"
            _data[data.screen].append(state)

ob.ebind(ob.EventAction.CloseWindow, _save_window)
ob.ebind(ob.EventAction.Startup, _load)
ob.ebind(ob.EventAction.Shutdown, _save)

print "Loaded historyplacement.py"
