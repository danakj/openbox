// -*- mode: C++; indent-tabs-mode: nil; -*-
#ifndef   __xeventhandler_hh
#define   __xeventhandler_hh

extern "C" {
#include <X11/Xlib.h>
}

namespace ob {

//! Handles X events
/*!
  There are 2 type of X events:<br>
    a) User Actions<br>
    b) Background Events<br>
  <p>
  User Actions are events like mouse drags and presses, key presses.
  Background Events are everything else. Stuff that can't be bound to user
    input.
  <p>
  When an XEvent comes to the application, it is sent to this class. This class
  will determine what the event means, such as "A Left-Mouse-Button Drag on
  this window", or "Double click with right mouse button on root window" or
  "space bar pressed", or Background Event.
  <p>
  If the XEvent or combination of XEvents form a User Action, then the action
  is dispatched to the OBBindingMapper.
  <p>
  If the XEvent is a Background Event, it is simply dealt with as appropriate.
*/
class OBXEventHandler
{
private:
  //! The time at which the last XEvent with a time was received
  Time _lasttime;

  //! Handles mouse button press events
  /*!
    @param e The XEvent to handle
  */
  void buttonPress(const XButtonEvent &e);
  //! Handles mouse button release events
  /*!
    @param e The XEvent to handle
  */
  void buttonRelease(const XButtonEvent &e);
  //! Handles keyboard key press events
  /*!
    @param e The XEvent to handle
  */
  void keyPress(const XKeyEvent &e); 
  //! Handles mouse motion events
  /*!
    @param e The XEvent to handle
  */
  void motion(const XMotionEvent &e);
  //! Handles mouse-enter events
  /*!
    @param e The XEvent to handle
  */
  void enterNotify(const XCrossingEvent &e);
  //! Handles mouse-leave events
  /*!
    @param e The XEvent to handle
  */
  void leaveNotify(const XCrossingEvent &e);
  //! Handles configure request events
  /*!
    @param e The XEvent to handle
  */
  void configureRequest(const XConfigureRequestEvent &e);
  //! Handles window map request events
  /*!
    @param e The XEvent to handle
  */
  void mapRequest(const XMapRequestEvent &e);
  //! Handles window unmap events
  /*!
    @param e The XEvent to handle
  */
  void unmapNotify(const XUnmapEvent &e);
  //! Handles window destroy events
  /*!
    @param e The XEvent to handle
  */
  void destroyNotify(const XDestroyWindowEvent &e);
  //! Handles window reparent events
  /*!
    @param e The XEvent to handle
  */
  void reparentNotify(const XReparentEvent &e);
  //! Handles window property change events
  /*!
    @param e The XEvent to handle
  */
  void propertyNotify(const XPropertyEvent &e);
  //! Handles window expose events
  /*!
    @param e The XEvent to handle
  */
  void expose(const XExposeEvent &e);
  //! Handles colormap events
  /*!
    @param e The XEvent to handle
  */
  void colormapNotify(const XColormapEvent &e);
  //! Handles focus-in events
  /*!
    @param e The XEvent to handle
  */
  void focusIn(const XFocusChangeEvent &e);
  //! Handles focus-out events
  /*!
    @param e The XEvent to handle
  */
  void focusOut(const XFocusChangeEvent &e);
#if defined(SHAPE) || defined(DOXYGEN_IGNORE)
  //! Handles Shape extension events
  /*!
    @param e The XEvent to handle
  */
  void shapeEvent(const XShapeEvent &e);
#endif // SHAPE 
  //! Handles client message events
  /*!
    @param e The XEvent to handle
  */
  void clientMessage(const XClientMessageEvent &e);
 
  
public:
  //! Constructs an OBXEventHandler object
  OBXEventHandler();
  
  //! Handle an XEvent
  /*!
    @param e The XEvent to handle
  */
  void handle(const XEvent &e);
};

}

#endif // __xeventhandler_hh
