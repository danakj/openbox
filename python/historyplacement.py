##############################################################################
### The history window placement algorithm. ebind historyplacement.place   ###
### to the ob.EventAction.PlaceWindow event to use it.                     ###
##############################################################################

import windowplacement, config, hooks

def place(data):
    """Place a window usingthe history placement algorithm."""
    _place(data)

export_functions = place

##############################################################################

config.add('historyplacement',
           'ignore_requested_positions',
           'Ignore Requested Positions',
           "When true, the placement algorithm will attempt to place " + \
           "windows even when they request a position (like XMMS can)." + \
           "Note this only applies to 'normal' windows, not to special " + \
           "cases like desktops and docks.",
           'boolean',
           0)
config.add('historyplacement',
           'dont_duplicate',
           "Don't Diplicate",
           "When true, if 2 copies of the same match in history are to be " + \
           "placed before one of them is closed (so it would be placed " + \
           "over-top of the last one), this will cause the second window to "+\
           "not be placed via history, and the 'Fallback Algorithm' will be "+\
           "used instead.",
           'boolean',
           1)
config.add('historyplacement',
           'filename',
           'History Database Filename',
           "The name of the file where history data will be stored. The " + \
           "number of the screen is appended onto this name. The file will " +\
           "be placed in ~/.openbox/.",
           'string',
           'historydb')
config.add('historyplacement',
           'fallback',
           'Fallback Algorithm',
           "The window placement algorithm that will be used when history " + \
           "placement does not have a place for the window.",
           'enum',
           windowplacement.random,
           options = windowplacement.export_functions)

###########################################################################

###########################################################################
###      Internal stuff, should not be accessed outside the module.     ###
###########################################################################

import ob, os, string

_data = []

class _State:
    def __init__(self, resname, resclass, role, x, y):
        self.resname = resname
        self.resclass = resclass
        self.role = role
        self.x = x
        self.y = y
        self.placed = 0
    def __eq__(self, other):
        if self.resname == other.resname and \
           self.resclass == other.resclass and \
           self.role == other.role:
            return 1
        return 0

def _load():
    global _data
    try:
        file = open(os.environ['HOME'] + '/.openbox/' + \
                    config.get('historyplacement', 'filename') + \
                    "." + str(ob.Openbox.screenNumber()), 'r')
        # read data
        for line in file.readlines():
            line = line[:-1] # drop the '\n'
            try:
                s = string.split(line, '\0')
                state = _State(s[0], s[1], s[2],
                               int(s[3]), int(s[4]))

                _data.append(state)
                
            except ValueError: pass
            except IndexError: pass
        file.close()
    except IOError: pass

def _save():
    file = open(os.environ['HOME'] + '/.openbox/'+ \
                config.get('historyplacement', 'filename') + \
                "." + str(ob.Openbox.screenNumber()), 'w')
    if file:
        for i in _data:
            file.write(i.resname + '\0' +
                       i.resclass + '\0' +
                       i.role + '\0' +
                       str(i.x) + '\0' +
                       str(i.y) + '\n')
        file.close()

def _create_state(client):
    area = client.area()
    return _State(client.resName(), client.resClass(),
                  client.role(), area[0], area[1])

def _place(client):
    state = _create_state(client)
    try:
        print "looking for : " + state.resname +  " : " + \
              state.resclass + " : " + state.role
        try:
            i = _data.index(state)
        except ValueError:
            print "No match in history"
        else:
            coords = _data[i] # get the equal element
            print "Found in history ("+str(coords.x)+","+\
                  str(coords.y)+")"
            if not (config.get('historyplacement', 'dont_duplicate') \
                    and coords.placed):
                coords.placed = 1
                if ob.Openbox.state() != ob.State.Starting:
#    if not (config.get('historyplacement', 'ignore_requested_positions') \
#            and data.client.normal()):
#        if data.client.positionRequested(): return
                    ca = client.area()
                    client.setArea((coords.x, coords.y, ca[2], ca[3]))
                return
            else:
                print "Already placed another window there"
    except TypeError:
        pass
    fallback = config.get('historyplacement', 'fallback')
    if fallback: fallback(client)

def _save_window(client):
    global _data
    state = _create_state(client)
    print "looking for : " + state.resname +  " : " + state.resclass + \
          " : " + state.role

    try:
        print "replacing"
        i = _data.index(state)
        _data[i] = state # replace it
    except ValueError:
        print "appending"
        _data.append(state)

hooks.startup.append(_load)
hooks.shutdown.append(_save)
hooks.closed.append(_save_window)

print "Loaded historyplacement.py"
