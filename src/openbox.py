# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.
import _openbox
def _swig_setattr(self,class_type,name,value):
    if (name == "this"):
        if isinstance(value, class_type):
            self.__dict__[name] = value.this
            if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
            del value.thisown
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    self.__dict__[name] = value

def _swig_getattr(self,class_type,name):
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0


Openbox_instance = _openbox.Openbox_instance

class Point(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Point, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Point, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_Point,args)
        self.thisown = 1
    def setX(*args): return apply(_openbox.Point_setX,args)
    def x(*args): return apply(_openbox.Point_x,args)
    def setY(*args): return apply(_openbox.Point_setY,args)
    def y(*args): return apply(_openbox.Point_y,args)
    def setPoint(*args): return apply(_openbox.Point_setPoint,args)
    def __repr__(self):
        return "<C Point instance at %s>" % (self.this,)

class PointPtr(Point):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Point
_openbox.Point_swigregister(PointPtr)

class Rect(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Rect, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Rect, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_Rect,args)
        self.thisown = 1
    def left(*args): return apply(_openbox.Rect_left,args)
    def top(*args): return apply(_openbox.Rect_top,args)
    def right(*args): return apply(_openbox.Rect_right,args)
    def bottom(*args): return apply(_openbox.Rect_bottom,args)
    def x(*args): return apply(_openbox.Rect_x,args)
    def y(*args): return apply(_openbox.Rect_y,args)
    def location(*args): return apply(_openbox.Rect_location,args)
    def setX(*args): return apply(_openbox.Rect_setX,args)
    def setY(*args): return apply(_openbox.Rect_setY,args)
    def setPos(*args): return apply(_openbox.Rect_setPos,args)
    def width(*args): return apply(_openbox.Rect_width,args)
    def height(*args): return apply(_openbox.Rect_height,args)
    def size(*args): return apply(_openbox.Rect_size,args)
    def setWidth(*args): return apply(_openbox.Rect_setWidth,args)
    def setHeight(*args): return apply(_openbox.Rect_setHeight,args)
    def setSize(*args): return apply(_openbox.Rect_setSize,args)
    def setRect(*args): return apply(_openbox.Rect_setRect,args)
    def setCoords(*args): return apply(_openbox.Rect_setCoords,args)
    def __eq__(*args): return apply(_openbox.Rect___eq__,args)
    def __ne__(*args): return apply(_openbox.Rect___ne__,args)
    def __or__(*args): return apply(_openbox.Rect___or__,args)
    def __and__(*args): return apply(_openbox.Rect___and__,args)
    def __ior__(*args): return apply(_openbox.Rect___ior__,args)
    def __iand__(*args): return apply(_openbox.Rect___iand__,args)
    def valid(*args): return apply(_openbox.Rect_valid,args)
    def intersects(*args): return apply(_openbox.Rect_intersects,args)
    def contains(*args): return apply(_openbox.Rect_contains,args)
    def __repr__(self):
        return "<C Rect instance at %s>" % (self.this,)

class RectPtr(Rect):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Rect
_openbox.Rect_swigregister(RectPtr)

class Cursors(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Cursors, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Cursors, name)
    __swig_setmethods__["session"] = _openbox.Cursors_session_set
    __swig_getmethods__["session"] = _openbox.Cursors_session_get
    if _newclass:session = property(_openbox.Cursors_session_get,_openbox.Cursors_session_set)
    __swig_setmethods__["move"] = _openbox.Cursors_move_set
    __swig_getmethods__["move"] = _openbox.Cursors_move_get
    if _newclass:move = property(_openbox.Cursors_move_get,_openbox.Cursors_move_set)
    __swig_setmethods__["ll_angle"] = _openbox.Cursors_ll_angle_set
    __swig_getmethods__["ll_angle"] = _openbox.Cursors_ll_angle_get
    if _newclass:ll_angle = property(_openbox.Cursors_ll_angle_get,_openbox.Cursors_ll_angle_set)
    __swig_setmethods__["lr_angle"] = _openbox.Cursors_lr_angle_set
    __swig_getmethods__["lr_angle"] = _openbox.Cursors_lr_angle_get
    if _newclass:lr_angle = property(_openbox.Cursors_lr_angle_get,_openbox.Cursors_lr_angle_set)
    __swig_setmethods__["ul_angle"] = _openbox.Cursors_ul_angle_set
    __swig_getmethods__["ul_angle"] = _openbox.Cursors_ul_angle_get
    if _newclass:ul_angle = property(_openbox.Cursors_ul_angle_get,_openbox.Cursors_ul_angle_set)
    __swig_setmethods__["ur_angle"] = _openbox.Cursors_ur_angle_set
    __swig_getmethods__["ur_angle"] = _openbox.Cursors_ur_angle_get
    if _newclass:ur_angle = property(_openbox.Cursors_ur_angle_get,_openbox.Cursors_ur_angle_set)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C Cursors instance at %s>" % (self.this,)

class CursorsPtr(Cursors):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Cursors
_openbox.Cursors_swigregister(CursorsPtr)

class Openbox(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Openbox, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Openbox, name)
    State_Starting = _openbox.Openbox_State_Starting
    State_Normal = _openbox.Openbox_State_Normal
    State_Exiting = _openbox.Openbox_State_Exiting
    def state(*args): return apply(_openbox.Openbox_state,args)
    def timerManager(*args): return apply(_openbox.Openbox_timerManager,args)
    def property(*args): return apply(_openbox.Openbox_property,args)
    def actions(*args): return apply(_openbox.Openbox_actions,args)
    def bindings(*args): return apply(_openbox.Openbox_bindings,args)
    def screen(*args): return apply(_openbox.Openbox_screen,args)
    def screenCount(*args): return apply(_openbox.Openbox_screenCount,args)
    def cursors(*args): return apply(_openbox.Openbox_cursors,args)
    def addClient(*args): return apply(_openbox.Openbox_addClient,args)
    def removeClient(*args): return apply(_openbox.Openbox_removeClient,args)
    def findClient(*args): return apply(_openbox.Openbox_findClient,args)
    def focusedClient(*args): return apply(_openbox.Openbox_focusedClient,args)
    def setFocusedClient(*args): return apply(_openbox.Openbox_setFocusedClient,args)
    def focusedScreen(*args): return apply(_openbox.Openbox_focusedScreen,args)
    def shutdown(*args): return apply(_openbox.Openbox_shutdown,args)
    def restart(*args): return apply(_openbox.Openbox_restart,args)
    def execute(*args): return apply(_openbox.Openbox_execute,args)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C Openbox instance at %s>" % (self.this,)

class OpenboxPtr(Openbox):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Openbox
_openbox.Openbox_swigregister(OpenboxPtr)

class OBScreen(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, OBScreen, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, OBScreen, name)
    def client(*args): return apply(_openbox.OBScreen_client,args)
    def clientCount(*args): return apply(_openbox.OBScreen_clientCount,args)
    event_mask = _openbox.OBScreen_event_mask
    def number(*args): return apply(_openbox.OBScreen_number,args)
    def managed(*args): return apply(_openbox.OBScreen_managed,args)
    def imageControl(*args): return apply(_openbox.OBScreen_imageControl,args)
    def area(*args): return apply(_openbox.OBScreen_area,args)
    def style(*args): return apply(_openbox.OBScreen_style,args)
    def focuswindow(*args): return apply(_openbox.OBScreen_focuswindow,args)
    def desktop(*args): return apply(_openbox.OBScreen_desktop,args)
    def numDesktops(*args): return apply(_openbox.OBScreen_numDesktops,args)
    def updateStrut(*args): return apply(_openbox.OBScreen_updateStrut,args)
    def manageExisting(*args): return apply(_openbox.OBScreen_manageExisting,args)
    def manageWindow(*args): return apply(_openbox.OBScreen_manageWindow,args)
    def unmanageWindow(*args): return apply(_openbox.OBScreen_unmanageWindow,args)
    def restack(*args): return apply(_openbox.OBScreen_restack,args)
    def setDesktopName(*args): return apply(_openbox.OBScreen_setDesktopName,args)
    def propertyHandler(*args): return apply(_openbox.OBScreen_propertyHandler,args)
    def clientMessageHandler(*args): return apply(_openbox.OBScreen_clientMessageHandler,args)
    def mapRequestHandler(*args): return apply(_openbox.OBScreen_mapRequestHandler,args)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C OBScreen instance at %s>" % (self.this,)

class OBScreenPtr(OBScreen):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OBScreen
_openbox.OBScreen_swigregister(OBScreenPtr)

class MwmHints(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, MwmHints, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, MwmHints, name)
    __swig_setmethods__["flags"] = _openbox.MwmHints_flags_set
    __swig_getmethods__["flags"] = _openbox.MwmHints_flags_get
    if _newclass:flags = property(_openbox.MwmHints_flags_get,_openbox.MwmHints_flags_set)
    __swig_setmethods__["functions"] = _openbox.MwmHints_functions_set
    __swig_getmethods__["functions"] = _openbox.MwmHints_functions_get
    if _newclass:functions = property(_openbox.MwmHints_functions_get,_openbox.MwmHints_functions_set)
    __swig_setmethods__["decorations"] = _openbox.MwmHints_decorations_set
    __swig_getmethods__["decorations"] = _openbox.MwmHints_decorations_get
    if _newclass:decorations = property(_openbox.MwmHints_decorations_get,_openbox.MwmHints_decorations_set)
    elements = _openbox.MwmHints_elements
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C MwmHints instance at %s>" % (self.this,)

class MwmHintsPtr(MwmHints):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = MwmHints
_openbox.MwmHints_swigregister(MwmHintsPtr)

class OBClient(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, OBClient, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, OBClient, name)
    __swig_setmethods__["frame"] = _openbox.OBClient_frame_set
    __swig_getmethods__["frame"] = _openbox.OBClient_frame_get
    if _newclass:frame = property(_openbox.OBClient_frame_get,_openbox.OBClient_frame_set)
    Layer_Icon = _openbox.OBClient_Layer_Icon
    Layer_Desktop = _openbox.OBClient_Layer_Desktop
    Layer_Below = _openbox.OBClient_Layer_Below
    Layer_Normal = _openbox.OBClient_Layer_Normal
    Layer_Above = _openbox.OBClient_Layer_Above
    Layer_Top = _openbox.OBClient_Layer_Top
    Layer_Fullscreen = _openbox.OBClient_Layer_Fullscreen
    Layer_Internal = _openbox.OBClient_Layer_Internal
    NUM_LAYERS = _openbox.OBClient_NUM_LAYERS
    TopLeft = _openbox.OBClient_TopLeft
    TopRight = _openbox.OBClient_TopRight
    BottomLeft = _openbox.OBClient_BottomLeft
    BottomRight = _openbox.OBClient_BottomRight
    Type_Desktop = _openbox.OBClient_Type_Desktop
    Type_Dock = _openbox.OBClient_Type_Dock
    Type_Toolbar = _openbox.OBClient_Type_Toolbar
    Type_Menu = _openbox.OBClient_Type_Menu
    Type_Utility = _openbox.OBClient_Type_Utility
    Type_Splash = _openbox.OBClient_Type_Splash
    Type_Dialog = _openbox.OBClient_Type_Dialog
    Type_Normal = _openbox.OBClient_Type_Normal
    MwmFlag_Functions = _openbox.OBClient_MwmFlag_Functions
    MwmFlag_Decorations = _openbox.OBClient_MwmFlag_Decorations
    MwmFunc_All = _openbox.OBClient_MwmFunc_All
    MwmFunc_Resize = _openbox.OBClient_MwmFunc_Resize
    MwmFunc_Move = _openbox.OBClient_MwmFunc_Move
    MwmFunc_Iconify = _openbox.OBClient_MwmFunc_Iconify
    MwmFunc_Maximize = _openbox.OBClient_MwmFunc_Maximize
    MwmDecor_All = _openbox.OBClient_MwmDecor_All
    MwmDecor_Border = _openbox.OBClient_MwmDecor_Border
    MwmDecor_Handle = _openbox.OBClient_MwmDecor_Handle
    MwmDecor_Title = _openbox.OBClient_MwmDecor_Title
    MwmDecor_Iconify = _openbox.OBClient_MwmDecor_Iconify
    MwmDecor_Maximize = _openbox.OBClient_MwmDecor_Maximize
    Func_Resize = _openbox.OBClient_Func_Resize
    Func_Move = _openbox.OBClient_Func_Move
    Func_Iconify = _openbox.OBClient_Func_Iconify
    Func_Maximize = _openbox.OBClient_Func_Maximize
    Func_Close = _openbox.OBClient_Func_Close
    Decor_Titlebar = _openbox.OBClient_Decor_Titlebar
    Decor_Handle = _openbox.OBClient_Decor_Handle
    Decor_Border = _openbox.OBClient_Decor_Border
    Decor_Iconify = _openbox.OBClient_Decor_Iconify
    Decor_Maximize = _openbox.OBClient_Decor_Maximize
    Decor_Sticky = _openbox.OBClient_Decor_Sticky
    Decor_Close = _openbox.OBClient_Decor_Close
    State_Remove = _openbox.OBClient_State_Remove
    State_Add = _openbox.OBClient_State_Add
    State_Toggle = _openbox.OBClient_State_Toggle
    event_mask = _openbox.OBClient_event_mask
    no_propagate_mask = _openbox.OBClient_no_propagate_mask
    __swig_setmethods__["ignore_unmaps"] = _openbox.OBClient_ignore_unmaps_set
    __swig_getmethods__["ignore_unmaps"] = _openbox.OBClient_ignore_unmaps_get
    if _newclass:ignore_unmaps = property(_openbox.OBClient_ignore_unmaps_get,_openbox.OBClient_ignore_unmaps_set)
    def screen(*args): return apply(_openbox.OBClient_screen,args)
    def window(*args): return apply(_openbox.OBClient_window,args)
    def type(*args): return apply(_openbox.OBClient_type,args)
    def normal(*args): return apply(_openbox.OBClient_normal,args)
    def desktop(*args): return apply(_openbox.OBClient_desktop,args)
    def title(*args): return apply(_openbox.OBClient_title,args)
    def iconTitle(*args): return apply(_openbox.OBClient_iconTitle,args)
    def appName(*args): return apply(_openbox.OBClient_appName,args)
    def appClass(*args): return apply(_openbox.OBClient_appClass,args)
    def role(*args): return apply(_openbox.OBClient_role,args)
    def canFocus(*args): return apply(_openbox.OBClient_canFocus,args)
    def urgent(*args): return apply(_openbox.OBClient_urgent,args)
    def focusNotify(*args): return apply(_openbox.OBClient_focusNotify,args)
    def shaped(*args): return apply(_openbox.OBClient_shaped,args)
    def gravity(*args): return apply(_openbox.OBClient_gravity,args)
    def positionRequested(*args): return apply(_openbox.OBClient_positionRequested,args)
    def decorations(*args): return apply(_openbox.OBClient_decorations,args)
    def funtions(*args): return apply(_openbox.OBClient_funtions,args)
    def transientFor(*args): return apply(_openbox.OBClient_transientFor,args)
    def modal(*args): return apply(_openbox.OBClient_modal,args)
    def shaded(*args): return apply(_openbox.OBClient_shaded,args)
    def iconic(*args): return apply(_openbox.OBClient_iconic,args)
    def maxVert(*args): return apply(_openbox.OBClient_maxVert,args)
    def maxHorz(*args): return apply(_openbox.OBClient_maxHorz,args)
    def layer(*args): return apply(_openbox.OBClient_layer,args)
    def toggleClientBorder(*args): return apply(_openbox.OBClient_toggleClientBorder,args)
    def area(*args): return apply(_openbox.OBClient_area,args)
    def strut(*args): return apply(_openbox.OBClient_strut,args)
    def move(*args): return apply(_openbox.OBClient_move,args)
    def resize(*args): return apply(_openbox.OBClient_resize,args)
    def focus(*args): return apply(_openbox.OBClient_focus,args)
    def unfocus(*args): return apply(_openbox.OBClient_unfocus,args)
    def focusHandler(*args): return apply(_openbox.OBClient_focusHandler,args)
    def unfocusHandler(*args): return apply(_openbox.OBClient_unfocusHandler,args)
    def propertyHandler(*args): return apply(_openbox.OBClient_propertyHandler,args)
    def clientMessageHandler(*args): return apply(_openbox.OBClient_clientMessageHandler,args)
    def configureRequestHandler(*args): return apply(_openbox.OBClient_configureRequestHandler,args)
    def unmapHandler(*args): return apply(_openbox.OBClient_unmapHandler,args)
    def destroyHandler(*args): return apply(_openbox.OBClient_destroyHandler,args)
    def reparentHandler(*args): return apply(_openbox.OBClient_reparentHandler,args)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C OBClient instance at %s>" % (self.this,)

class OBClientPtr(OBClient):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OBClient
_openbox.OBClient_swigregister(OBClientPtr)

MC_Frame = _openbox.MC_Frame
MC_Titlebar = _openbox.MC_Titlebar
MC_Handle = _openbox.MC_Handle
MC_Window = _openbox.MC_Window
MC_MaximizeButton = _openbox.MC_MaximizeButton
MC_CloseButton = _openbox.MC_CloseButton
MC_IconifyButton = _openbox.MC_IconifyButton
MC_StickyButton = _openbox.MC_StickyButton
MC_Grip = _openbox.MC_Grip
MC_Root = _openbox.MC_Root
MC_MenuItem = _openbox.MC_MenuItem
NUM_MOUSE_CONTEXT = _openbox.NUM_MOUSE_CONTEXT
MousePress = _openbox.MousePress
MouseClick = _openbox.MouseClick
MouseDoubleClick = _openbox.MouseDoubleClick
MouseMotion = _openbox.MouseMotion
NUM_MOUSE_ACTION = _openbox.NUM_MOUSE_ACTION
KC_Menu = _openbox.KC_Menu
KC_All = _openbox.KC_All
NUM_KEY_CONTEXT = _openbox.NUM_KEY_CONTEXT
EventEnterWindow = _openbox.EventEnterWindow
EventLeaveWindow = _openbox.EventLeaveWindow
EventPlaceWindow = _openbox.EventPlaceWindow
EventNewWindow = _openbox.EventNewWindow
EventCloseWindow = _openbox.EventCloseWindow
EventStartup = _openbox.EventStartup
EventShutdown = _openbox.EventShutdown
EventFocus = _openbox.EventFocus
EventBell = _openbox.EventBell
NUM_EVENTS = _openbox.NUM_EVENTS
class MouseData(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, MouseData, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, MouseData, name)
    __swig_setmethods__["screen"] = _openbox.MouseData_screen_set
    __swig_getmethods__["screen"] = _openbox.MouseData_screen_get
    if _newclass:screen = property(_openbox.MouseData_screen_get,_openbox.MouseData_screen_set)
    __swig_setmethods__["client"] = _openbox.MouseData_client_set
    __swig_getmethods__["client"] = _openbox.MouseData_client_get
    if _newclass:client = property(_openbox.MouseData_client_get,_openbox.MouseData_client_set)
    __swig_setmethods__["time"] = _openbox.MouseData_time_set
    __swig_getmethods__["time"] = _openbox.MouseData_time_get
    if _newclass:time = property(_openbox.MouseData_time_get,_openbox.MouseData_time_set)
    __swig_setmethods__["state"] = _openbox.MouseData_state_set
    __swig_getmethods__["state"] = _openbox.MouseData_state_get
    if _newclass:state = property(_openbox.MouseData_state_get,_openbox.MouseData_state_set)
    __swig_setmethods__["button"] = _openbox.MouseData_button_set
    __swig_getmethods__["button"] = _openbox.MouseData_button_get
    if _newclass:button = property(_openbox.MouseData_button_get,_openbox.MouseData_button_set)
    __swig_setmethods__["context"] = _openbox.MouseData_context_set
    __swig_getmethods__["context"] = _openbox.MouseData_context_get
    if _newclass:context = property(_openbox.MouseData_context_get,_openbox.MouseData_context_set)
    __swig_setmethods__["action"] = _openbox.MouseData_action_set
    __swig_getmethods__["action"] = _openbox.MouseData_action_get
    if _newclass:action = property(_openbox.MouseData_action_get,_openbox.MouseData_action_set)
    __swig_setmethods__["xroot"] = _openbox.MouseData_xroot_set
    __swig_getmethods__["xroot"] = _openbox.MouseData_xroot_get
    if _newclass:xroot = property(_openbox.MouseData_xroot_get,_openbox.MouseData_xroot_set)
    __swig_setmethods__["yroot"] = _openbox.MouseData_yroot_set
    __swig_getmethods__["yroot"] = _openbox.MouseData_yroot_get
    if _newclass:yroot = property(_openbox.MouseData_yroot_get,_openbox.MouseData_yroot_set)
    __swig_setmethods__["pressx"] = _openbox.MouseData_pressx_set
    __swig_getmethods__["pressx"] = _openbox.MouseData_pressx_get
    if _newclass:pressx = property(_openbox.MouseData_pressx_get,_openbox.MouseData_pressx_set)
    __swig_setmethods__["pressy"] = _openbox.MouseData_pressy_set
    __swig_getmethods__["pressy"] = _openbox.MouseData_pressy_get
    if _newclass:pressy = property(_openbox.MouseData_pressy_get,_openbox.MouseData_pressy_set)
    __swig_setmethods__["press_clientx"] = _openbox.MouseData_press_clientx_set
    __swig_getmethods__["press_clientx"] = _openbox.MouseData_press_clientx_get
    if _newclass:press_clientx = property(_openbox.MouseData_press_clientx_get,_openbox.MouseData_press_clientx_set)
    __swig_setmethods__["press_clienty"] = _openbox.MouseData_press_clienty_set
    __swig_getmethods__["press_clienty"] = _openbox.MouseData_press_clienty_get
    if _newclass:press_clienty = property(_openbox.MouseData_press_clienty_get,_openbox.MouseData_press_clienty_set)
    __swig_setmethods__["press_clientwidth"] = _openbox.MouseData_press_clientwidth_set
    __swig_getmethods__["press_clientwidth"] = _openbox.MouseData_press_clientwidth_get
    if _newclass:press_clientwidth = property(_openbox.MouseData_press_clientwidth_get,_openbox.MouseData_press_clientwidth_set)
    __swig_setmethods__["press_clientheight"] = _openbox.MouseData_press_clientheight_set
    __swig_getmethods__["press_clientheight"] = _openbox.MouseData_press_clientheight_get
    if _newclass:press_clientheight = property(_openbox.MouseData_press_clientheight_get,_openbox.MouseData_press_clientheight_set)
    def __init__(self,*args):
        self.this = apply(_openbox.new_MouseData,args)
        self.thisown = 1
    def __repr__(self):
        return "<C MouseData instance at %s>" % (self.this,)

class MouseDataPtr(MouseData):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = MouseData
_openbox.MouseData_swigregister(MouseDataPtr)

class EventData(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, EventData, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, EventData, name)
    __swig_setmethods__["screen"] = _openbox.EventData_screen_set
    __swig_getmethods__["screen"] = _openbox.EventData_screen_get
    if _newclass:screen = property(_openbox.EventData_screen_get,_openbox.EventData_screen_set)
    __swig_setmethods__["client"] = _openbox.EventData_client_set
    __swig_getmethods__["client"] = _openbox.EventData_client_get
    if _newclass:client = property(_openbox.EventData_client_get,_openbox.EventData_client_set)
    __swig_setmethods__["state"] = _openbox.EventData_state_set
    __swig_getmethods__["state"] = _openbox.EventData_state_get
    if _newclass:state = property(_openbox.EventData_state_get,_openbox.EventData_state_set)
    __swig_setmethods__["action"] = _openbox.EventData_action_set
    __swig_getmethods__["action"] = _openbox.EventData_action_get
    if _newclass:action = property(_openbox.EventData_action_get,_openbox.EventData_action_set)
    def __init__(self,*args):
        self.this = apply(_openbox.new_EventData,args)
        self.thisown = 1
    def __repr__(self):
        return "<C EventData instance at %s>" % (self.this,)

class EventDataPtr(EventData):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = EventData
_openbox.EventData_swigregister(EventDataPtr)

class KeyData(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, KeyData, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, KeyData, name)
    __swig_setmethods__["screen"] = _openbox.KeyData_screen_set
    __swig_getmethods__["screen"] = _openbox.KeyData_screen_get
    if _newclass:screen = property(_openbox.KeyData_screen_get,_openbox.KeyData_screen_set)
    __swig_setmethods__["client"] = _openbox.KeyData_client_set
    __swig_getmethods__["client"] = _openbox.KeyData_client_get
    if _newclass:client = property(_openbox.KeyData_client_get,_openbox.KeyData_client_set)
    __swig_setmethods__["time"] = _openbox.KeyData_time_set
    __swig_getmethods__["time"] = _openbox.KeyData_time_get
    if _newclass:time = property(_openbox.KeyData_time_get,_openbox.KeyData_time_set)
    __swig_setmethods__["state"] = _openbox.KeyData_state_set
    __swig_getmethods__["state"] = _openbox.KeyData_state_get
    if _newclass:state = property(_openbox.KeyData_state_get,_openbox.KeyData_state_set)
    __swig_setmethods__["key"] = _openbox.KeyData_key_set
    __swig_getmethods__["key"] = _openbox.KeyData_key_get
    if _newclass:key = property(_openbox.KeyData_key_get,_openbox.KeyData_key_set)
    def __init__(self,*args):
        self.this = apply(_openbox.new_KeyData,args)
        self.thisown = 1
    def __repr__(self):
        return "<C KeyData instance at %s>" % (self.this,)

class KeyDataPtr(KeyData):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = KeyData
_openbox.KeyData_swigregister(KeyDataPtr)

mbind = _openbox.mbind

kbind = _openbox.kbind

ebind = _openbox.ebind

set_reset_key = _openbox.set_reset_key

send_client_msg = _openbox.send_client_msg

X_PROTOCOL = _openbox.X_PROTOCOL
X_PROTOCOL_REVISION = _openbox.X_PROTOCOL_REVISION
None = _openbox.None
ParentRelative = _openbox.ParentRelative
CopyFromParent = _openbox.CopyFromParent
PointerWindow = _openbox.PointerWindow
InputFocus = _openbox.InputFocus
PointerRoot = _openbox.PointerRoot
AnyPropertyType = _openbox.AnyPropertyType
AnyKey = _openbox.AnyKey
AnyButton = _openbox.AnyButton
AllTemporary = _openbox.AllTemporary
CurrentTime = _openbox.CurrentTime
NoSymbol = _openbox.NoSymbol
NoEventMask = _openbox.NoEventMask
KeyPressMask = _openbox.KeyPressMask
KeyReleaseMask = _openbox.KeyReleaseMask
ButtonPressMask = _openbox.ButtonPressMask
ButtonReleaseMask = _openbox.ButtonReleaseMask
EnterWindowMask = _openbox.EnterWindowMask
LeaveWindowMask = _openbox.LeaveWindowMask
PointerMotionMask = _openbox.PointerMotionMask
PointerMotionHintMask = _openbox.PointerMotionHintMask
Button1MotionMask = _openbox.Button1MotionMask
Button2MotionMask = _openbox.Button2MotionMask
Button3MotionMask = _openbox.Button3MotionMask
Button4MotionMask = _openbox.Button4MotionMask
Button5MotionMask = _openbox.Button5MotionMask
ButtonMotionMask = _openbox.ButtonMotionMask
KeymapStateMask = _openbox.KeymapStateMask
ExposureMask = _openbox.ExposureMask
VisibilityChangeMask = _openbox.VisibilityChangeMask
StructureNotifyMask = _openbox.StructureNotifyMask
ResizeRedirectMask = _openbox.ResizeRedirectMask
SubstructureNotifyMask = _openbox.SubstructureNotifyMask
SubstructureRedirectMask = _openbox.SubstructureRedirectMask
FocusChangeMask = _openbox.FocusChangeMask
PropertyChangeMask = _openbox.PropertyChangeMask
ColormapChangeMask = _openbox.ColormapChangeMask
OwnerGrabButtonMask = _openbox.OwnerGrabButtonMask
KeyPress = _openbox.KeyPress
KeyRelease = _openbox.KeyRelease
ButtonPress = _openbox.ButtonPress
ButtonRelease = _openbox.ButtonRelease
MotionNotify = _openbox.MotionNotify
EnterNotify = _openbox.EnterNotify
LeaveNotify = _openbox.LeaveNotify
FocusIn = _openbox.FocusIn
FocusOut = _openbox.FocusOut
KeymapNotify = _openbox.KeymapNotify
Expose = _openbox.Expose
GraphicsExpose = _openbox.GraphicsExpose
NoExpose = _openbox.NoExpose
VisibilityNotify = _openbox.VisibilityNotify
CreateNotify = _openbox.CreateNotify
DestroyNotify = _openbox.DestroyNotify
UnmapNotify = _openbox.UnmapNotify
MapNotify = _openbox.MapNotify
MapRequest = _openbox.MapRequest
ReparentNotify = _openbox.ReparentNotify
ConfigureNotify = _openbox.ConfigureNotify
ConfigureRequest = _openbox.ConfigureRequest
GravityNotify = _openbox.GravityNotify
ResizeRequest = _openbox.ResizeRequest
CirculateNotify = _openbox.CirculateNotify
CirculateRequest = _openbox.CirculateRequest
PropertyNotify = _openbox.PropertyNotify
SelectionClear = _openbox.SelectionClear
SelectionRequest = _openbox.SelectionRequest
SelectionNotify = _openbox.SelectionNotify
ColormapNotify = _openbox.ColormapNotify
ClientMessage = _openbox.ClientMessage
MappingNotify = _openbox.MappingNotify
LASTEvent = _openbox.LASTEvent
ShiftMask = _openbox.ShiftMask
LockMask = _openbox.LockMask
ControlMask = _openbox.ControlMask
Mod1Mask = _openbox.Mod1Mask
Mod2Mask = _openbox.Mod2Mask
Mod3Mask = _openbox.Mod3Mask
Mod4Mask = _openbox.Mod4Mask
Mod5Mask = _openbox.Mod5Mask
ShiftMapIndex = _openbox.ShiftMapIndex
LockMapIndex = _openbox.LockMapIndex
ControlMapIndex = _openbox.ControlMapIndex
Mod1MapIndex = _openbox.Mod1MapIndex
Mod2MapIndex = _openbox.Mod2MapIndex
Mod3MapIndex = _openbox.Mod3MapIndex
Mod4MapIndex = _openbox.Mod4MapIndex
Mod5MapIndex = _openbox.Mod5MapIndex
Button1Mask = _openbox.Button1Mask
Button2Mask = _openbox.Button2Mask
Button3Mask = _openbox.Button3Mask
Button4Mask = _openbox.Button4Mask
Button5Mask = _openbox.Button5Mask
AnyModifier = _openbox.AnyModifier
Button1 = _openbox.Button1
Button2 = _openbox.Button2
Button3 = _openbox.Button3
Button4 = _openbox.Button4
Button5 = _openbox.Button5
NotifyNormal = _openbox.NotifyNormal
NotifyGrab = _openbox.NotifyGrab
NotifyUngrab = _openbox.NotifyUngrab
NotifyWhileGrabbed = _openbox.NotifyWhileGrabbed
NotifyHint = _openbox.NotifyHint
NotifyAncestor = _openbox.NotifyAncestor
NotifyVirtual = _openbox.NotifyVirtual
NotifyInferior = _openbox.NotifyInferior
NotifyNonlinear = _openbox.NotifyNonlinear
NotifyNonlinearVirtual = _openbox.NotifyNonlinearVirtual
NotifyPointer = _openbox.NotifyPointer
NotifyPointerRoot = _openbox.NotifyPointerRoot
NotifyDetailNone = _openbox.NotifyDetailNone
VisibilityUnobscured = _openbox.VisibilityUnobscured
VisibilityPartiallyObscured = _openbox.VisibilityPartiallyObscured
VisibilityFullyObscured = _openbox.VisibilityFullyObscured
PlaceOnTop = _openbox.PlaceOnTop
PlaceOnBottom = _openbox.PlaceOnBottom
FamilyInternet = _openbox.FamilyInternet
FamilyDECnet = _openbox.FamilyDECnet
FamilyChaos = _openbox.FamilyChaos
PropertyNewValue = _openbox.PropertyNewValue
PropertyDelete = _openbox.PropertyDelete
ColormapUninstalled = _openbox.ColormapUninstalled
ColormapInstalled = _openbox.ColormapInstalled
GrabModeSync = _openbox.GrabModeSync
GrabModeAsync = _openbox.GrabModeAsync
GrabSuccess = _openbox.GrabSuccess
AlreadyGrabbed = _openbox.AlreadyGrabbed
GrabInvalidTime = _openbox.GrabInvalidTime
GrabNotViewable = _openbox.GrabNotViewable
GrabFrozen = _openbox.GrabFrozen
AsyncPointer = _openbox.AsyncPointer
SyncPointer = _openbox.SyncPointer
ReplayPointer = _openbox.ReplayPointer
AsyncKeyboard = _openbox.AsyncKeyboard
SyncKeyboard = _openbox.SyncKeyboard
ReplayKeyboard = _openbox.ReplayKeyboard
AsyncBoth = _openbox.AsyncBoth
SyncBoth = _openbox.SyncBoth
RevertToParent = _openbox.RevertToParent
Success = _openbox.Success
BadRequest = _openbox.BadRequest
BadValue = _openbox.BadValue
BadWindow = _openbox.BadWindow
BadPixmap = _openbox.BadPixmap
BadAtom = _openbox.BadAtom
BadCursor = _openbox.BadCursor
BadFont = _openbox.BadFont
BadMatch = _openbox.BadMatch
BadDrawable = _openbox.BadDrawable
BadAccess = _openbox.BadAccess
BadAlloc = _openbox.BadAlloc
BadColor = _openbox.BadColor
BadGC = _openbox.BadGC
BadIDChoice = _openbox.BadIDChoice
BadName = _openbox.BadName
BadLength = _openbox.BadLength
BadImplementation = _openbox.BadImplementation
FirstExtensionError = _openbox.FirstExtensionError
LastExtensionError = _openbox.LastExtensionError
InputOutput = _openbox.InputOutput
InputOnly = _openbox.InputOnly
CWBackPixmap = _openbox.CWBackPixmap
CWBackPixel = _openbox.CWBackPixel
CWBorderPixmap = _openbox.CWBorderPixmap
CWBorderPixel = _openbox.CWBorderPixel
CWBitGravity = _openbox.CWBitGravity
CWWinGravity = _openbox.CWWinGravity
CWBackingStore = _openbox.CWBackingStore
CWBackingPlanes = _openbox.CWBackingPlanes
CWBackingPixel = _openbox.CWBackingPixel
CWOverrideRedirect = _openbox.CWOverrideRedirect
CWSaveUnder = _openbox.CWSaveUnder
CWEventMask = _openbox.CWEventMask
CWDontPropagate = _openbox.CWDontPropagate
CWColormap = _openbox.CWColormap
CWCursor = _openbox.CWCursor
CWX = _openbox.CWX
CWY = _openbox.CWY
CWWidth = _openbox.CWWidth
CWHeight = _openbox.CWHeight
CWBorderWidth = _openbox.CWBorderWidth
CWSibling = _openbox.CWSibling
CWStackMode = _openbox.CWStackMode
ForgetGravity = _openbox.ForgetGravity
NorthWestGravity = _openbox.NorthWestGravity
NorthGravity = _openbox.NorthGravity
NorthEastGravity = _openbox.NorthEastGravity
WestGravity = _openbox.WestGravity
CenterGravity = _openbox.CenterGravity
EastGravity = _openbox.EastGravity
SouthWestGravity = _openbox.SouthWestGravity
SouthGravity = _openbox.SouthGravity
SouthEastGravity = _openbox.SouthEastGravity
StaticGravity = _openbox.StaticGravity
UnmapGravity = _openbox.UnmapGravity
NotUseful = _openbox.NotUseful
WhenMapped = _openbox.WhenMapped
Always = _openbox.Always
IsUnmapped = _openbox.IsUnmapped
IsUnviewable = _openbox.IsUnviewable
IsViewable = _openbox.IsViewable
SetModeInsert = _openbox.SetModeInsert
SetModeDelete = _openbox.SetModeDelete
DestroyAll = _openbox.DestroyAll
RetainPermanent = _openbox.RetainPermanent
RetainTemporary = _openbox.RetainTemporary
Above = _openbox.Above
Below = _openbox.Below
TopIf = _openbox.TopIf
BottomIf = _openbox.BottomIf
Opposite = _openbox.Opposite
RaiseLowest = _openbox.RaiseLowest
LowerHighest = _openbox.LowerHighest
PropModeReplace = _openbox.PropModeReplace
PropModePrepend = _openbox.PropModePrepend
PropModeAppend = _openbox.PropModeAppend
GXclear = _openbox.GXclear
GXand = _openbox.GXand
GXandReverse = _openbox.GXandReverse
GXcopy = _openbox.GXcopy
GXandInverted = _openbox.GXandInverted
GXnoop = _openbox.GXnoop
GXxor = _openbox.GXxor
GXor = _openbox.GXor
GXnor = _openbox.GXnor
GXequiv = _openbox.GXequiv
GXinvert = _openbox.GXinvert
GXorReverse = _openbox.GXorReverse
GXcopyInverted = _openbox.GXcopyInverted
GXorInverted = _openbox.GXorInverted
GXnand = _openbox.GXnand
GXset = _openbox.GXset
LineSolid = _openbox.LineSolid
LineOnOffDash = _openbox.LineOnOffDash
LineDoubleDash = _openbox.LineDoubleDash
CapNotLast = _openbox.CapNotLast
CapButt = _openbox.CapButt
CapRound = _openbox.CapRound
CapProjecting = _openbox.CapProjecting
JoinMiter = _openbox.JoinMiter
JoinRound = _openbox.JoinRound
JoinBevel = _openbox.JoinBevel
FillSolid = _openbox.FillSolid
FillTiled = _openbox.FillTiled
FillStippled = _openbox.FillStippled
FillOpaqueStippled = _openbox.FillOpaqueStippled
EvenOddRule = _openbox.EvenOddRule
WindingRule = _openbox.WindingRule
ClipByChildren = _openbox.ClipByChildren
IncludeInferiors = _openbox.IncludeInferiors
Unsorted = _openbox.Unsorted
YSorted = _openbox.YSorted
YXSorted = _openbox.YXSorted
YXBanded = _openbox.YXBanded
CoordModeOrigin = _openbox.CoordModeOrigin
CoordModePrevious = _openbox.CoordModePrevious
Complex = _openbox.Complex
Nonconvex = _openbox.Nonconvex
Convex = _openbox.Convex
ArcChord = _openbox.ArcChord
ArcPieSlice = _openbox.ArcPieSlice
GCFunction = _openbox.GCFunction
GCPlaneMask = _openbox.GCPlaneMask
GCForeground = _openbox.GCForeground
GCBackground = _openbox.GCBackground
GCLineWidth = _openbox.GCLineWidth
GCLineStyle = _openbox.GCLineStyle
GCCapStyle = _openbox.GCCapStyle
GCJoinStyle = _openbox.GCJoinStyle
GCFillStyle = _openbox.GCFillStyle
GCFillRule = _openbox.GCFillRule
GCTile = _openbox.GCTile
GCStipple = _openbox.GCStipple
GCTileStipXOrigin = _openbox.GCTileStipXOrigin
GCTileStipYOrigin = _openbox.GCTileStipYOrigin
GCFont = _openbox.GCFont
GCSubwindowMode = _openbox.GCSubwindowMode
GCGraphicsExposures = _openbox.GCGraphicsExposures
GCClipXOrigin = _openbox.GCClipXOrigin
GCClipYOrigin = _openbox.GCClipYOrigin
GCClipMask = _openbox.GCClipMask
GCDashOffset = _openbox.GCDashOffset
GCDashList = _openbox.GCDashList
GCArcMode = _openbox.GCArcMode
GCLastBit = _openbox.GCLastBit
FontLeftToRight = _openbox.FontLeftToRight
FontRightToLeft = _openbox.FontRightToLeft
FontChange = _openbox.FontChange
XYBitmap = _openbox.XYBitmap
XYPixmap = _openbox.XYPixmap
ZPixmap = _openbox.ZPixmap
AllocNone = _openbox.AllocNone
AllocAll = _openbox.AllocAll
DoRed = _openbox.DoRed
DoGreen = _openbox.DoGreen
DoBlue = _openbox.DoBlue
CursorShape = _openbox.CursorShape
TileShape = _openbox.TileShape
StippleShape = _openbox.StippleShape
AutoRepeatModeOff = _openbox.AutoRepeatModeOff
AutoRepeatModeOn = _openbox.AutoRepeatModeOn
AutoRepeatModeDefault = _openbox.AutoRepeatModeDefault
LedModeOff = _openbox.LedModeOff
LedModeOn = _openbox.LedModeOn
KBKeyClickPercent = _openbox.KBKeyClickPercent
KBBellPercent = _openbox.KBBellPercent
KBBellPitch = _openbox.KBBellPitch
KBBellDuration = _openbox.KBBellDuration
KBLed = _openbox.KBLed
KBLedMode = _openbox.KBLedMode
KBKey = _openbox.KBKey
KBAutoRepeatMode = _openbox.KBAutoRepeatMode
MappingSuccess = _openbox.MappingSuccess
MappingBusy = _openbox.MappingBusy
MappingFailed = _openbox.MappingFailed
MappingModifier = _openbox.MappingModifier
MappingKeyboard = _openbox.MappingKeyboard
MappingPointer = _openbox.MappingPointer
DontPreferBlanking = _openbox.DontPreferBlanking
PreferBlanking = _openbox.PreferBlanking
DefaultBlanking = _openbox.DefaultBlanking
DisableScreenSaver = _openbox.DisableScreenSaver
DisableScreenInterval = _openbox.DisableScreenInterval
DontAllowExposures = _openbox.DontAllowExposures
AllowExposures = _openbox.AllowExposures
DefaultExposures = _openbox.DefaultExposures
ScreenSaverReset = _openbox.ScreenSaverReset
ScreenSaverActive = _openbox.ScreenSaverActive
HostInsert = _openbox.HostInsert
HostDelete = _openbox.HostDelete
EnableAccess = _openbox.EnableAccess
DisableAccess = _openbox.DisableAccess
StaticGray = _openbox.StaticGray
GrayScale = _openbox.GrayScale
StaticColor = _openbox.StaticColor
PseudoColor = _openbox.PseudoColor
TrueColor = _openbox.TrueColor
DirectColor = _openbox.DirectColor
LSBFirst = _openbox.LSBFirst
MSBFirst = _openbox.MSBFirst

