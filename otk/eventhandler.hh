#ifndef __eventhandler__hh
#define __eventhandler__hh

extern "C" {
#include <X11/Xlib.h>

#ifdef    SHAPE
#include <X11/extensions/shape.h>
#endif // SHAPE
}

namespace otk {

class OtkEventHandler{
public:
  //! Dispatches events to one of the other handlers based on their type.
  virtual void handle(const XEvent &e);

  //! Called whenever any key is pressed.
  virtual void keyPressHandler(const XKeyEvent &) {}

  //! Called whenever any key is released.
  virtual void keyReleaseHandler(const XKeyEvent &) {}

  //! Called whenever a button of the pointer is pressed.
  virtual void buttonPressHandler(const XButtonEvent &) {}

  //! Called whenever a button of the pointer is released.
  virtual void buttonReleaseHandler(const XButtonEvent &) {}

  //! Called whenever the pointer enters a window.
  virtual void enterHandler(const XCrossingEvent &) {}

  //! Called whenever the pointer leaves a window.
  virtual void leaveHandler(const XCrossingEvent &) {}

  //! Called when a window gains focus.
  virtual void focusHandler(const XFocusChangeEvent &) {}

  //! Called when a window looses focus.
  virtual void unfocusHandler(const XFocusChangeEvent &) {}

  //! Called when a window becomes visible to the user.
  virtual void exposeHandler(const XExposeEvent &) {}

  //! Called to handle GraphicsExpose events.
  virtual void graphicsExposeHandler(const XGraphicsExposeEvent &) {}

  //! Called to handle NoExpose events.
  virtual void noExposeEventHandler(const XNoExposeEvent &) {}

  //! Called when the window requests a change in its z-order.
  virtual void circulateRequestHandler(const XCirculateRequestEvent &)
  {}

  //! Called when a different client initiates a configure window request.
  virtual void configureRequestHandler(const XConfigureRequestEvent &)
  {}

  //! Called when a different client tries to map a window.
  virtual void mapRequestHandler(const XMapRequestEvent &) {}

  //! Called when another client attemps to change the size of a window.
  virtual void resizeRequestHandler(const XResizeRequestEvent &) {}

  //! Called when the z-order of the window has changed.
  virtual void circulateHandler(const XCirculateEvent &) {}

  //! Called when the window as been reconfigured.
  virtual void configureHandler(const XConfigureEvent &) {}

  //! Called when a window is created.
  virtual void createHandler(const XCreateWindowEvent &) {}

  //! Called when a window is destroyed.
  virtual void destroyHandler(const XDestroyWindowEvent &) {}

  //! Called when a window is moved because of a change in the size of its 
  //! parent.
  virtual void gravityHandler(const XGravityEvent &) {}

  //! Called when a window is mapped.
  virtual void mapHandler(const XMapEvent &) {}

  //! Called when the server generats a MappingNotify event
  virtual void mappingHandler(const XMappingEvent &) {}

  //! Called when a window is reparented
  virtual void reparentHandler(const XReparentEvent &) {}

  //! Called when a window is unmapped
  virtual void unmapHandler(const XUnmapEvent &) {}

  //! Called when a the visibilty of a window changes
  virtual void visibilityHandler(const XVisibilityEvent &) {}

  //! Called when the colormap changes, or is installed or unistalled
  virtual void colorMapHandler(const XColormapEvent &) {}

  //! Called when a property of a window changes
  virtual void propertyHandler(const XPropertyEvent &) {}

  //! Called when the client loses ownership of a selection
  virtual void selectionClearHandler(const XSelectionClearEvent &) {}

  //! Called when a ConvertSelection protocol request is sent
  virtual void selectionHandler(const XSelectionEvent &) {}

  //! Called when a SelectionEvent occurs
  virtual void selectionRequestHandler(const XSelectionRequestEvent &) {}

  //! Called when a client calls XSendEvent
  /*!
    Some types of client messages are filtered out and sent to more specific
    event handler functions.
  */
  virtual void clientMessageHandler(const XClientMessageEvent &);

#if defined(SHAPE) || defined(DOXYGEN_IGNORE)
  //! Called when a shape extention event fires
  virtual void shapeHandler(const XShapeEvent &) {};
#endif // SHAPE 

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
