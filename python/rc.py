import hooks, ob, keymap, buttonmap, os, sys, input, motion, historyplacement
import stackedcycle
from input import Pointer

hooks.managed.append(historyplacement.place)

_grab = 0
def printshit(keydata, client):
    global _grab
    print "shit"
    _grab = not _grab
    print _grab
    def gfunc(data, client=None): pass
    if _grab:
        input.Keyboard.grab(gfunc)
        input.Pointer.grab(gfunc)
    else:
        input.Keyboard.ungrab()
        input.Pointer.ungrab()

def myexec(prog):
    print "execing: ", prog
    if (os.fork() == 0):
        try:
            os.setsid()
            os.execl("/bin/sh", "/bin/sh", "-c", prog)
        except:
            print str(sys.exc_info()[0]) + ": " + str(sys.exc_info()[1])
            try:
                print "failed to execute '" + prog + "'"
            except:
                print str(sys.exc_info()[0]) + ": " + str(sys.exc_info()[1])
            os._exit(0)

def myactivate(c):
    if ob.Openbox.showingDesktop():
        ob.Openbox.setShowingDesktop(False)
    if c.iconic():
        c.setIconic(False)
    elif not c.visible():
        # if its not visible for other reasons, then don't mess with it
        return
    if c.shaded():
        c.setShaded(False)
    c.focus()
    c.raiseWindow()
                                                      
hooks.requestactivate.append(myactivate)

def myfocus(c):
    if c and c.normal(): c.focus()

#hooks.showwindow.append(myfocus)
hooks.pointerenter.append(myfocus)

hooks.visible.append(myfocus)

mykmap=((("C-a", "d"), printshit),
        (("C-Tab",), stackedcycle.next),
        (("C-S-Tab",), stackedcycle.previous),
        (("C-space",), lambda k, c: myexec("xterm")))
keymap.set(mykmap)

def mytogglesticky(client):
    if client.desktop() == 0xffffffff: d = ob.Openbox.desktop()
    else: d = 0xffffffff
    client.setDesktop(d)

mybmap=(("1", "maximize", Pointer.Action_Click,
         lambda c: c.setMaximized(not c.maximized())),
        ("2", "maximize", Pointer.Action_Click,
         lambda c: c.setMaximizedVert(not c.maximizedVert())),
        ("3", "maximize", Pointer.Action_Click,
         lambda c: c.setMaximizedHorz(not c.maximizedHorz())),
        ("1", "alldesktops", Pointer.Action_Click, mytogglesticky),
        ("1", "iconify", Pointer.Action_Click,
         lambda c: c.setIconic(True)),
        ("1", "icon", Pointer.Action_DoubleClick, ob.Client.close),
        ("1", "close", Pointer.Action_Click, ob.Client.close),
        ("1", "titlebar", Pointer.Action_Motion, motion.move),
        ("1", "handle", Pointer.Action_Motion, motion.move),
        ("Mod1-1", "frame", Pointer.Action_Click, ob.Client.raiseWindow),
        ("Mod1-1", "frame", Pointer.Action_Motion, motion.move),
        ("1", "titlebar", Pointer.Action_Press, ob.Client.raiseWindow),
        ("1", "handle", Pointer.Action_Press, ob.Client.raiseWindow),
        ("1", "client", Pointer.Action_Press, ob.Client.raiseWindow),
        ("2", "titlebar", Pointer.Action_Press, ob.Client.lowerWindow),
        ("2", "handle", Pointer.Action_Press, ob.Client.lowerWindow),
        ("Mod1-3", "frame", Pointer.Action_Click, ob.Client.lowerWindow),
        ("Mod1-3", "frame", Pointer.Action_Motion, motion.resize),
        ("1", "blcorner", Pointer.Action_Motion, motion.resize),
        ("1", "brcorner", Pointer.Action_Motion, motion.resize),
        ("1", "titlebar", Pointer.Action_Press, ob.Client.focus),
        ("1", "handle", Pointer.Action_Press, ob.Client.focus),
        ("1", "client", Pointer.Action_Press, ob.Client.focus),
        ("1", "titlebar", Pointer.Action_DoubleClick,
         lambda c: c.setShaded(not c.shaded())),
        ("4", "titlebar", Pointer.Action_Click,
         lambda c: c.setShaded(True)),
        ("5", "titlebar", Pointer.Action_Click,
         lambda c: c.setShaded(False)),
        ("4", "root", Pointer.Action_Click,
         lambda c: ob.Openbox.setNextDesktop()),
        ("5", "root", Pointer.Action_Click,
         lambda c: ob.Openbox.setPreviousDesktop()),
        ("Mod1-4", "frame", Pointer.Action_Click,
         lambda c: ob.Openbox.setNextDesktop()),
        ("Mod1-5", "frame", Pointer.Action_Click,
         lambda c: ob.Openbox.setPreviousDesktop()),
        ("Mod1-4", "root", Pointer.Action_Click,
         lambda c: ob.Openbox.setNextDesktop()),
        ("Mod1-5", "root", Pointer.Action_Click,
         lambda c: ob.Openbox.setPreviousDesktop()))
buttonmap.set(mybmap)
