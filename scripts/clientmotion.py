posqueue = [];

def def_motion_press(data):
	client = Openbox_findClient(openbox, data.window())

	global posqueue
	newi = [data.button(), data.xroot(), data.yroot()]
	if client:
		newi.append(new_Rect(OBClient_area(client)))
	posqueue.append(newi)

def def_motion_release(data):
	global posqueue
	button = data.button()
	for i in posqueue:
		if i[0] == button:
			client = Openbox_findClient(openbox, data.window())
			if client:
				delete_Rect(i[3])
			posqueue.remove(i)
			break

def def_motion(data):
	client = Openbox_findClient(openbox, data.window())
	if not client: return

	global posqueue
	dx = data.xroot() - posqueue[0][1]
	dy = data.yroot() - posqueue[0][2]

	area = posqueue[0][3] # A Rect
	type = data.target()
	if (type == Type_Titlebar) or (type == Type_Label):
		OBClient_move(client, Rect_x(area) + dx, Rect_y(area) + dy)
	elif type == Type_LeftGrip:
		OBClient_resize(client, OBClient_TopRight,
				Rect_width(area) - dx, Rect_height(area) + dy)
	elif type == Type_RightGrip:
		OBClient_resize(client, OBClient_TopLeft,
				Rect_width(area) + dx, Rect_height(area) + dy)

def def_enter(data):
	client = Openbox_findClient(openbox, data.window())
	if not client: return
	if enter_focus != 0:
		OBClient_focus(client)

def def_leave(data):
	client = Openbox_findClient(openbox, data.window())
	if not client: return
	if leave_unfocus != 0:
		OBClient_unfocus(client)


register(Action_EnterWindow, def_enter)
register(Action_LeaveWindow, def_leave)

register(Action_ButtonPress, def_motion_press)
register(Action_ButtonRelease, def_motion_release)
register(Action_MouseMotion, def_motion)

print "Loaded clientmotion.py"
