// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module ob_frame

%{
#include "config.h"
#include "frame.hh"
%}

namespace ob {

%ignore FrameGeometry;

%ignore Frame::event_mask;
%ignore Frame::Frame(Client*);
%ignore Frame::~Frame();
%ignore Frame::styleChanged(const otk::RenderStyle &);
%ignore Frame::grabClient();
%ignore Frame::releaseClient();
%ignore Frame::adjustSize();
%ignore Frame::adjustPosition();
%ignore Frame::adjustShape();
%ignore Frame::adjustState();
%ignore Frame::adjustFocus();
%ignore Frame::adjustTitle();
%ignore Frame::adjustIcon();
%ignore Frame::visible();
%ignore Frame::show();
%ignore Frame::hide();
%ignore Frame::buttonPressHandler(const XButtonEvent &);
%ignore Frame::buttonReleaseHandler(const XButtonEvent &);
%ignore Frame::mouseContext(Window) const;
%ignore Frame::allWindows() const;

}

%import "../otk/renderstyle.hh"
%import "../otk/eventhandler.hh"
%include "frame.hh"
