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

def def_motion(action, win, type, modifiers, xroot, yroot, time):
	client = Openbox_findClient(openbox, win)
	if not client: return

	global posqueue
	dx = xroot - posqueue[0][1]
	dy = yroot - posqueue[0][2]

	area = posqueue[0][3] # A Rect
	if (type == Type_Titlebar) or (type == Type_Label):
		OBClient_move(client, Rect_x(area) + dx, Rect_y(area) + dy)
	elif type == Type_LeftGrip:
		OBClient_resize(client, OBClient_TopRight,
				Rect_width(area) - dx, Rect_height(area) + dy)
	elif type == Type_RightGrip:
		OBClient_resize(client, OBClient_TopLeft,
				Rect_width(area) + dx, Rect_height(area) + dy)

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
