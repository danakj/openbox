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

OBDisplay_display = _openbox.OBDisplay_display

class OtkEventDispatcher(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, OtkEventDispatcher, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, OtkEventDispatcher, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_OtkEventDispatcher,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_OtkEventDispatcher):
        try:
            if self.thisown: destroy(self)
        except: pass
    def clearAllHandlers(*args): return apply(_openbox.OtkEventDispatcher_clearAllHandlers,args)
    def registerHandler(*args): return apply(_openbox.OtkEventDispatcher_registerHandler,args)
    def clearHandler(*args): return apply(_openbox.OtkEventDispatcher_clearHandler,args)
    def dispatchEvents(*args): return apply(_openbox.OtkEventDispatcher_dispatchEvents,args)
    def setFallbackHandler(*args): return apply(_openbox.OtkEventDispatcher_setFallbackHandler,args)
    def getFallbackHandler(*args): return apply(_openbox.OtkEventDispatcher_getFallbackHandler,args)
    def setMasterHandler(*args): return apply(_openbox.OtkEventDispatcher_setMasterHandler,args)
    def getMasterHandler(*args): return apply(_openbox.OtkEventDispatcher_getMasterHandler,args)
    def findHandler(*args): return apply(_openbox.OtkEventDispatcher_findHandler,args)
    def lastTime(*args): return apply(_openbox.OtkEventDispatcher_lastTime,args)
    def __repr__(self):
        return "<C OtkEventDispatcher instance at %s>" % (self.this,)

class OtkEventDispatcherPtr(OtkEventDispatcher):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OtkEventDispatcher
_openbox.OtkEventDispatcher_swigregister(OtkEventDispatcherPtr)

class OtkEventHandler(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, OtkEventHandler, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, OtkEventHandler, name)
    def handle(*args): return apply(_openbox.OtkEventHandler_handle,args)
    def keyPressHandler(*args): return apply(_openbox.OtkEventHandler_keyPressHandler,args)
    def keyReleaseHandler(*args): return apply(_openbox.OtkEventHandler_keyReleaseHandler,args)
    def buttonPressHandler(*args): return apply(_openbox.OtkEventHandler_buttonPressHandler,args)
    def buttonReleaseHandler(*args): return apply(_openbox.OtkEventHandler_buttonReleaseHandler,args)
    def motionHandler(*args): return apply(_openbox.OtkEventHandler_motionHandler,args)
    def enterHandler(*args): return apply(_openbox.OtkEventHandler_enterHandler,args)
    def leaveHandler(*args): return apply(_openbox.OtkEventHandler_leaveHandler,args)
    def focusHandler(*args): return apply(_openbox.OtkEventHandler_focusHandler,args)
    def unfocusHandler(*args): return apply(_openbox.OtkEventHandler_unfocusHandler,args)
    def exposeHandler(*args): return apply(_openbox.OtkEventHandler_exposeHandler,args)
    def graphicsExposeHandler(*args): return apply(_openbox.OtkEventHandler_graphicsExposeHandler,args)
    def noExposeEventHandler(*args): return apply(_openbox.OtkEventHandler_noExposeEventHandler,args)
    def circulateRequestHandler(*args): return apply(_openbox.OtkEventHandler_circulateRequestHandler,args)
    def configureRequestHandler(*args): return apply(_openbox.OtkEventHandler_configureRequestHandler,args)
    def mapRequestHandler(*args): return apply(_openbox.OtkEventHandler_mapRequestHandler,args)
    def resizeRequestHandler(*args): return apply(_openbox.OtkEventHandler_resizeRequestHandler,args)
    def circulateHandler(*args): return apply(_openbox.OtkEventHandler_circulateHandler,args)
    def configureHandler(*args): return apply(_openbox.OtkEventHandler_configureHandler,args)
    def createHandler(*args): return apply(_openbox.OtkEventHandler_createHandler,args)
    def destroyHandler(*args): return apply(_openbox.OtkEventHandler_destroyHandler,args)
    def gravityHandler(*args): return apply(_openbox.OtkEventHandler_gravityHandler,args)
    def mapHandler(*args): return apply(_openbox.OtkEventHandler_mapHandler,args)
    def mappingHandler(*args): return apply(_openbox.OtkEventHandler_mappingHandler,args)
    def reparentHandler(*args): return apply(_openbox.OtkEventHandler_reparentHandler,args)
    def unmapHandler(*args): return apply(_openbox.OtkEventHandler_unmapHandler,args)
    def visibilityHandler(*args): return apply(_openbox.OtkEventHandler_visibilityHandler,args)
    def colorMapHandler(*args): return apply(_openbox.OtkEventHandler_colorMapHandler,args)
    def propertyHandler(*args): return apply(_openbox.OtkEventHandler_propertyHandler,args)
    def selectionClearHandler(*args): return apply(_openbox.OtkEventHandler_selectionClearHandler,args)
    def selectionHandler(*args): return apply(_openbox.OtkEventHandler_selectionHandler,args)
    def selectionRequestHandler(*args): return apply(_openbox.OtkEventHandler_selectionRequestHandler,args)
    def clientMessageHandler(*args): return apply(_openbox.OtkEventHandler_clientMessageHandler,args)
    def __del__(self, destroy= _openbox.delete_OtkEventHandler):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C OtkEventHandler instance at %s>" % (self.this,)

class OtkEventHandlerPtr(OtkEventHandler):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OtkEventHandler
_openbox.OtkEventHandler_swigregister(OtkEventHandlerPtr)

class OtkWidget(OtkEventHandler):
    __swig_setmethods__ = {}
    for _s in [OtkEventHandler]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, OtkWidget, name, value)
    __swig_getmethods__ = {}
    for _s in [OtkEventHandler]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, OtkWidget, name)
    Horizontal = _openbox.OtkWidget_Horizontal
    Vertical = _openbox.OtkWidget_Vertical
    def __init__(self,*args):
        self.this = apply(_openbox.new_OtkWidget,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_OtkWidget):
        try:
            if self.thisown: destroy(self)
        except: pass
    def update(*args): return apply(_openbox.OtkWidget_update,args)
    def exposeHandler(*args): return apply(_openbox.OtkWidget_exposeHandler,args)
    def configureHandler(*args): return apply(_openbox.OtkWidget_configureHandler,args)
    def window(*args): return apply(_openbox.OtkWidget_window,args)
    def parent(*args): return apply(_openbox.OtkWidget_parent,args)
    def children(*args): return apply(_openbox.OtkWidget_children,args)
    def screen(*args): return apply(_openbox.OtkWidget_screen,args)
    def rect(*args): return apply(_openbox.OtkWidget_rect,args)
    def move(*args): return apply(_openbox.OtkWidget_move,args)
    def setWidth(*args): return apply(_openbox.OtkWidget_setWidth,args)
    def setHeight(*args): return apply(_openbox.OtkWidget_setHeight,args)
    def width(*args): return apply(_openbox.OtkWidget_width,args)
    def height(*args): return apply(_openbox.OtkWidget_height,args)
    def resize(*args): return apply(_openbox.OtkWidget_resize,args)
    def setGeometry(*args): return apply(_openbox.OtkWidget_setGeometry,args)
    def isVisible(*args): return apply(_openbox.OtkWidget_isVisible,args)
    def show(*args): return apply(_openbox.OtkWidget_show,args)
    def hide(*args): return apply(_openbox.OtkWidget_hide,args)
    def isFocused(*args): return apply(_openbox.OtkWidget_isFocused,args)
    def focus(*args): return apply(_openbox.OtkWidget_focus,args)
    def unfocus(*args): return apply(_openbox.OtkWidget_unfocus,args)
    def hasGrabbedMouse(*args): return apply(_openbox.OtkWidget_hasGrabbedMouse,args)
    def grabMouse(*args): return apply(_openbox.OtkWidget_grabMouse,args)
    def ungrabMouse(*args): return apply(_openbox.OtkWidget_ungrabMouse,args)
    def hasGrabbedKeyboard(*args): return apply(_openbox.OtkWidget_hasGrabbedKeyboard,args)
    def grabKeyboard(*args): return apply(_openbox.OtkWidget_grabKeyboard,args)
    def ungrabKeyboard(*args): return apply(_openbox.OtkWidget_ungrabKeyboard,args)
    def texture(*args): return apply(_openbox.OtkWidget_texture,args)
    def setTexture(*args): return apply(_openbox.OtkWidget_setTexture,args)
    def borderColor(*args): return apply(_openbox.OtkWidget_borderColor,args)
    def setBorderColor(*args): return apply(_openbox.OtkWidget_setBorderColor,args)
    def borderWidth(*args): return apply(_openbox.OtkWidget_borderWidth,args)
    def setBorderWidth(*args): return apply(_openbox.OtkWidget_setBorderWidth,args)
    def addChild(*args): return apply(_openbox.OtkWidget_addChild,args)
    def removeChild(*args): return apply(_openbox.OtkWidget_removeChild,args)
    def isStretchableHorz(*args): return apply(_openbox.OtkWidget_isStretchableHorz,args)
    def setStretchableHorz(*args): return apply(_openbox.OtkWidget_setStretchableHorz,args)
    def isStretchableVert(*args): return apply(_openbox.OtkWidget_isStretchableVert,args)
    def setStretchableVert(*args): return apply(_openbox.OtkWidget_setStretchableVert,args)
    def cursor(*args): return apply(_openbox.OtkWidget_cursor,args)
    def setCursor(*args): return apply(_openbox.OtkWidget_setCursor,args)
    def bevelWidth(*args): return apply(_openbox.OtkWidget_bevelWidth,args)
    def setBevelWidth(*args): return apply(_openbox.OtkWidget_setBevelWidth,args)
    def direction(*args): return apply(_openbox.OtkWidget_direction,args)
    def setDirection(*args): return apply(_openbox.OtkWidget_setDirection,args)
    def style(*args): return apply(_openbox.OtkWidget_style,args)
    def setStyle(*args): return apply(_openbox.OtkWidget_setStyle,args)
    def eventDispatcher(*args): return apply(_openbox.OtkWidget_eventDispatcher,args)
    def setEventDispatcher(*args): return apply(_openbox.OtkWidget_setEventDispatcher,args)
    def __repr__(self):
        return "<C OtkWidget instance at %s>" % (self.this,)

class OtkWidgetPtr(OtkWidget):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OtkWidget
_openbox.OtkWidget_swigregister(OtkWidgetPtr)

class OtkFocusWidget(OtkWidget):
    __swig_setmethods__ = {}
    for _s in [OtkWidget]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, OtkFocusWidget, name, value)
    __swig_getmethods__ = {}
    for _s in [OtkWidget]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, OtkFocusWidget, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_OtkFocusWidget,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_OtkFocusWidget):
        try:
            if self.thisown: destroy(self)
        except: pass
    def focus(*args): return apply(_openbox.OtkFocusWidget_focus,args)
    def unfocus(*args): return apply(_openbox.OtkFocusWidget_unfocus,args)
    def setTexture(*args): return apply(_openbox.OtkFocusWidget_setTexture,args)
    def setBorderColor(*args): return apply(_openbox.OtkFocusWidget_setBorderColor,args)
    def setUnfocusTexture(*args): return apply(_openbox.OtkFocusWidget_setUnfocusTexture,args)
    def getUnfocusTexture(*args): return apply(_openbox.OtkFocusWidget_getUnfocusTexture,args)
    def setUnfocusBorderColor(*args): return apply(_openbox.OtkFocusWidget_setUnfocusBorderColor,args)
    def getUnfocusBorderColor(*args): return apply(_openbox.OtkFocusWidget_getUnfocusBorderColor,args)
    def isFocused(*args): return apply(_openbox.OtkFocusWidget_isFocused,args)
    def isUnfocused(*args): return apply(_openbox.OtkFocusWidget_isUnfocused,args)
    def __repr__(self):
        return "<C OtkFocusWidget instance at %s>" % (self.this,)

class OtkFocusWidgetPtr(OtkFocusWidget):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OtkFocusWidget
_openbox.OtkFocusWidget_swigregister(OtkFocusWidgetPtr)

class OtkFocusLabel(OtkFocusWidget):
    __swig_setmethods__ = {}
    for _s in [OtkFocusWidget]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, OtkFocusLabel, name, value)
    __swig_getmethods__ = {}
    for _s in [OtkFocusWidget]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, OtkFocusLabel, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_OtkFocusLabel,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_OtkFocusLabel):
        try:
            if self.thisown: destroy(self)
        except: pass
    def getText(*args): return apply(_openbox.OtkFocusLabel_getText,args)
    def setText(*args): return apply(_openbox.OtkFocusLabel_setText,args)
    def update(*args): return apply(_openbox.OtkFocusLabel_update,args)
    def setStyle(*args): return apply(_openbox.OtkFocusLabel_setStyle,args)
    def __repr__(self):
        return "<C OtkFocusLabel instance at %s>" % (self.this,)

class OtkFocusLabelPtr(OtkFocusLabel):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OtkFocusLabel
_openbox.OtkFocusLabel_swigregister(OtkFocusLabelPtr)

class OtkAppWidget(OtkWidget):
    __swig_setmethods__ = {}
    for _s in [OtkWidget]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, OtkAppWidget, name, value)
    __swig_getmethods__ = {}
    for _s in [OtkWidget]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, OtkAppWidget, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_OtkAppWidget,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_OtkAppWidget):
        try:
            if self.thisown: destroy(self)
        except: pass
    def show(*args): return apply(_openbox.OtkAppWidget_show,args)
    def hide(*args): return apply(_openbox.OtkAppWidget_hide,args)
    def clientMessageHandler(*args): return apply(_openbox.OtkAppWidget_clientMessageHandler,args)
    def __repr__(self):
        return "<C OtkAppWidget instance at %s>" % (self.this,)

class OtkAppWidgetPtr(OtkAppWidget):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OtkAppWidget
_openbox.OtkAppWidget_swigregister(OtkAppWidgetPtr)

class OtkApplication(OtkEventDispatcher):
    __swig_setmethods__ = {}
    for _s in [OtkEventDispatcher]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, OtkApplication, name, value)
    __swig_getmethods__ = {}
    for _s in [OtkEventDispatcher]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, OtkApplication, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_OtkApplication,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_OtkApplication):
        try:
            if self.thisown: destroy(self)
        except: pass
    def run(*args): return apply(_openbox.OtkApplication_run,args)
    def setDockable(*args): return apply(_openbox.OtkApplication_setDockable,args)
    def isDockable(*args): return apply(_openbox.OtkApplication_isDockable,args)
    def getStyle(*args): return apply(_openbox.OtkApplication_getStyle,args)
    def __repr__(self):
        return "<C OtkApplication instance at %s>" % (self.this,)

class OtkApplicationPtr(OtkApplication):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OtkApplication
_openbox.OtkApplication_swigregister(OtkApplicationPtr)

class PointerAssassin(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, PointerAssassin, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, PointerAssassin, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C PointerAssassin instance at %s>" % (self.this,)

class PointerAssassinPtr(PointerAssassin):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = PointerAssassin
_openbox.PointerAssassin_swigregister(PointerAssassinPtr)

class OtkButton(OtkFocusLabel):
    __swig_setmethods__ = {}
    for _s in [OtkFocusLabel]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, OtkButton, name, value)
    __swig_getmethods__ = {}
    for _s in [OtkFocusLabel]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, OtkButton, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_OtkButton,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_OtkButton):
        try:
            if self.thisown: destroy(self)
        except: pass
    def getPressedFocusTexture(*args): return apply(_openbox.OtkButton_getPressedFocusTexture,args)
    def setPressedFocusTexture(*args): return apply(_openbox.OtkButton_setPressedFocusTexture,args)
    def getPressedUnfocusTexture(*args): return apply(_openbox.OtkButton_getPressedUnfocusTexture,args)
    def setPressedUnfocusTexture(*args): return apply(_openbox.OtkButton_setPressedUnfocusTexture,args)
    def setTexture(*args): return apply(_openbox.OtkButton_setTexture,args)
    def setUnfocusTexture(*args): return apply(_openbox.OtkButton_setUnfocusTexture,args)
    def isPressed(*args): return apply(_openbox.OtkButton_isPressed,args)
    def press(*args): return apply(_openbox.OtkButton_press,args)
    def release(*args): return apply(_openbox.OtkButton_release,args)
    def buttonPressHandler(*args): return apply(_openbox.OtkButton_buttonPressHandler,args)
    def buttonReleaseHandler(*args): return apply(_openbox.OtkButton_buttonReleaseHandler,args)
    def setStyle(*args): return apply(_openbox.OtkButton_setStyle,args)
    def __repr__(self):
        return "<C OtkButton instance at %s>" % (self.this,)

class OtkButtonPtr(OtkButton):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OtkButton
_openbox.OtkButton_swigregister(OtkButtonPtr)

class BColor(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BColor, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BColor, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_BColor,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_BColor):
        try:
            if self.thisown: destroy(self)
        except: pass
    def name(*args): return apply(_openbox.BColor_name,args)
    def red(*args): return apply(_openbox.BColor_red,args)
    def green(*args): return apply(_openbox.BColor_green,args)
    def blue(*args): return apply(_openbox.BColor_blue,args)
    def setRGB(*args): return apply(_openbox.BColor_setRGB,args)
    def screen(*args): return apply(_openbox.BColor_screen,args)
    def setScreen(*args): return apply(_openbox.BColor_setScreen,args)
    def isAllocated(*args): return apply(_openbox.BColor_isAllocated,args)
    def isValid(*args): return apply(_openbox.BColor_isValid,args)
    def pixel(*args): return apply(_openbox.BColor_pixel,args)
    def equals(*args): return apply(_openbox.BColor_equals,args)
    __swig_getmethods__["cleanupColorCache"] = lambda x: _openbox.BColor_cleanupColorCache
    if _newclass:cleanupColorCache = staticmethod(_openbox.BColor_cleanupColorCache)
    def __repr__(self):
        return "<C BColor instance at %s>" % (self.this,)

class BColorPtr(BColor):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BColor
_openbox.BColor_swigregister(BColorPtr)
BColor_cleanupColorCache = _openbox.BColor_cleanupColorCache


class Configuration(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Configuration, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Configuration, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_Configuration,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_Configuration):
        try:
            if self.thisown: destroy(self)
        except: pass
    def file(*args): return apply(_openbox.Configuration_file,args)
    def setFile(*args): return apply(_openbox.Configuration_setFile,args)
    def autoSave(*args): return apply(_openbox.Configuration_autoSave,args)
    def setAutoSave(*args): return apply(_openbox.Configuration_setAutoSave,args)
    def isModified(*args): return apply(_openbox.Configuration_isModified,args)
    def save(*args): return apply(_openbox.Configuration_save,args)
    def load(*args): return apply(_openbox.Configuration_load,args)
    def merge(*args): return apply(_openbox.Configuration_merge,args)
    def create(*args): return apply(_openbox.Configuration_create,args)
    def setValue_bool(*args): return apply(_openbox.Configuration_setValue_bool,args)
    def setValue(*args): return apply(_openbox.Configuration_setValue,args)
    def setValue_unsigned(*args): return apply(_openbox.Configuration_setValue_unsigned,args)
    def setValue_long(*args): return apply(_openbox.Configuration_setValue_long,args)
    def setValue_unsignedlong(*args): return apply(_openbox.Configuration_setValue_unsignedlong,args)
    def setValue_string(*args): return apply(_openbox.Configuration_setValue_string,args)
    def setValue_charptr(*args): return apply(_openbox.Configuration_setValue_charptr,args)
    def getValue(*args): return apply(_openbox.Configuration_getValue,args)
    def __repr__(self):
        return "<C Configuration instance at %s>" % (self.this,)

class ConfigurationPtr(Configuration):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Configuration
_openbox.Configuration_swigregister(ConfigurationPtr)

class OBDisplay(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, OBDisplay, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, OBDisplay, name)
    __swig_getmethods__["initialize"] = lambda x: _openbox.OBDisplay_initialize
    if _newclass:initialize = staticmethod(_openbox.OBDisplay_initialize)
    __swig_getmethods__["destroy"] = lambda x: _openbox.OBDisplay_destroy
    if _newclass:destroy = staticmethod(_openbox.OBDisplay_destroy)
    __swig_getmethods__["gcCache"] = lambda x: _openbox.OBDisplay_gcCache
    if _newclass:gcCache = staticmethod(_openbox.OBDisplay_gcCache)
    __swig_getmethods__["screenInfo"] = lambda x: _openbox.OBDisplay_screenInfo
    if _newclass:screenInfo = staticmethod(_openbox.OBDisplay_screenInfo)
    __swig_getmethods__["findScreen"] = lambda x: _openbox.OBDisplay_findScreen
    if _newclass:findScreen = staticmethod(_openbox.OBDisplay_findScreen)
    __swig_getmethods__["xkb"] = lambda x: _openbox.OBDisplay_xkb
    if _newclass:xkb = staticmethod(_openbox.OBDisplay_xkb)
    __swig_getmethods__["xkbEventBase"] = lambda x: _openbox.OBDisplay_xkbEventBase
    if _newclass:xkbEventBase = staticmethod(_openbox.OBDisplay_xkbEventBase)
    __swig_getmethods__["shape"] = lambda x: _openbox.OBDisplay_shape
    if _newclass:shape = staticmethod(_openbox.OBDisplay_shape)
    __swig_getmethods__["shapeEventBase"] = lambda x: _openbox.OBDisplay_shapeEventBase
    if _newclass:shapeEventBase = staticmethod(_openbox.OBDisplay_shapeEventBase)
    __swig_getmethods__["xinerama"] = lambda x: _openbox.OBDisplay_xinerama
    if _newclass:xinerama = staticmethod(_openbox.OBDisplay_xinerama)
    __swig_getmethods__["numLockMask"] = lambda x: _openbox.OBDisplay_numLockMask
    if _newclass:numLockMask = staticmethod(_openbox.OBDisplay_numLockMask)
    __swig_getmethods__["scrollLockMask"] = lambda x: _openbox.OBDisplay_scrollLockMask
    if _newclass:scrollLockMask = staticmethod(_openbox.OBDisplay_scrollLockMask)
    __swig_getmethods__["grab"] = lambda x: _openbox.OBDisplay_grab
    if _newclass:grab = staticmethod(_openbox.OBDisplay_grab)
    __swig_getmethods__["ungrab"] = lambda x: _openbox.OBDisplay_ungrab
    if _newclass:ungrab = staticmethod(_openbox.OBDisplay_ungrab)
    __swig_getmethods__["grabButton"] = lambda x: _openbox.OBDisplay_grabButton
    if _newclass:grabButton = staticmethod(_openbox.OBDisplay_grabButton)
    __swig_getmethods__["ungrabButton"] = lambda x: _openbox.OBDisplay_ungrabButton
    if _newclass:ungrabButton = staticmethod(_openbox.OBDisplay_ungrabButton)
    __swig_getmethods__["grabKey"] = lambda x: _openbox.OBDisplay_grabKey
    if _newclass:grabKey = staticmethod(_openbox.OBDisplay_grabKey)
    __swig_getmethods__["ungrabKey"] = lambda x: _openbox.OBDisplay_ungrabKey
    if _newclass:ungrabKey = staticmethod(_openbox.OBDisplay_ungrabKey)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C OBDisplay instance at %s>" % (self.this,)

class OBDisplayPtr(OBDisplay):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OBDisplay
_openbox.OBDisplay_swigregister(OBDisplayPtr)
OBDisplay_initialize = _openbox.OBDisplay_initialize

OBDisplay_destroy = _openbox.OBDisplay_destroy

OBDisplay_gcCache = _openbox.OBDisplay_gcCache

OBDisplay_screenInfo = _openbox.OBDisplay_screenInfo

OBDisplay_findScreen = _openbox.OBDisplay_findScreen

OBDisplay_xkb = _openbox.OBDisplay_xkb

OBDisplay_xkbEventBase = _openbox.OBDisplay_xkbEventBase

OBDisplay_shape = _openbox.OBDisplay_shape

OBDisplay_shapeEventBase = _openbox.OBDisplay_shapeEventBase

OBDisplay_xinerama = _openbox.OBDisplay_xinerama

OBDisplay_numLockMask = _openbox.OBDisplay_numLockMask

OBDisplay_scrollLockMask = _openbox.OBDisplay_scrollLockMask

OBDisplay_grab = _openbox.OBDisplay_grab

OBDisplay_ungrab = _openbox.OBDisplay_ungrab

OBDisplay_grabButton = _openbox.OBDisplay_grabButton

OBDisplay_ungrabButton = _openbox.OBDisplay_ungrabButton

OBDisplay_grabKey = _openbox.OBDisplay_grabKey

OBDisplay_ungrabKey = _openbox.OBDisplay_ungrabKey


class BFont(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BFont, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BFont, name)
    __swig_getmethods__["fallbackFont"] = lambda x: _openbox.BFont_fallbackFont
    if _newclass:fallbackFont = staticmethod(_openbox.BFont_fallbackFont)
    __swig_getmethods__["setFallbackFont"] = lambda x: _openbox.BFont_setFallbackFont
    if _newclass:setFallbackFont = staticmethod(_openbox.BFont_setFallbackFont)
    def __init__(self,*args):
        self.this = apply(_openbox.new_BFont,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_BFont):
        try:
            if self.thisown: destroy(self)
        except: pass
    def fontstring(*args): return apply(_openbox.BFont_fontstring,args)
    def height(*args): return apply(_openbox.BFont_height,args)
    def maxCharWidth(*args): return apply(_openbox.BFont_maxCharWidth,args)
    def measureString(*args): return apply(_openbox.BFont_measureString,args)
    def drawString(*args): return apply(_openbox.BFont_drawString,args)
    def __repr__(self):
        return "<C BFont instance at %s>" % (self.this,)

class BFontPtr(BFont):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BFont
_openbox.BFont_swigregister(BFontPtr)
BFont_fallbackFont = _openbox.BFont_fallbackFont

BFont_setFallbackFont = _openbox.BFont_setFallbackFont


class BGCCacheContext(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BGCCacheContext, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BGCCacheContext, name)
    def set(*args): return apply(_openbox.BGCCacheContext_set,args)
    def __del__(self, destroy= _openbox.delete_BGCCacheContext):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C BGCCacheContext instance at %s>" % (self.this,)

class BGCCacheContextPtr(BGCCacheContext):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BGCCacheContext
_openbox.BGCCacheContext_swigregister(BGCCacheContextPtr)

class BGCCacheItem(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BGCCacheItem, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BGCCacheItem, name)
    def gc(*args): return apply(_openbox.BGCCacheItem_gc,args)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C BGCCacheItem instance at %s>" % (self.this,)

class BGCCacheItemPtr(BGCCacheItem):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BGCCacheItem
_openbox.BGCCacheItem_swigregister(BGCCacheItemPtr)

class BGCCache(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BGCCache, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BGCCache, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_BGCCache,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_BGCCache):
        try:
            if self.thisown: destroy(self)
        except: pass
    def purge(*args): return apply(_openbox.BGCCache_purge,args)
    def find(*args): return apply(_openbox.BGCCache_find,args)
    def release(*args): return apply(_openbox.BGCCache_release,args)
    def __repr__(self):
        return "<C BGCCache instance at %s>" % (self.this,)

class BGCCachePtr(BGCCache):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BGCCache
_openbox.BGCCache_swigregister(BGCCachePtr)

class BPen(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BPen, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BPen, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_BPen,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_BPen):
        try:
            if self.thisown: destroy(self)
        except: pass
    def gc(*args): return apply(_openbox.BPen_gc,args)
    def __repr__(self):
        return "<C BPen instance at %s>" % (self.this,)

class BPenPtr(BPen):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BPen
_openbox.BPen_swigregister(BPenPtr)

class BImage(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BImage, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BImage, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_BImage,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_BImage):
        try:
            if self.thisown: destroy(self)
        except: pass
    def render(*args): return apply(_openbox.BImage_render,args)
    def __repr__(self):
        return "<C BImage instance at %s>" % (self.this,)

class BImagePtr(BImage):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BImage
_openbox.BImage_swigregister(BImagePtr)

class BImageControl(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BImageControl, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BImageControl, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_BImageControl,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_BImageControl):
        try:
            if self.thisown: destroy(self)
        except: pass
    def doDither(*args): return apply(_openbox.BImageControl_doDither,args)
    def getScreenInfo(*args): return apply(_openbox.BImageControl_getScreenInfo,args)
    def getDrawable(*args): return apply(_openbox.BImageControl_getDrawable,args)
    def getVisual(*args): return apply(_openbox.BImageControl_getVisual,args)
    def getBitsPerPixel(*args): return apply(_openbox.BImageControl_getBitsPerPixel,args)
    def getDepth(*args): return apply(_openbox.BImageControl_getDepth,args)
    def getColorsPerChannel(*args): return apply(_openbox.BImageControl_getColorsPerChannel,args)
    def getSqrt(*args): return apply(_openbox.BImageControl_getSqrt,args)
    def renderImage(*args): return apply(_openbox.BImageControl_renderImage,args)
    def installRootColormap(*args): return apply(_openbox.BImageControl_installRootColormap,args)
    def removeImage(*args): return apply(_openbox.BImageControl_removeImage,args)
    def getColorTables(*args): return apply(_openbox.BImageControl_getColorTables,args)
    def getXColorTable(*args): return apply(_openbox.BImageControl_getXColorTable,args)
    def getGradientBuffers(*args): return apply(_openbox.BImageControl_getGradientBuffers,args)
    def setDither(*args): return apply(_openbox.BImageControl_setDither,args)
    def setColorsPerChannel(*args): return apply(_openbox.BImageControl_setColorsPerChannel,args)
    __swig_getmethods__["timeout"] = lambda x: _openbox.BImageControl_timeout
    if _newclass:timeout = staticmethod(_openbox.BImageControl_timeout)
    def __repr__(self):
        return "<C BImageControl instance at %s>" % (self.this,)

class BImageControlPtr(BImageControl):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BImageControl
_openbox.BImageControl_swigregister(BImageControlPtr)
BImageControl_timeout = _openbox.BImageControl_timeout


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

class OBProperty(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, OBProperty, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, OBProperty, name)
    Atom_Cardinal = _openbox.OBProperty_Atom_Cardinal
    Atom_Window = _openbox.OBProperty_Atom_Window
    Atom_Pixmap = _openbox.OBProperty_Atom_Pixmap
    Atom_Atom = _openbox.OBProperty_Atom_Atom
    Atom_String = _openbox.OBProperty_Atom_String
    Atom_Utf8 = _openbox.OBProperty_Atom_Utf8
    openbox_pid = _openbox.OBProperty_openbox_pid
    wm_colormap_windows = _openbox.OBProperty_wm_colormap_windows
    wm_protocols = _openbox.OBProperty_wm_protocols
    wm_state = _openbox.OBProperty_wm_state
    wm_delete_window = _openbox.OBProperty_wm_delete_window
    wm_take_focus = _openbox.OBProperty_wm_take_focus
    wm_change_state = _openbox.OBProperty_wm_change_state
    wm_name = _openbox.OBProperty_wm_name
    wm_icon_name = _openbox.OBProperty_wm_icon_name
    wm_class = _openbox.OBProperty_wm_class
    wm_window_role = _openbox.OBProperty_wm_window_role
    motif_wm_hints = _openbox.OBProperty_motif_wm_hints
    blackbox_attributes = _openbox.OBProperty_blackbox_attributes
    blackbox_change_attributes = _openbox.OBProperty_blackbox_change_attributes
    blackbox_hints = _openbox.OBProperty_blackbox_hints
    blackbox_structure_messages = _openbox.OBProperty_blackbox_structure_messages
    blackbox_notify_startup = _openbox.OBProperty_blackbox_notify_startup
    blackbox_notify_window_add = _openbox.OBProperty_blackbox_notify_window_add
    blackbox_notify_window_del = _openbox.OBProperty_blackbox_notify_window_del
    blackbox_notify_window_focus = _openbox.OBProperty_blackbox_notify_window_focus
    blackbox_notify_current_workspace = _openbox.OBProperty_blackbox_notify_current_workspace
    blackbox_notify_workspace_count = _openbox.OBProperty_blackbox_notify_workspace_count
    blackbox_notify_window_raise = _openbox.OBProperty_blackbox_notify_window_raise
    blackbox_notify_window_lower = _openbox.OBProperty_blackbox_notify_window_lower
    blackbox_change_workspace = _openbox.OBProperty_blackbox_change_workspace
    blackbox_change_window_focus = _openbox.OBProperty_blackbox_change_window_focus
    blackbox_cycle_window_focus = _openbox.OBProperty_blackbox_cycle_window_focus
    openbox_show_root_menu = _openbox.OBProperty_openbox_show_root_menu
    openbox_show_workspace_menu = _openbox.OBProperty_openbox_show_workspace_menu
    net_supported = _openbox.OBProperty_net_supported
    net_client_list = _openbox.OBProperty_net_client_list
    net_client_list_stacking = _openbox.OBProperty_net_client_list_stacking
    net_number_of_desktops = _openbox.OBProperty_net_number_of_desktops
    net_desktop_geometry = _openbox.OBProperty_net_desktop_geometry
    net_desktop_viewport = _openbox.OBProperty_net_desktop_viewport
    net_current_desktop = _openbox.OBProperty_net_current_desktop
    net_desktop_names = _openbox.OBProperty_net_desktop_names
    net_active_window = _openbox.OBProperty_net_active_window
    net_workarea = _openbox.OBProperty_net_workarea
    net_supporting_wm_check = _openbox.OBProperty_net_supporting_wm_check
    net_close_window = _openbox.OBProperty_net_close_window
    net_wm_moveresize = _openbox.OBProperty_net_wm_moveresize
    net_wm_name = _openbox.OBProperty_net_wm_name
    net_wm_visible_name = _openbox.OBProperty_net_wm_visible_name
    net_wm_icon_name = _openbox.OBProperty_net_wm_icon_name
    net_wm_visible_icon_name = _openbox.OBProperty_net_wm_visible_icon_name
    net_wm_desktop = _openbox.OBProperty_net_wm_desktop
    net_wm_window_type = _openbox.OBProperty_net_wm_window_type
    net_wm_state = _openbox.OBProperty_net_wm_state
    net_wm_strut = _openbox.OBProperty_net_wm_strut
    net_wm_allowed_actions = _openbox.OBProperty_net_wm_allowed_actions
    net_wm_window_type_desktop = _openbox.OBProperty_net_wm_window_type_desktop
    net_wm_window_type_dock = _openbox.OBProperty_net_wm_window_type_dock
    net_wm_window_type_toolbar = _openbox.OBProperty_net_wm_window_type_toolbar
    net_wm_window_type_menu = _openbox.OBProperty_net_wm_window_type_menu
    net_wm_window_type_utility = _openbox.OBProperty_net_wm_window_type_utility
    net_wm_window_type_splash = _openbox.OBProperty_net_wm_window_type_splash
    net_wm_window_type_dialog = _openbox.OBProperty_net_wm_window_type_dialog
    net_wm_window_type_normal = _openbox.OBProperty_net_wm_window_type_normal
    net_wm_moveresize_size_topleft = _openbox.OBProperty_net_wm_moveresize_size_topleft
    net_wm_moveresize_size_topright = _openbox.OBProperty_net_wm_moveresize_size_topright
    net_wm_moveresize_size_bottomleft = _openbox.OBProperty_net_wm_moveresize_size_bottomleft
    net_wm_moveresize_size_bottomright = _openbox.OBProperty_net_wm_moveresize_size_bottomright
    net_wm_moveresize_move = _openbox.OBProperty_net_wm_moveresize_move
    net_wm_action_move = _openbox.OBProperty_net_wm_action_move
    net_wm_action_resize = _openbox.OBProperty_net_wm_action_resize
    net_wm_action_shade = _openbox.OBProperty_net_wm_action_shade
    net_wm_action_maximize_horz = _openbox.OBProperty_net_wm_action_maximize_horz
    net_wm_action_maximize_vert = _openbox.OBProperty_net_wm_action_maximize_vert
    net_wm_action_change_desktop = _openbox.OBProperty_net_wm_action_change_desktop
    net_wm_action_close = _openbox.OBProperty_net_wm_action_close
    net_wm_state_modal = _openbox.OBProperty_net_wm_state_modal
    net_wm_state_sticky = _openbox.OBProperty_net_wm_state_sticky
    net_wm_state_maximized_vert = _openbox.OBProperty_net_wm_state_maximized_vert
    net_wm_state_maximized_horz = _openbox.OBProperty_net_wm_state_maximized_horz
    net_wm_state_shaded = _openbox.OBProperty_net_wm_state_shaded
    net_wm_state_skip_taskbar = _openbox.OBProperty_net_wm_state_skip_taskbar
    net_wm_state_skip_pager = _openbox.OBProperty_net_wm_state_skip_pager
    net_wm_state_hidden = _openbox.OBProperty_net_wm_state_hidden
    net_wm_state_fullscreen = _openbox.OBProperty_net_wm_state_fullscreen
    net_wm_state_above = _openbox.OBProperty_net_wm_state_above
    net_wm_state_below = _openbox.OBProperty_net_wm_state_below
    kde_net_system_tray_windows = _openbox.OBProperty_kde_net_system_tray_windows
    kde_net_wm_system_tray_window_for = _openbox.OBProperty_kde_net_wm_system_tray_window_for
    kde_net_wm_window_type_override = _openbox.OBProperty_kde_net_wm_window_type_override
    NUM_ATOMS = _openbox.OBProperty_NUM_ATOMS
    ascii = _openbox.OBProperty_ascii
    utf8 = _openbox.OBProperty_utf8
    NUM_STRING_TYPE = _openbox.OBProperty_NUM_STRING_TYPE
    def __init__(self,*args):
        self.this = apply(_openbox.new_OBProperty,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_OBProperty):
        try:
            if self.thisown: destroy(self)
        except: pass
    def set(*args): return apply(_openbox.OBProperty_set,args)
    def get(*args): return apply(_openbox.OBProperty_get,args)
    def erase(*args): return apply(_openbox.OBProperty_erase,args)
    def atom(*args): return apply(_openbox.OBProperty_atom,args)
    def __repr__(self):
        return "<C OBProperty instance at %s>" % (self.this,)

class OBPropertyPtr(OBProperty):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OBProperty
_openbox.OBProperty_swigregister(OBPropertyPtr)

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
    def equals(*args): return apply(_openbox.Rect_equals,args)
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

class ScreenInfo(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, ScreenInfo, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, ScreenInfo, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_ScreenInfo,args)
        self.thisown = 1
    def visual(*args): return apply(_openbox.ScreenInfo_visual,args)
    def rootWindow(*args): return apply(_openbox.ScreenInfo_rootWindow,args)
    def colormap(*args): return apply(_openbox.ScreenInfo_colormap,args)
    def depth(*args): return apply(_openbox.ScreenInfo_depth,args)
    def screen(*args): return apply(_openbox.ScreenInfo_screen,args)
    def rect(*args): return apply(_openbox.ScreenInfo_rect,args)
    def width(*args): return apply(_openbox.ScreenInfo_width,args)
    def height(*args): return apply(_openbox.ScreenInfo_height,args)
    def displayString(*args): return apply(_openbox.ScreenInfo_displayString,args)
    def __repr__(self):
        return "<C ScreenInfo instance at %s>" % (self.this,)

class ScreenInfoPtr(ScreenInfo):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = ScreenInfo
_openbox.ScreenInfo_swigregister(ScreenInfoPtr)

class Strut(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Strut, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Strut, name)
    __swig_setmethods__["top"] = _openbox.Strut_top_set
    __swig_getmethods__["top"] = _openbox.Strut_top_get
    if _newclass:top = property(_openbox.Strut_top_get,_openbox.Strut_top_set)
    __swig_setmethods__["bottom"] = _openbox.Strut_bottom_set
    __swig_getmethods__["bottom"] = _openbox.Strut_bottom_get
    if _newclass:bottom = property(_openbox.Strut_bottom_get,_openbox.Strut_bottom_set)
    __swig_setmethods__["left"] = _openbox.Strut_left_set
    __swig_getmethods__["left"] = _openbox.Strut_left_get
    if _newclass:left = property(_openbox.Strut_left_get,_openbox.Strut_left_set)
    __swig_setmethods__["right"] = _openbox.Strut_right_set
    __swig_getmethods__["right"] = _openbox.Strut_right_get
    if _newclass:right = property(_openbox.Strut_right_get,_openbox.Strut_right_set)
    def __init__(self,*args):
        self.this = apply(_openbox.new_Strut,args)
        self.thisown = 1
    def __repr__(self):
        return "<C Strut instance at %s>" % (self.this,)

class StrutPtr(Strut):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Strut
_openbox.Strut_swigregister(StrutPtr)

class PixmapMask(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, PixmapMask, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, PixmapMask, name)
    __swig_setmethods__["mask"] = _openbox.PixmapMask_mask_set
    __swig_getmethods__["mask"] = _openbox.PixmapMask_mask_get
    if _newclass:mask = property(_openbox.PixmapMask_mask_get,_openbox.PixmapMask_mask_set)
    __swig_setmethods__["w"] = _openbox.PixmapMask_w_set
    __swig_getmethods__["w"] = _openbox.PixmapMask_w_get
    if _newclass:w = property(_openbox.PixmapMask_w_get,_openbox.PixmapMask_w_set)
    __swig_setmethods__["h"] = _openbox.PixmapMask_h_set
    __swig_getmethods__["h"] = _openbox.PixmapMask_h_get
    if _newclass:h = property(_openbox.PixmapMask_h_get,_openbox.PixmapMask_h_set)
    def __init__(self,*args):
        self.this = apply(_openbox.new_PixmapMask,args)
        self.thisown = 1
    def __repr__(self):
        return "<C PixmapMask instance at %s>" % (self.this,)

class PixmapMaskPtr(PixmapMask):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = PixmapMask
_openbox.PixmapMask_swigregister(PixmapMaskPtr)

class Style(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Style, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Style, name)
    ButtonFocus = _openbox.Style_ButtonFocus
    ButtonUnfocus = _openbox.Style_ButtonUnfocus
    TitleFocus = _openbox.Style_TitleFocus
    TitleUnfocus = _openbox.Style_TitleUnfocus
    LabelFocus = _openbox.Style_LabelFocus
    LabelUnfocus = _openbox.Style_LabelUnfocus
    HandleFocus = _openbox.Style_HandleFocus
    HandleUnfocus = _openbox.Style_HandleUnfocus
    GripFocus = _openbox.Style_GripFocus
    GripUnfocus = _openbox.Style_GripUnfocus
    LeftJustify = _openbox.Style_LeftJustify
    RightJustify = _openbox.Style_RightJustify
    CenterJustify = _openbox.Style_CenterJustify
    RoundBullet = _openbox.Style_RoundBullet
    TriangleBullet = _openbox.Style_TriangleBullet
    SquareBullet = _openbox.Style_SquareBullet
    NoBullet = _openbox.Style_NoBullet
    __swig_setmethods__["image_control"] = _openbox.Style_image_control_set
    __swig_getmethods__["image_control"] = _openbox.Style_image_control_get
    if _newclass:image_control = property(_openbox.Style_image_control_get,_openbox.Style_image_control_set)
    __swig_setmethods__["l_text_focus"] = _openbox.Style_l_text_focus_set
    __swig_getmethods__["l_text_focus"] = _openbox.Style_l_text_focus_get
    if _newclass:l_text_focus = property(_openbox.Style_l_text_focus_get,_openbox.Style_l_text_focus_set)
    __swig_setmethods__["l_text_unfocus"] = _openbox.Style_l_text_unfocus_set
    __swig_getmethods__["l_text_unfocus"] = _openbox.Style_l_text_unfocus_get
    if _newclass:l_text_unfocus = property(_openbox.Style_l_text_unfocus_get,_openbox.Style_l_text_unfocus_set)
    __swig_setmethods__["b_pic_focus"] = _openbox.Style_b_pic_focus_set
    __swig_getmethods__["b_pic_focus"] = _openbox.Style_b_pic_focus_get
    if _newclass:b_pic_focus = property(_openbox.Style_b_pic_focus_get,_openbox.Style_b_pic_focus_set)
    __swig_setmethods__["b_pic_unfocus"] = _openbox.Style_b_pic_unfocus_set
    __swig_getmethods__["b_pic_unfocus"] = _openbox.Style_b_pic_unfocus_get
    if _newclass:b_pic_unfocus = property(_openbox.Style_b_pic_unfocus_get,_openbox.Style_b_pic_unfocus_set)
    __swig_setmethods__["border_color"] = _openbox.Style_border_color_set
    __swig_getmethods__["border_color"] = _openbox.Style_border_color_get
    if _newclass:border_color = property(_openbox.Style_border_color_get,_openbox.Style_border_color_set)
    __swig_setmethods__["font"] = _openbox.Style_font_set
    __swig_getmethods__["font"] = _openbox.Style_font_get
    if _newclass:font = property(_openbox.Style_font_get,_openbox.Style_font_set)
    __swig_setmethods__["f_focus"] = _openbox.Style_f_focus_set
    __swig_getmethods__["f_focus"] = _openbox.Style_f_focus_get
    if _newclass:f_focus = property(_openbox.Style_f_focus_get,_openbox.Style_f_focus_set)
    __swig_setmethods__["f_unfocus"] = _openbox.Style_f_unfocus_set
    __swig_getmethods__["f_unfocus"] = _openbox.Style_f_unfocus_get
    if _newclass:f_unfocus = property(_openbox.Style_f_unfocus_get,_openbox.Style_f_unfocus_set)
    __swig_setmethods__["t_focus"] = _openbox.Style_t_focus_set
    __swig_getmethods__["t_focus"] = _openbox.Style_t_focus_get
    if _newclass:t_focus = property(_openbox.Style_t_focus_get,_openbox.Style_t_focus_set)
    __swig_setmethods__["t_unfocus"] = _openbox.Style_t_unfocus_set
    __swig_getmethods__["t_unfocus"] = _openbox.Style_t_unfocus_get
    if _newclass:t_unfocus = property(_openbox.Style_t_unfocus_get,_openbox.Style_t_unfocus_set)
    __swig_setmethods__["l_focus"] = _openbox.Style_l_focus_set
    __swig_getmethods__["l_focus"] = _openbox.Style_l_focus_get
    if _newclass:l_focus = property(_openbox.Style_l_focus_get,_openbox.Style_l_focus_set)
    __swig_setmethods__["l_unfocus"] = _openbox.Style_l_unfocus_set
    __swig_getmethods__["l_unfocus"] = _openbox.Style_l_unfocus_get
    if _newclass:l_unfocus = property(_openbox.Style_l_unfocus_get,_openbox.Style_l_unfocus_set)
    __swig_setmethods__["h_focus"] = _openbox.Style_h_focus_set
    __swig_getmethods__["h_focus"] = _openbox.Style_h_focus_get
    if _newclass:h_focus = property(_openbox.Style_h_focus_get,_openbox.Style_h_focus_set)
    __swig_setmethods__["h_unfocus"] = _openbox.Style_h_unfocus_set
    __swig_getmethods__["h_unfocus"] = _openbox.Style_h_unfocus_get
    if _newclass:h_unfocus = property(_openbox.Style_h_unfocus_get,_openbox.Style_h_unfocus_set)
    __swig_setmethods__["b_focus"] = _openbox.Style_b_focus_set
    __swig_getmethods__["b_focus"] = _openbox.Style_b_focus_get
    if _newclass:b_focus = property(_openbox.Style_b_focus_get,_openbox.Style_b_focus_set)
    __swig_setmethods__["b_unfocus"] = _openbox.Style_b_unfocus_set
    __swig_getmethods__["b_unfocus"] = _openbox.Style_b_unfocus_get
    if _newclass:b_unfocus = property(_openbox.Style_b_unfocus_get,_openbox.Style_b_unfocus_set)
    __swig_setmethods__["b_pressed_focus"] = _openbox.Style_b_pressed_focus_set
    __swig_getmethods__["b_pressed_focus"] = _openbox.Style_b_pressed_focus_get
    if _newclass:b_pressed_focus = property(_openbox.Style_b_pressed_focus_get,_openbox.Style_b_pressed_focus_set)
    __swig_setmethods__["b_pressed_unfocus"] = _openbox.Style_b_pressed_unfocus_set
    __swig_getmethods__["b_pressed_unfocus"] = _openbox.Style_b_pressed_unfocus_get
    if _newclass:b_pressed_unfocus = property(_openbox.Style_b_pressed_unfocus_get,_openbox.Style_b_pressed_unfocus_set)
    __swig_setmethods__["g_focus"] = _openbox.Style_g_focus_set
    __swig_getmethods__["g_focus"] = _openbox.Style_g_focus_get
    if _newclass:g_focus = property(_openbox.Style_g_focus_get,_openbox.Style_g_focus_set)
    __swig_setmethods__["g_unfocus"] = _openbox.Style_g_unfocus_set
    __swig_getmethods__["g_unfocus"] = _openbox.Style_g_unfocus_get
    if _newclass:g_unfocus = property(_openbox.Style_g_unfocus_get,_openbox.Style_g_unfocus_set)
    __swig_setmethods__["close_button"] = _openbox.Style_close_button_set
    __swig_getmethods__["close_button"] = _openbox.Style_close_button_get
    if _newclass:close_button = property(_openbox.Style_close_button_get,_openbox.Style_close_button_set)
    __swig_setmethods__["max_button"] = _openbox.Style_max_button_set
    __swig_getmethods__["max_button"] = _openbox.Style_max_button_get
    if _newclass:max_button = property(_openbox.Style_max_button_get,_openbox.Style_max_button_set)
    __swig_setmethods__["icon_button"] = _openbox.Style_icon_button_set
    __swig_getmethods__["icon_button"] = _openbox.Style_icon_button_get
    if _newclass:icon_button = property(_openbox.Style_icon_button_get,_openbox.Style_icon_button_set)
    __swig_setmethods__["stick_button"] = _openbox.Style_stick_button_set
    __swig_getmethods__["stick_button"] = _openbox.Style_stick_button_get
    if _newclass:stick_button = property(_openbox.Style_stick_button_get,_openbox.Style_stick_button_set)
    __swig_setmethods__["justify"] = _openbox.Style_justify_set
    __swig_getmethods__["justify"] = _openbox.Style_justify_get
    if _newclass:justify = property(_openbox.Style_justify_get,_openbox.Style_justify_set)
    __swig_setmethods__["bullet_type"] = _openbox.Style_bullet_type_set
    __swig_getmethods__["bullet_type"] = _openbox.Style_bullet_type_get
    if _newclass:bullet_type = property(_openbox.Style_bullet_type_get,_openbox.Style_bullet_type_set)
    __swig_setmethods__["handle_width"] = _openbox.Style_handle_width_set
    __swig_getmethods__["handle_width"] = _openbox.Style_handle_width_get
    if _newclass:handle_width = property(_openbox.Style_handle_width_get,_openbox.Style_handle_width_set)
    __swig_setmethods__["bevel_width"] = _openbox.Style_bevel_width_set
    __swig_getmethods__["bevel_width"] = _openbox.Style_bevel_width_get
    if _newclass:bevel_width = property(_openbox.Style_bevel_width_get,_openbox.Style_bevel_width_set)
    __swig_setmethods__["frame_width"] = _openbox.Style_frame_width_set
    __swig_getmethods__["frame_width"] = _openbox.Style_frame_width_get
    if _newclass:frame_width = property(_openbox.Style_frame_width_get,_openbox.Style_frame_width_set)
    __swig_setmethods__["border_width"] = _openbox.Style_border_width_set
    __swig_getmethods__["border_width"] = _openbox.Style_border_width_get
    if _newclass:border_width = property(_openbox.Style_border_width_get,_openbox.Style_border_width_set)
    __swig_setmethods__["screen_number"] = _openbox.Style_screen_number_set
    __swig_getmethods__["screen_number"] = _openbox.Style_screen_number_get
    if _newclass:screen_number = property(_openbox.Style_screen_number_get,_openbox.Style_screen_number_set)
    __swig_setmethods__["shadow_fonts"] = _openbox.Style_shadow_fonts_set
    __swig_getmethods__["shadow_fonts"] = _openbox.Style_shadow_fonts_get
    if _newclass:shadow_fonts = property(_openbox.Style_shadow_fonts_get,_openbox.Style_shadow_fonts_set)
    __swig_setmethods__["aa_fonts"] = _openbox.Style_aa_fonts_set
    __swig_getmethods__["aa_fonts"] = _openbox.Style_aa_fonts_get
    if _newclass:aa_fonts = property(_openbox.Style_aa_fonts_get,_openbox.Style_aa_fonts_set)
    def __init__(self,*args):
        self.this = apply(_openbox.new_Style,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_Style):
        try:
            if self.thisown: destroy(self)
        except: pass
    def readDatabaseMask(*args): return apply(_openbox.Style_readDatabaseMask,args)
    def readDatabaseTexture(*args): return apply(_openbox.Style_readDatabaseTexture,args)
    def readDatabaseColor(*args): return apply(_openbox.Style_readDatabaseColor,args)
    def readDatabaseFont(*args): return apply(_openbox.Style_readDatabaseFont,args)
    def load(*args): return apply(_openbox.Style_load,args)
    def getCloseButtonMask(*args): return apply(_openbox.Style_getCloseButtonMask,args)
    def getMaximizeButtonMask(*args): return apply(_openbox.Style_getMaximizeButtonMask,args)
    def getIconifyButtonMask(*args): return apply(_openbox.Style_getIconifyButtonMask,args)
    def getStickyButtonMask(*args): return apply(_openbox.Style_getStickyButtonMask,args)
    def getTextFocus(*args): return apply(_openbox.Style_getTextFocus,args)
    def getTextUnfocus(*args): return apply(_openbox.Style_getTextUnfocus,args)
    def getButtonPicFocus(*args): return apply(_openbox.Style_getButtonPicFocus,args)
    def getButtonPicUnfocus(*args): return apply(_openbox.Style_getButtonPicUnfocus,args)
    def getTitleFocus(*args): return apply(_openbox.Style_getTitleFocus,args)
    def getTitleUnfocus(*args): return apply(_openbox.Style_getTitleUnfocus,args)
    def getLabelFocus(*args): return apply(_openbox.Style_getLabelFocus,args)
    def getLabelUnfocus(*args): return apply(_openbox.Style_getLabelUnfocus,args)
    def getHandleFocus(*args): return apply(_openbox.Style_getHandleFocus,args)
    def getHandleUnfocus(*args): return apply(_openbox.Style_getHandleUnfocus,args)
    def getButtonFocus(*args): return apply(_openbox.Style_getButtonFocus,args)
    def getButtonUnfocus(*args): return apply(_openbox.Style_getButtonUnfocus,args)
    def getButtonPressedFocus(*args): return apply(_openbox.Style_getButtonPressedFocus,args)
    def getButtonPressedUnfocus(*args): return apply(_openbox.Style_getButtonPressedUnfocus,args)
    def getGripFocus(*args): return apply(_openbox.Style_getGripFocus,args)
    def getGripUnfocus(*args): return apply(_openbox.Style_getGripUnfocus,args)
    def getHandleWidth(*args): return apply(_openbox.Style_getHandleWidth,args)
    def getBevelWidth(*args): return apply(_openbox.Style_getBevelWidth,args)
    def getFrameWidth(*args): return apply(_openbox.Style_getFrameWidth,args)
    def getBorderWidth(*args): return apply(_openbox.Style_getBorderWidth,args)
    def getFont(*args): return apply(_openbox.Style_getFont,args)
    def setShadowFonts(*args): return apply(_openbox.Style_setShadowFonts,args)
    def hasShadowFonts(*args): return apply(_openbox.Style_hasShadowFonts,args)
    def setAAFonts(*args): return apply(_openbox.Style_setAAFonts,args)
    def hasAAFonts(*args): return apply(_openbox.Style_hasAAFonts,args)
    def textJustify(*args): return apply(_openbox.Style_textJustify,args)
    def bulletType(*args): return apply(_openbox.Style_bulletType,args)
    def getBorderColor(*args): return apply(_openbox.Style_getBorderColor,args)
    def getFrameFocus(*args): return apply(_openbox.Style_getFrameFocus,args)
    def getFrameUnfocus(*args): return apply(_openbox.Style_getFrameUnfocus,args)
    def setImageControl(*args): return apply(_openbox.Style_setImageControl,args)
    def getScreen(*args): return apply(_openbox.Style_getScreen,args)
    def __repr__(self):
        return "<C Style instance at %s>" % (self.this,)

class StylePtr(Style):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Style
_openbox.Style_swigregister(StylePtr)

class BTexture(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BTexture, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BTexture, name)
    NoTexture = _openbox.BTexture_NoTexture
    Flat = _openbox.BTexture_Flat
    Sunken = _openbox.BTexture_Sunken
    Raised = _openbox.BTexture_Raised
    Solid = _openbox.BTexture_Solid
    Gradient = _openbox.BTexture_Gradient
    Horizontal = _openbox.BTexture_Horizontal
    Vertical = _openbox.BTexture_Vertical
    Diagonal = _openbox.BTexture_Diagonal
    CrossDiagonal = _openbox.BTexture_CrossDiagonal
    Rectangle = _openbox.BTexture_Rectangle
    Pyramid = _openbox.BTexture_Pyramid
    PipeCross = _openbox.BTexture_PipeCross
    Elliptic = _openbox.BTexture_Elliptic
    Bevel1 = _openbox.BTexture_Bevel1
    Bevel2 = _openbox.BTexture_Bevel2
    Border = _openbox.BTexture_Border
    Invert = _openbox.BTexture_Invert
    Parent_Relative = _openbox.BTexture_Parent_Relative
    Interlaced = _openbox.BTexture_Interlaced
    def __init__(self,*args):
        self.this = apply(_openbox.new_BTexture,args)
        self.thisown = 1
    def setColor(*args): return apply(_openbox.BTexture_setColor,args)
    def setColorTo(*args): return apply(_openbox.BTexture_setColorTo,args)
    def setBorderColor(*args): return apply(_openbox.BTexture_setBorderColor,args)
    def color(*args): return apply(_openbox.BTexture_color,args)
    def colorTo(*args): return apply(_openbox.BTexture_colorTo,args)
    def lightColor(*args): return apply(_openbox.BTexture_lightColor,args)
    def shadowColor(*args): return apply(_openbox.BTexture_shadowColor,args)
    def borderColor(*args): return apply(_openbox.BTexture_borderColor,args)
    def texture(*args): return apply(_openbox.BTexture_texture,args)
    def setTexture(*args): return apply(_openbox.BTexture_setTexture,args)
    def addTexture(*args): return apply(_openbox.BTexture_addTexture,args)
    def equals(*args): return apply(_openbox.BTexture_equals,args)
    def screen(*args): return apply(_openbox.BTexture_screen,args)
    def setScreen(*args): return apply(_openbox.BTexture_setScreen,args)
    def setImageControl(*args): return apply(_openbox.BTexture_setImageControl,args)
    def description(*args): return apply(_openbox.BTexture_description,args)
    def setDescription(*args): return apply(_openbox.BTexture_setDescription,args)
    def render(*args): return apply(_openbox.BTexture_render,args)
    def __repr__(self):
        return "<C BTexture instance at %s>" % (self.this,)

class BTexturePtr(BTexture):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BTexture
_openbox.BTexture_swigregister(BTexturePtr)

class OBTimer(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, OBTimer, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, OBTimer, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_OBTimer,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_OBTimer):
        try:
            if self.thisown: destroy(self)
        except: pass
    def fire(*args): return apply(_openbox.OBTimer_fire,args)
    def timing(*args): return apply(_openbox.OBTimer_timing,args)
    def recurring(*args): return apply(_openbox.OBTimer_recurring,args)
    def timeout(*args): return apply(_openbox.OBTimer_timeout,args)
    def startTime(*args): return apply(_openbox.OBTimer_startTime,args)
    def remainingTime(*args): return apply(_openbox.OBTimer_remainingTime,args)
    def shouldFire(*args): return apply(_openbox.OBTimer_shouldFire,args)
    def endTime(*args): return apply(_openbox.OBTimer_endTime,args)
    def setRecurring(*args): return apply(_openbox.OBTimer_setRecurring,args)
    def setTimeout(*args): return apply(_openbox.OBTimer_setTimeout,args)
    def start(*args): return apply(_openbox.OBTimer_start,args)
    def stop(*args): return apply(_openbox.OBTimer_stop,args)
    def __repr__(self):
        return "<C OBTimer instance at %s>" % (self.this,)

class OBTimerPtr(OBTimer):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OBTimer
_openbox.OBTimer_swigregister(OBTimerPtr)

class OBTimerQueueManager(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, OBTimerQueueManager, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, OBTimerQueueManager, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_OBTimerQueueManager,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_OBTimerQueueManager):
        try:
            if self.thisown: destroy(self)
        except: pass
    def fire(*args): return apply(_openbox.OBTimerQueueManager_fire,args)
    def addTimer(*args): return apply(_openbox.OBTimerQueueManager_addTimer,args)
    def removeTimer(*args): return apply(_openbox.OBTimerQueueManager_removeTimer,args)
    def __repr__(self):
        return "<C OBTimerQueueManager instance at %s>" % (self.this,)

class OBTimerQueueManagerPtr(OBTimerQueueManager):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OBTimerQueueManager
_openbox.OBTimerQueueManager_swigregister(OBTimerQueueManagerPtr)

expandTilde = _openbox.expandTilde

bexec = _openbox.bexec

textPropertyToString = _openbox.textPropertyToString

itostring_unsigned_long = _openbox.itostring_unsigned_long

itostring_long = _openbox.itostring_long

itostring_unsigned = _openbox.itostring_unsigned

itostring = _openbox.itostring

basename = _openbox.basename

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
cvar = _openbox.cvar
BSENTINEL = cvar.BSENTINEL

class Openbox(OtkEventDispatcher,OtkEventHandler):
    __swig_setmethods__ = {}
    for _s in [OtkEventDispatcher,OtkEventHandler]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, Openbox, name, value)
    __swig_getmethods__ = {}
    for _s in [OtkEventDispatcher,OtkEventHandler]: __swig_getmethods__.update(_s.__swig_getmethods__)
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

class OBScreen(OtkEventHandler,):
    __swig_setmethods__ = {}
    for _s in [OtkEventHandler,]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, OBScreen, name, value)
    __swig_getmethods__ = {}
    for _s in [OtkEventHandler,]: __swig_getmethods__.update(_s.__swig_getmethods__)
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

class OBClient(OtkEventHandler,):
    __swig_setmethods__ = {}
    for _s in [OtkEventHandler,]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, OBClient, name, value)
    __swig_getmethods__ = {}
    for _s in [OtkEventHandler,]: __swig_getmethods__.update(_s.__swig_getmethods__)
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


