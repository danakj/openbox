def def_click_client(data):
	client = Openbox_findClient(openbox, data.window())
	if not client: return

	button = data.button()
	type = data.target()
	if button == 1 and type == Type_CloseButton:
		OBClient_close(client)
	elif button <= 3 and type == Type_MaximizeButton:
		print "OBClient_maximize(client)"
	elif button == 1 and type == Type_IconifyButton:
		print "OBClient_iconify(client)"
	elif button == 1 and type == Type_StickyButton:
		print "OBClient_sendtodesktop(client, 0xffffffff)"
	elif type == Type_Titlebar or type == Type_CloseButton or \
	     type == Type_MaximizeButton or type == Type_IconifyButton or \
	     type == Type_StickyButton or type == Type_Label:
		if button == 4:
			print "OBClient_shade(client)"
		elif button == 5:
			print "OBClient_unshade(client)"

def def_press_model(data):
	if data.button() != 1: return
	client = Openbox_findClient(openbox, data.window())
	if not client or (type == Type_StickyButton or
			  type == Type_IconifyButton or
			  type == Type_MaximizeButton or
			  type == Type_CloseButton):
		return
	if click_focus != 0:
		OBClient_focus(client)
	if click_raise != 0:
		print "OBClient_raise(client)"

def def_press_root(data):
	button = data.button()
	if type == Type_Root:
		if button == 1:
			print "nothing probly.."
			client = Openbox_focusedClient(openbox)
			if client: OBClient_unfocus(client)
		elif button == 2:
			print "workspace menu"
		elif button == 3:
			print "root menu"
		elif button == 4:
			print "next workspace"
		elif button == 5:
			print "previous workspace"

def def_doubleclick_client(data):
	client = Openbox_findClient(openbox, data.window())
	if not client: return

	button = data.button()
	if button == 1 and (type == Type_Titlebar or type == Type_Label):
		print "OBClient_toggleshade(client)"


register(Action_ButtonPress, def_press_model, 1)
register(Action_Click, def_click_client)
register(Action_ButtonPress, def_press_root)
register(Action_DoubleClick, def_doubleclick_client)

print "Loaded clicks.py"
