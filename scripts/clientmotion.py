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

def def_do_move(xroot, yroot, client):
	global posqueue
	dx = xroot - posqueue[0][1]
	dy = yroot - posqueue[0][2]
	area = posqueue[0][3] # A Rect
	OBClient_move(client, Rect_x(area) + dx, Rect_y(area) + dy)

def def_do_resize(xroot, yroot, client, anchor_corner):
	global posqueue
	dx = xroot - posqueue[0][1]
	dy = yroot - posqueue[0][2]
	OBClient_resize(client, anchor_corner,
			Rect_width(area) - dx, Rect_height(area) + dy)

def def_motion(data):
	client = Openbox_findClient(openbox, data.window())
	if not client: return

	global posqueue
	if not posqueue[0][0] == 1: return
	
	type = data.target()
	if (type == Type_Titlebar) or (type == Type_Label) or \
	   (type == Type_Plate) or (type == Type_Handle):
		def_do_move(data.xroot(), data.yroot(), client)
	elif type == Type_LeftGrip:
		def_do_resize(data.xroot(), data.yroot(), client,
			      OBClient_TopRight)
	elif type == Type_RightGrip:
		def_do_resize(data.xroot(), data.yroot(), client,
			      OBClient_TopLeft)

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
