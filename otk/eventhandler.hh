#ifndef __eventhandler__hh
#define __eventhandler__hh

extern "C" {
#include <X11/Xlib.h>
}

namespace otk {

class OtkEventHandler{
public:
  //! Dispatches events to one of the other handlers based on their type.
  virtual int handle(const XEvent &e);

  //! Called whenever any key is pressed.
  virtual int keyPressHandler(const XKeyEvent &) {return 1;}

  //! Called whenever any key is released.
  virtual int keyReleaseHandler(const XKeyEvent &) {return 1;}

  //! Called whenever a button of the pointer is pressed.
  virtual int buttonPressHandler(const XButtonEvent &) {return 1;}

  //! Called whenever a button of the pointer is released.
  virtual int buttonReleaseHandler(const XButtonEvent &) {return 1;}

  //! Called whenever the pointer enters a window.
  virtual int enterHandler(const XCrossingEvent &) {return 1;}

  //! Called whenever the pointer leaves a window.
  virtual int leaveHandler(const XCrossingEvent &) {return 1;}

  //! Called when a window gains focus.
  virtual int focusHandler(const XFocusChangeEvent &) {return 1;}

  //! Called when a window looses focus.
  virtual int unfocusHandler(const XFocusChangeEvent &) {return 1;}

  //! Called when a window becomes visible to the user.
  virtual int exposeHandler(const XExposeEvent &) {return 1;}

  //! Called to handle GraphicsExpose events.
  virtual int graphicsExposeHandler(const XGraphicsExposeEvent &) {return 1;}

  //! Called to handle NoExpose events.
  virtual int noExposeEventHandler(const XNoExposeEvent &) {return 1;}

  //! Called when the window requests a change in its z-order.
  virtual int circulateRequestHandler(const XCirculateRequestEvent &)
  {return 1;}

  //! Called when a different client initiates a configure window request.
  virtual int configureRequestHandler(const XConfigureRequestEvent &)
  {return 1;}

  //! Called when a different client tries to map a window.
  virtual int mapRequestHandler(const XMapRequestEvent &) {return 1;}

  //! Called when another client attemps to change the size of a window.
  virtual int resizeRequestHandler(const XResizeRequestEvent &) {return 1;}

  //! Called when the z-order of the window has changed.
  virtual int circulateHandler(const XCirculateEvent &) {return 1;}

  //! Called when the window as been reconfigured.
  virtual int configureHandler(const XConfigureEvent &) {return 1;}

  //! Called when a window is created.
  virtual int createHandler(const XCreateWindowEvent &) {return 1;}

  //! Called when a window is destroyed.
  virtual int destroyHandler(const XDestroyWindowEvent &) {return 1;}

  //! Called when a window is moved because of a change in the size of its 
  //! parent.
  virtual int gravityHandler(const XGravityEvent &) {return 1;}

  //! Called when a window is mapped.
  virtual int mapHandler(const XMapEvent &) {return 1;}

  //! Called when the server generats a MappingNotify event
  virtual int mappingHandler(const XMappingEvent &) {return 1;}

  //! Called when a window is reparented
  virtual int reparentHandler(const XReparentEvent &) {return 1;}

  //! Called when a window is unmapped
  virtual int unmapHandler(const XUnmapEvent &) {return 1;}

  //! Called when a the visibilty of a window changes
  virtual int visibilityHandler(const XVisibilityEvent &) {return 1;}

  //! Called when the colormap changes, or is installed or unistalled
  virtual int colorMapHandler(const XColormapEvent &) {return 1;}

  //! Called when a client calls XSendEvent
  virtual int clientMessageHandler(const XClientMessageEvent &) {return 1;}

  //! Called when a property of a window changes
  virtual int propertyHandler(const XPropertyEvent &) {return 1;}

  //! Called when the client loses ownership of a selection
  virtual int selectionClearHandler(const XSelectionClearEvent &) {return 1;}

  //! Called when a ConvertSelection protocol request is sent
  virtual int selectionHandler(const XSelectionEvent &) {return 1;}

  //! Called when a SelectionEvent occurs
  virtual int selectionRequestHandler(const XSelectionRequestEvent &)
  {return 1;}

  virtual ~OtkEventHandler();

protected:
  /*! Constructor for the XEventHandler class.
    This is protected so that XEventHandlers can't be instantiated on their
    own.
  */
  OtkEventHandler();

private:
};

}

#endif
