posqueue = [];

def def_motion_press(action, win, type, modifiers, button, xroot, yroot, time):
	client = Openbox_findClient(openbox, win)

	global posqueue
	newi = [button, xroot, yroot]
	if client:
		newi.append(new_Rect(OBClient_area(client)))
	posqueue.append(newi)

def def_motion_release(action, win, type, modifiers, button, xroot, yroot,
		       time):
	global posqueue
	for i in posqueue:
		if i[0] == button:
			client = Openbox_findClient(openbox, win)
			if client:
				delete_Rect(i[3])
			posqueue.remove(i)
			break

def def_do_motion(client, xroot, yroot):
	global posqueue
	dx = xroot - posqueue[0][1]
	dy = yroot - posqueue[0][2]
	area = posqueue[0][3] # A Rect
	OBClient_move(client, Rect_x(area) + dx, Rect_y(area) + dy)

def def_do_resize(client, xroot, yroot, archor_corner):
	global posqueue
	dx = xroot - posqueue[0][1]
	dy = yroot - posqueue[0][2]
	area = posqueue[0][3] # A Rect
	OBClient_resize(client, anchor_corner,
			Rect_width(area) - dx, Rect_height(area) + dy)

def def_motion(action, win, type, modifiers, xroot, yroot, time):
	client = Openbox_findClient(openbox, win)
	if not client: return

	if (type == Type_Titlebar) or (type == Type_Label):
		def_do_motion(client, xroot, yroot)
	elif type == Type_LeftGrip:
		def_do_resize(client, xroot, yroot, OBClient_TopRight)
	elif type == Type_RightGrip:
		def_do_resize(client, xroot, yroot, OBClient_TopLeft)

def def_enter(action, win, type, modifiers):
	client = Openbox_findClient(openbox, win)
	if not client: return
	if enter_focus != 0:
		OBClient_focus(client)

def def_leave(action, win, type, modifiers):
	client = Openbox_findClient(openbox, win)
	if not client: return
	if leave_unfocus != 0:
		OBClient_unfocus(client)


register(Action_EnterWindow, def_enter)
register(Action_LeaveWindow, def_leave)

register(Action_ButtonPress, def_motion_press)
register(Action_ButtonRelease, def_motion_release)
register(Action_MouseMotion, def_motion)

print "Loaded clientmotion.py"
