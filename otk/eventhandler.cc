#include "eventhandler.hh"
#include <iostream>

namespace otk {

OtkEventHandler::OtkEventHandler()
{
}


OtkEventHandler::~OtkEventHandler()
{
}


int OtkEventHandler::handle(const XEvent &e)
{
  switch(e.type){
  case KeyPress:
    return keyPressHandler(e.xkey);
  case KeyRelease:
    return keyReleaseHandler(e.xkey);
  case ButtonPress:
    return buttonPressHandler(e.xbutton);
  case ButtonRelease:
    return buttonReleaseHandler(e.xbutton);
  case EnterNotify:
    return enterHandler(e.xcrossing);
  case LeaveNotify:
    return leaveHandler(e.xcrossing);
  case FocusIn:
    return focusHandler(e.xfocus);
  case FocusOut:
    return unfocusHandler(e.xfocus);
  case Expose:
    return exposeHandler(e.xexpose);
  case GraphicsExpose:
    return graphicsExposeHandler(e.xgraphicsexpose);
  case NoExpose:
    return noExposeEventHandler(e.xnoexpose);
  case CirculateRequest:
    return circulateRequestHandler(e.xcirculaterequest);
  case ConfigureRequest:
    return configureRequestHandler(e.xconfigurerequest);
  case MapRequest:
    return mapRequestHandler(e.xmaprequest);
  case ResizeRequest:
    return resizeRequestHandler(e.xresizerequest);
  case CirculateNotify:
    return circulateHandler(e.xcirculate);
  case ConfigureNotify:
    return configureHandler(e.xconfigure);
  case CreateNotify:
    return createHandler(e.xcreatewindow);
  case DestroyNotify:
    return destroyHandler(e.xdestroywindow);
  case GravityNotify:
    return gravityHandler(e.xgravity);
  case MapNotify:
    return mapHandler(e.xmap);
  case MappingNotify:
    return mappingHandler(e.xmapping);
  case ReparentNotify:
    return reparentHandler(e.xreparent);
  case UnmapNotify:
    return unmapHandler(e.xunmap);
  case VisibilityNotify:
    return visibilityHandler(e.xvisibility);
  case ColormapNotify:
    return colorMapHandler(e.xcolormap);
  case ClientMessage:
    return clientMessageHandler(e.xclient);
  case PropertyNotify:
    return propertyHandler(e.xproperty);
  case SelectionClear:
    return selectionClearHandler(e.xselectionclear);
  case SelectionNotify:
    return selectionHandler(e.xselection);
  case SelectionRequest:
    return selectionRequestHandler(e.xselectionrequest);
  };
  return 0;
}

}
