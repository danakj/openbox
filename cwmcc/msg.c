void prop_message(Window about, Atom messagetype, long data0, long data1,
		  long data2, long data3)
{
    XEvent ce;
    ce.xclient.type = ClientMessage;
    ce.xclient.message_type = messagetype;
    ce.xclient.display = cwmcc_display;
    ce.xclient.window = about;
    ce.xclient.format = 32;
    ce.xclient.data.l[0] = data0;
    ce.xclient.data.l[1] = data1;
    ce.xclient.data.l[2] = data2;
    ce.xclient.data.l[3] = data3;
    XSendEvent(cwmcc_display, ob_root, FALSE,
	       SubstructureNotifyMask | SubstructureRedirectMask, &ce);
}
