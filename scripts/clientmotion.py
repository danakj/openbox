posqueue = [];

def motion_press(action, win, type, modifiers, button, xroot, yroot, time):
	client = Openbox_findClient(openbox, win)

	global posqueue
	newi = [button, xroot, yroot]
	if client:
		newi.append(new_Rect(OBClient_area(client)))
	posqueue.append(newi)

	#  ButtonPressAction *a = _posqueue[BUTTONS - 1];
	#  for (int i=BUTTONS-1; i>0;)
	#    _posqueue[i] = _posqueue[--i];
	#  _posqueue[0] = a;
	#  a->button = e.button;
	#  a->pos.setPoint(e.x_root, e.y_root);
	
	#  OBClient *c = Openbox::instance->findClient(e.window);
	#  // if it's not defined, they should have clicked on the root window, so this
	#  // area would be meaningless anyways
	#  if (c) a->clientarea = c->area();
	
def motion_release(action, win, type, modifiers, button, xroot, yroot, time):
	global posqueue
	for i in posqueue:
		if i[0] == button:
			#delete_Rect i[3]
			posqueue.remove(i)
			break
	
	#  ButtonPressAction *a = 0;
	#  for (int i=0; i<BUTTONS; ++i) {
	#    if (_posqueue[i]->button == e.button)
	#      a = _posqueue[i];
	#    if (a) // found one and removed it
	#      _posqueue[i] = _posqueue[i+1];
	#  }
	#  if (a) { // found one
	#    _posqueue[BUTTONS-1] = a;
	#    a->button = 0;
	#  }


def motion(action, win, type, modifiers, xroot, yroot, time):
	client = Openbox_findClient(openbox, win)

	global posqueue
	dx = xroot - posqueue[0][1]
	dy = yroot - posqueue[0][2]
	#  _dx = x_root - _posqueue[0]->pos.x();
	#  _dy = y_root - _posqueue[0]->pos.y();

	if not client:
		return
	area = posqueue[0][3] # A Rect
	if (type == Type_Titlebar) or (type == Type_Label):
		OBClient_move(client, Rect_x(area) + dx, Rect_y(area) + dy)
		#      c->move(_posqueue[0]->clientarea.x() + _dx,
		#              _posqueue[0]->clientarea.y() + _dy);
	elif type == Type_LeftGrip:
		OBClient_resize(client, OBClient_TopRight,
				Rect_width(area) - dx, Rect_height(area) + dy)
		#      c->resize(OBClient::TopRight,
		#        _posqueue[0]->clientarea.width() - _dx,
		#        _posqueue[0]->clientarea.height() + _dy);
	elif type == Type_RightGrip:
		OBClient_resize(client, OBClient_TopLeft,
				Rect_width(area) + dx, Rect_height(area) + dy)
		#      c->resize(OBClient::TopLeft,
		#        _posqueue[0]->clientarea.width() + _dx,
		#        _posqueue[0]->clientarea.height() + _dy);


register(Action_ButtonPress, motion_press)
register(Action_ButtonRelease, motion_release)
register(Action_MouseMotion, motion)
