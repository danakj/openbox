def def_click_client(action, win, type, modifiers, button, time):
	client = Openbox_findClient(openbox, win)
	if not client: return

	if button == Button1 and type == Type_CloseButton:
		OBClient_close(client)
	elif button <= Button3 and type == Type_MaximizeButton:
		print "OBClient_maximize(client)"
	elif button == Button1 and type == Type_IconifyButton:
		print "OBClient_iconify(client)"
	elif button == Button1 and type == Type_StickyButton:
		print "OBClient_sendtodesktop(client, 0xffffffff)"
	elif type == Type_Titlebar or type == Type_CloseButton or \
	     type == Type_MaximizeButton or type == Type_IconifyButton or \
	     type == Type_StickyButton or type == Type_Label:
		if button == Button4:
			print "OBClient_shade(client)"
		elif button == Button5:
			print "OBClient_unshade(client)"

def def_press_model(action, win, type, modifiers, button, xroot, yroot, time):
	if button != Button1: return
	client = Openbox_findClient(openbox, win)
	if not client or (type == Type_StickyButton or
			  type == Type_IconifyButton or
			  type == Type_MaximizeButton or
			  type == Type_CloseButton):
		return
	if click_focus != 0:
		OBClient_focus(client)
	print "OBClient_raise(client)"

def def_click_root(action, win, type, modifiers, button, time):
	if type == Type_Root:
		if button == Button1:
			print "nothing probly.."
			client = Openbox_focusedClient(openbox)
			if client: OBClient_unfocus(client)
		elif button == Button2:
			print "workspace menu"
		elif button == Button3:
			print "root menu"
		elif button == Button4:
			print "next workspace"
		elif button == Button5:
			print "previous workspace"

def def_doubleclick_client(action, win, type, modifiers, button, time):
	client = Openbox_findClient(openbox, win)
	if not client: return

	if button == Button1 and (type == Type_Titlebar or type == Type_Label):
		print "OBClient_toggleshade(client)"


preregister(Action_ButtonPress, def_press_model)
register(Action_Click, def_click_client)
register(Action_Click, def_click_root)
register(Action_DoubleClick, def_doubleclick_client)

print "Loaded clicks.py"
