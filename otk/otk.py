# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.
import _otk
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


OBDisplay_display = _otk.OBDisplay_display

class OtkEventDispatcher(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, OtkEventDispatcher, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, OtkEventDispatcher, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_OtkEventDispatcher,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_OtkEventDispatcher):
        try:
            if self.thisown: destroy(self)
        except: pass
    def clearAllHandlers(*args): return apply(_otk.OtkEventDispatcher_clearAllHandlers,args)
    def registerHandler(*args): return apply(_otk.OtkEventDispatcher_registerHandler,args)
    def clearHandler(*args): return apply(_otk.OtkEventDispatcher_clearHandler,args)
    def dispatchEvents(*args): return apply(_otk.OtkEventDispatcher_dispatchEvents,args)
    def setFallbackHandler(*args): return apply(_otk.OtkEventDispatcher_setFallbackHandler,args)
    def getFallbackHandler(*args): return apply(_otk.OtkEventDispatcher_getFallbackHandler,args)
    def setMasterHandler(*args): return apply(_otk.OtkEventDispatcher_setMasterHandler,args)
    def getMasterHandler(*args): return apply(_otk.OtkEventDispatcher_getMasterHandler,args)
    def findHandler(*args): return apply(_otk.OtkEventDispatcher_findHandler,args)
    def lastTime(*args): return apply(_otk.OtkEventDispatcher_lastTime,args)
    def __repr__(self):
        return "<C OtkEventDispatcher instance at %s>" % (self.this,)

class OtkEventDispatcherPtr(OtkEventDispatcher):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OtkEventDispatcher
_otk.OtkEventDispatcher_swigregister(OtkEventDispatcherPtr)

class OtkEventHandler(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, OtkEventHandler, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, OtkEventHandler, name)
    def handle(*args): return apply(_otk.OtkEventHandler_handle,args)
    def keyPressHandler(*args): return apply(_otk.OtkEventHandler_keyPressHandler,args)
    def keyReleaseHandler(*args): return apply(_otk.OtkEventHandler_keyReleaseHandler,args)
    def buttonPressHandler(*args): return apply(_otk.OtkEventHandler_buttonPressHandler,args)
    def buttonReleaseHandler(*args): return apply(_otk.OtkEventHandler_buttonReleaseHandler,args)
    def motionHandler(*args): return apply(_otk.OtkEventHandler_motionHandler,args)
    def enterHandler(*args): return apply(_otk.OtkEventHandler_enterHandler,args)
    def leaveHandler(*args): return apply(_otk.OtkEventHandler_leaveHandler,args)
    def focusHandler(*args): return apply(_otk.OtkEventHandler_focusHandler,args)
    def unfocusHandler(*args): return apply(_otk.OtkEventHandler_unfocusHandler,args)
    def exposeHandler(*args): return apply(_otk.OtkEventHandler_exposeHandler,args)
    def graphicsExposeHandler(*args): return apply(_otk.OtkEventHandler_graphicsExposeHandler,args)
    def noExposeEventHandler(*args): return apply(_otk.OtkEventHandler_noExposeEventHandler,args)
    def circulateRequestHandler(*args): return apply(_otk.OtkEventHandler_circulateRequestHandler,args)
    def configureRequestHandler(*args): return apply(_otk.OtkEventHandler_configureRequestHandler,args)
    def mapRequestHandler(*args): return apply(_otk.OtkEventHandler_mapRequestHandler,args)
    def resizeRequestHandler(*args): return apply(_otk.OtkEventHandler_resizeRequestHandler,args)
    def circulateHandler(*args): return apply(_otk.OtkEventHandler_circulateHandler,args)
    def configureHandler(*args): return apply(_otk.OtkEventHandler_configureHandler,args)
    def createHandler(*args): return apply(_otk.OtkEventHandler_createHandler,args)
    def destroyHandler(*args): return apply(_otk.OtkEventHandler_destroyHandler,args)
    def gravityHandler(*args): return apply(_otk.OtkEventHandler_gravityHandler,args)
    def mapHandler(*args): return apply(_otk.OtkEventHandler_mapHandler,args)
    def mappingHandler(*args): return apply(_otk.OtkEventHandler_mappingHandler,args)
    def reparentHandler(*args): return apply(_otk.OtkEventHandler_reparentHandler,args)
    def unmapHandler(*args): return apply(_otk.OtkEventHandler_unmapHandler,args)
    def visibilityHandler(*args): return apply(_otk.OtkEventHandler_visibilityHandler,args)
    def colorMapHandler(*args): return apply(_otk.OtkEventHandler_colorMapHandler,args)
    def propertyHandler(*args): return apply(_otk.OtkEventHandler_propertyHandler,args)
    def selectionClearHandler(*args): return apply(_otk.OtkEventHandler_selectionClearHandler,args)
    def selectionHandler(*args): return apply(_otk.OtkEventHandler_selectionHandler,args)
    def selectionRequestHandler(*args): return apply(_otk.OtkEventHandler_selectionRequestHandler,args)
    def clientMessageHandler(*args): return apply(_otk.OtkEventHandler_clientMessageHandler,args)
    def __del__(self, destroy= _otk.delete_OtkEventHandler):
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
_otk.OtkEventHandler_swigregister(OtkEventHandlerPtr)

class OtkWidget(OtkEventHandler):
    __swig_setmethods__ = {}
    for _s in [OtkEventHandler]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, OtkWidget, name, value)
    __swig_getmethods__ = {}
    for _s in [OtkEventHandler]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, OtkWidget, name)
    Horizontal = _otk.OtkWidget_Horizontal
    Vertical = _otk.OtkWidget_Vertical
    def __init__(self,*args):
        self.this = apply(_otk.new_OtkWidget,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_OtkWidget):
        try:
            if self.thisown: destroy(self)
        except: pass
    def update(*args): return apply(_otk.OtkWidget_update,args)
    def exposeHandler(*args): return apply(_otk.OtkWidget_exposeHandler,args)
    def configureHandler(*args): return apply(_otk.OtkWidget_configureHandler,args)
    def window(*args): return apply(_otk.OtkWidget_window,args)
    def parent(*args): return apply(_otk.OtkWidget_parent,args)
    def children(*args): return apply(_otk.OtkWidget_children,args)
    def screen(*args): return apply(_otk.OtkWidget_screen,args)
    def rect(*args): return apply(_otk.OtkWidget_rect,args)
    def move(*args): return apply(_otk.OtkWidget_move,args)
    def setWidth(*args): return apply(_otk.OtkWidget_setWidth,args)
    def setHeight(*args): return apply(_otk.OtkWidget_setHeight,args)
    def width(*args): return apply(_otk.OtkWidget_width,args)
    def height(*args): return apply(_otk.OtkWidget_height,args)
    def resize(*args): return apply(_otk.OtkWidget_resize,args)
    def setGeometry(*args): return apply(_otk.OtkWidget_setGeometry,args)
    def isVisible(*args): return apply(_otk.OtkWidget_isVisible,args)
    def show(*args): return apply(_otk.OtkWidget_show,args)
    def hide(*args): return apply(_otk.OtkWidget_hide,args)
    def isFocused(*args): return apply(_otk.OtkWidget_isFocused,args)
    def focus(*args): return apply(_otk.OtkWidget_focus,args)
    def unfocus(*args): return apply(_otk.OtkWidget_unfocus,args)
    def hasGrabbedMouse(*args): return apply(_otk.OtkWidget_hasGrabbedMouse,args)
    def grabMouse(*args): return apply(_otk.OtkWidget_grabMouse,args)
    def ungrabMouse(*args): return apply(_otk.OtkWidget_ungrabMouse,args)
    def hasGrabbedKeyboard(*args): return apply(_otk.OtkWidget_hasGrabbedKeyboard,args)
    def grabKeyboard(*args): return apply(_otk.OtkWidget_grabKeyboard,args)
    def ungrabKeyboard(*args): return apply(_otk.OtkWidget_ungrabKeyboard,args)
    def texture(*args): return apply(_otk.OtkWidget_texture,args)
    def setTexture(*args): return apply(_otk.OtkWidget_setTexture,args)
    def borderColor(*args): return apply(_otk.OtkWidget_borderColor,args)
    def setBorderColor(*args): return apply(_otk.OtkWidget_setBorderColor,args)
    def borderWidth(*args): return apply(_otk.OtkWidget_borderWidth,args)
    def setBorderWidth(*args): return apply(_otk.OtkWidget_setBorderWidth,args)
    def addChild(*args): return apply(_otk.OtkWidget_addChild,args)
    def removeChild(*args): return apply(_otk.OtkWidget_removeChild,args)
    def isStretchableHorz(*args): return apply(_otk.OtkWidget_isStretchableHorz,args)
    def setStretchableHorz(*args): return apply(_otk.OtkWidget_setStretchableHorz,args)
    def isStretchableVert(*args): return apply(_otk.OtkWidget_isStretchableVert,args)
    def setStretchableVert(*args): return apply(_otk.OtkWidget_setStretchableVert,args)
    def cursor(*args): return apply(_otk.OtkWidget_cursor,args)
    def setCursor(*args): return apply(_otk.OtkWidget_setCursor,args)
    def bevelWidth(*args): return apply(_otk.OtkWidget_bevelWidth,args)
    def setBevelWidth(*args): return apply(_otk.OtkWidget_setBevelWidth,args)
    def direction(*args): return apply(_otk.OtkWidget_direction,args)
    def setDirection(*args): return apply(_otk.OtkWidget_setDirection,args)
    def style(*args): return apply(_otk.OtkWidget_style,args)
    def setStyle(*args): return apply(_otk.OtkWidget_setStyle,args)
    def eventDispatcher(*args): return apply(_otk.OtkWidget_eventDispatcher,args)
    def setEventDispatcher(*args): return apply(_otk.OtkWidget_setEventDispatcher,args)
    def __repr__(self):
        return "<C OtkWidget instance at %s>" % (self.this,)

class OtkWidgetPtr(OtkWidget):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OtkWidget
_otk.OtkWidget_swigregister(OtkWidgetPtr)

class OtkFocusWidget(OtkWidget):
    __swig_setmethods__ = {}
    for _s in [OtkWidget]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, OtkFocusWidget, name, value)
    __swig_getmethods__ = {}
    for _s in [OtkWidget]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, OtkFocusWidget, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_OtkFocusWidget,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_OtkFocusWidget):
        try:
            if self.thisown: destroy(self)
        except: pass
    def focus(*args): return apply(_otk.OtkFocusWidget_focus,args)
    def unfocus(*args): return apply(_otk.OtkFocusWidget_unfocus,args)
    def setTexture(*args): return apply(_otk.OtkFocusWidget_setTexture,args)
    def setBorderColor(*args): return apply(_otk.OtkFocusWidget_setBorderColor,args)
    def setUnfocusTexture(*args): return apply(_otk.OtkFocusWidget_setUnfocusTexture,args)
    def getUnfocusTexture(*args): return apply(_otk.OtkFocusWidget_getUnfocusTexture,args)
    def setUnfocusBorderColor(*args): return apply(_otk.OtkFocusWidget_setUnfocusBorderColor,args)
    def getUnfocusBorderColor(*args): return apply(_otk.OtkFocusWidget_getUnfocusBorderColor,args)
    def isFocused(*args): return apply(_otk.OtkFocusWidget_isFocused,args)
    def isUnfocused(*args): return apply(_otk.OtkFocusWidget_isUnfocused,args)
    def __repr__(self):
        return "<C OtkFocusWidget instance at %s>" % (self.this,)

class OtkFocusWidgetPtr(OtkFocusWidget):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OtkFocusWidget
_otk.OtkFocusWidget_swigregister(OtkFocusWidgetPtr)

class OtkFocusLabel(OtkFocusWidget):
    __swig_setmethods__ = {}
    for _s in [OtkFocusWidget]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, OtkFocusLabel, name, value)
    __swig_getmethods__ = {}
    for _s in [OtkFocusWidget]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, OtkFocusLabel, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_OtkFocusLabel,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_OtkFocusLabel):
        try:
            if self.thisown: destroy(self)
        except: pass
    def getText(*args): return apply(_otk.OtkFocusLabel_getText,args)
    def setText(*args): return apply(_otk.OtkFocusLabel_setText,args)
    def update(*args): return apply(_otk.OtkFocusLabel_update,args)
    def setStyle(*args): return apply(_otk.OtkFocusLabel_setStyle,args)
    def __repr__(self):
        return "<C OtkFocusLabel instance at %s>" % (self.this,)

class OtkFocusLabelPtr(OtkFocusLabel):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OtkFocusLabel
_otk.OtkFocusLabel_swigregister(OtkFocusLabelPtr)

class OtkAppWidget(OtkWidget):
    __swig_setmethods__ = {}
    for _s in [OtkWidget]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, OtkAppWidget, name, value)
    __swig_getmethods__ = {}
    for _s in [OtkWidget]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, OtkAppWidget, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_OtkAppWidget,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_OtkAppWidget):
        try:
            if self.thisown: destroy(self)
        except: pass
    def show(*args): return apply(_otk.OtkAppWidget_show,args)
    def hide(*args): return apply(_otk.OtkAppWidget_hide,args)
    def clientMessageHandler(*args): return apply(_otk.OtkAppWidget_clientMessageHandler,args)
    def __repr__(self):
        return "<C OtkAppWidget instance at %s>" % (self.this,)

class OtkAppWidgetPtr(OtkAppWidget):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OtkAppWidget
_otk.OtkAppWidget_swigregister(OtkAppWidgetPtr)

class OtkApplication(OtkEventDispatcher):
    __swig_setmethods__ = {}
    for _s in [OtkEventDispatcher]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, OtkApplication, name, value)
    __swig_getmethods__ = {}
    for _s in [OtkEventDispatcher]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, OtkApplication, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_OtkApplication,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_OtkApplication):
        try:
            if self.thisown: destroy(self)
        except: pass
    def run(*args): return apply(_otk.OtkApplication_run,args)
    def setDockable(*args): return apply(_otk.OtkApplication_setDockable,args)
    def isDockable(*args): return apply(_otk.OtkApplication_isDockable,args)
    def getStyle(*args): return apply(_otk.OtkApplication_getStyle,args)
    def __repr__(self):
        return "<C OtkApplication instance at %s>" % (self.this,)

class OtkApplicationPtr(OtkApplication):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OtkApplication
_otk.OtkApplication_swigregister(OtkApplicationPtr)

class PointerAssassin(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, PointerAssassin, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, PointerAssassin, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_PointerAssassin,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_PointerAssassin):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C PointerAssassin instance at %s>" % (self.this,)

class PointerAssassinPtr(PointerAssassin):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = PointerAssassin
_otk.PointerAssassin_swigregister(PointerAssassinPtr)

class OtkButton(OtkFocusLabel):
    __swig_setmethods__ = {}
    for _s in [OtkFocusLabel]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, OtkButton, name, value)
    __swig_getmethods__ = {}
    for _s in [OtkFocusLabel]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, OtkButton, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_OtkButton,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_OtkButton):
        try:
            if self.thisown: destroy(self)
        except: pass
    def getPressedFocusTexture(*args): return apply(_otk.OtkButton_getPressedFocusTexture,args)
    def setPressedFocusTexture(*args): return apply(_otk.OtkButton_setPressedFocusTexture,args)
    def getPressedUnfocusTexture(*args): return apply(_otk.OtkButton_getPressedUnfocusTexture,args)
    def setPressedUnfocusTexture(*args): return apply(_otk.OtkButton_setPressedUnfocusTexture,args)
    def setTexture(*args): return apply(_otk.OtkButton_setTexture,args)
    def setUnfocusTexture(*args): return apply(_otk.OtkButton_setUnfocusTexture,args)
    def isPressed(*args): return apply(_otk.OtkButton_isPressed,args)
    def press(*args): return apply(_otk.OtkButton_press,args)
    def release(*args): return apply(_otk.OtkButton_release,args)
    def buttonPressHandler(*args): return apply(_otk.OtkButton_buttonPressHandler,args)
    def buttonReleaseHandler(*args): return apply(_otk.OtkButton_buttonReleaseHandler,args)
    def setStyle(*args): return apply(_otk.OtkButton_setStyle,args)
    def __repr__(self):
        return "<C OtkButton instance at %s>" % (self.this,)

class OtkButtonPtr(OtkButton):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OtkButton
_otk.OtkButton_swigregister(OtkButtonPtr)

class BColor(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BColor, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BColor, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_BColor,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_BColor):
        try:
            if self.thisown: destroy(self)
        except: pass
    def name(*args): return apply(_otk.BColor_name,args)
    def red(*args): return apply(_otk.BColor_red,args)
    def green(*args): return apply(_otk.BColor_green,args)
    def blue(*args): return apply(_otk.BColor_blue,args)
    def setRGB(*args): return apply(_otk.BColor_setRGB,args)
    def screen(*args): return apply(_otk.BColor_screen,args)
    def setScreen(*args): return apply(_otk.BColor_setScreen,args)
    def isAllocated(*args): return apply(_otk.BColor_isAllocated,args)
    def isValid(*args): return apply(_otk.BColor_isValid,args)
    def pixel(*args): return apply(_otk.BColor_pixel,args)
    def equals(*args): return apply(_otk.BColor_equals,args)
    __swig_getmethods__["cleanupColorCache"] = lambda x: _otk.BColor_cleanupColorCache
    if _newclass:cleanupColorCache = staticmethod(_otk.BColor_cleanupColorCache)
    def __repr__(self):
        return "<C BColor instance at %s>" % (self.this,)

class BColorPtr(BColor):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BColor
_otk.BColor_swigregister(BColorPtr)
BColor_cleanupColorCache = _otk.BColor_cleanupColorCache


class Configuration(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Configuration, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Configuration, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_Configuration,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_Configuration):
        try:
            if self.thisown: destroy(self)
        except: pass
    def file(*args): return apply(_otk.Configuration_file,args)
    def setFile(*args): return apply(_otk.Configuration_setFile,args)
    def autoSave(*args): return apply(_otk.Configuration_autoSave,args)
    def setAutoSave(*args): return apply(_otk.Configuration_setAutoSave,args)
    def isModified(*args): return apply(_otk.Configuration_isModified,args)
    def save(*args): return apply(_otk.Configuration_save,args)
    def load(*args): return apply(_otk.Configuration_load,args)
    def merge(*args): return apply(_otk.Configuration_merge,args)
    def create(*args): return apply(_otk.Configuration_create,args)
    def setValue_bool(*args): return apply(_otk.Configuration_setValue_bool,args)
    def setValue(*args): return apply(_otk.Configuration_setValue,args)
    def setValue_unsigned(*args): return apply(_otk.Configuration_setValue_unsigned,args)
    def setValue_long(*args): return apply(_otk.Configuration_setValue_long,args)
    def setValue_unsignedlong(*args): return apply(_otk.Configuration_setValue_unsignedlong,args)
    def setValue_string(*args): return apply(_otk.Configuration_setValue_string,args)
    def setValue_charptr(*args): return apply(_otk.Configuration_setValue_charptr,args)
    def getValue(*args): return apply(_otk.Configuration_getValue,args)
    def __repr__(self):
        return "<C Configuration instance at %s>" % (self.this,)

class ConfigurationPtr(Configuration):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Configuration
_otk.Configuration_swigregister(ConfigurationPtr)

class OBDisplay(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, OBDisplay, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, OBDisplay, name)
    __swig_getmethods__["initialize"] = lambda x: _otk.OBDisplay_initialize
    if _newclass:initialize = staticmethod(_otk.OBDisplay_initialize)
    __swig_getmethods__["destroy"] = lambda x: _otk.OBDisplay_destroy
    if _newclass:destroy = staticmethod(_otk.OBDisplay_destroy)
    __swig_getmethods__["gcCache"] = lambda x: _otk.OBDisplay_gcCache
    if _newclass:gcCache = staticmethod(_otk.OBDisplay_gcCache)
    __swig_getmethods__["screenInfo"] = lambda x: _otk.OBDisplay_screenInfo
    if _newclass:screenInfo = staticmethod(_otk.OBDisplay_screenInfo)
    __swig_getmethods__["findScreen"] = lambda x: _otk.OBDisplay_findScreen
    if _newclass:findScreen = staticmethod(_otk.OBDisplay_findScreen)
    __swig_getmethods__["xkb"] = lambda x: _otk.OBDisplay_xkb
    if _newclass:xkb = staticmethod(_otk.OBDisplay_xkb)
    __swig_getmethods__["xkbEventBase"] = lambda x: _otk.OBDisplay_xkbEventBase
    if _newclass:xkbEventBase = staticmethod(_otk.OBDisplay_xkbEventBase)
    __swig_getmethods__["shape"] = lambda x: _otk.OBDisplay_shape
    if _newclass:shape = staticmethod(_otk.OBDisplay_shape)
    __swig_getmethods__["shapeEventBase"] = lambda x: _otk.OBDisplay_shapeEventBase
    if _newclass:shapeEventBase = staticmethod(_otk.OBDisplay_shapeEventBase)
    __swig_getmethods__["xinerama"] = lambda x: _otk.OBDisplay_xinerama
    if _newclass:xinerama = staticmethod(_otk.OBDisplay_xinerama)
    __swig_getmethods__["numLockMask"] = lambda x: _otk.OBDisplay_numLockMask
    if _newclass:numLockMask = staticmethod(_otk.OBDisplay_numLockMask)
    __swig_getmethods__["scrollLockMask"] = lambda x: _otk.OBDisplay_scrollLockMask
    if _newclass:scrollLockMask = staticmethod(_otk.OBDisplay_scrollLockMask)
    __swig_getmethods__["grab"] = lambda x: _otk.OBDisplay_grab
    if _newclass:grab = staticmethod(_otk.OBDisplay_grab)
    __swig_getmethods__["ungrab"] = lambda x: _otk.OBDisplay_ungrab
    if _newclass:ungrab = staticmethod(_otk.OBDisplay_ungrab)
    __swig_getmethods__["grabButton"] = lambda x: _otk.OBDisplay_grabButton
    if _newclass:grabButton = staticmethod(_otk.OBDisplay_grabButton)
    __swig_getmethods__["ungrabButton"] = lambda x: _otk.OBDisplay_ungrabButton
    if _newclass:ungrabButton = staticmethod(_otk.OBDisplay_ungrabButton)
    __swig_getmethods__["grabKey"] = lambda x: _otk.OBDisplay_grabKey
    if _newclass:grabKey = staticmethod(_otk.OBDisplay_grabKey)
    __swig_getmethods__["ungrabKey"] = lambda x: _otk.OBDisplay_ungrabKey
    if _newclass:ungrabKey = staticmethod(_otk.OBDisplay_ungrabKey)
    def __del__(self, destroy= _otk.delete_OBDisplay):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C OBDisplay instance at %s>" % (self.this,)

class OBDisplayPtr(OBDisplay):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OBDisplay
_otk.OBDisplay_swigregister(OBDisplayPtr)
OBDisplay_initialize = _otk.OBDisplay_initialize

OBDisplay_destroy = _otk.OBDisplay_destroy

OBDisplay_gcCache = _otk.OBDisplay_gcCache

OBDisplay_screenInfo = _otk.OBDisplay_screenInfo

OBDisplay_findScreen = _otk.OBDisplay_findScreen

OBDisplay_xkb = _otk.OBDisplay_xkb

OBDisplay_xkbEventBase = _otk.OBDisplay_xkbEventBase

OBDisplay_shape = _otk.OBDisplay_shape

OBDisplay_shapeEventBase = _otk.OBDisplay_shapeEventBase

OBDisplay_xinerama = _otk.OBDisplay_xinerama

OBDisplay_numLockMask = _otk.OBDisplay_numLockMask

OBDisplay_scrollLockMask = _otk.OBDisplay_scrollLockMask

OBDisplay_grab = _otk.OBDisplay_grab

OBDisplay_ungrab = _otk.OBDisplay_ungrab

OBDisplay_grabButton = _otk.OBDisplay_grabButton

OBDisplay_ungrabButton = _otk.OBDisplay_ungrabButton

OBDisplay_grabKey = _otk.OBDisplay_grabKey

OBDisplay_ungrabKey = _otk.OBDisplay_ungrabKey


class BFont(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BFont, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BFont, name)
    __swig_getmethods__["fallbackFont"] = lambda x: _otk.BFont_fallbackFont
    if _newclass:fallbackFont = staticmethod(_otk.BFont_fallbackFont)
    __swig_getmethods__["setFallbackFont"] = lambda x: _otk.BFont_setFallbackFont
    if _newclass:setFallbackFont = staticmethod(_otk.BFont_setFallbackFont)
    def __init__(self,*args):
        self.this = apply(_otk.new_BFont,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_BFont):
        try:
            if self.thisown: destroy(self)
        except: pass
    def fontstring(*args): return apply(_otk.BFont_fontstring,args)
    def height(*args): return apply(_otk.BFont_height,args)
    def maxCharWidth(*args): return apply(_otk.BFont_maxCharWidth,args)
    def measureString(*args): return apply(_otk.BFont_measureString,args)
    def drawString(*args): return apply(_otk.BFont_drawString,args)
    def __repr__(self):
        return "<C BFont instance at %s>" % (self.this,)

class BFontPtr(BFont):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BFont
_otk.BFont_swigregister(BFontPtr)
BFont_fallbackFont = _otk.BFont_fallbackFont

BFont_setFallbackFont = _otk.BFont_setFallbackFont


class BGCCacheContext(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BGCCacheContext, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BGCCacheContext, name)
    def set(*args): return apply(_otk.BGCCacheContext_set,args)
    def __del__(self, destroy= _otk.delete_BGCCacheContext):
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
_otk.BGCCacheContext_swigregister(BGCCacheContextPtr)

class BGCCacheItem(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BGCCacheItem, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BGCCacheItem, name)
    def gc(*args): return apply(_otk.BGCCacheItem_gc,args)
    def __del__(self, destroy= _otk.delete_BGCCacheItem):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C BGCCacheItem instance at %s>" % (self.this,)

class BGCCacheItemPtr(BGCCacheItem):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BGCCacheItem
_otk.BGCCacheItem_swigregister(BGCCacheItemPtr)

class BGCCache(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BGCCache, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BGCCache, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_BGCCache,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_BGCCache):
        try:
            if self.thisown: destroy(self)
        except: pass
    def purge(*args): return apply(_otk.BGCCache_purge,args)
    def find(*args): return apply(_otk.BGCCache_find,args)
    def release(*args): return apply(_otk.BGCCache_release,args)
    def __repr__(self):
        return "<C BGCCache instance at %s>" % (self.this,)

class BGCCachePtr(BGCCache):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BGCCache
_otk.BGCCache_swigregister(BGCCachePtr)

class BPen(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BPen, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BPen, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_BPen,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_BPen):
        try:
            if self.thisown: destroy(self)
        except: pass
    def gc(*args): return apply(_otk.BPen_gc,args)
    def __repr__(self):
        return "<C BPen instance at %s>" % (self.this,)

class BPenPtr(BPen):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BPen
_otk.BPen_swigregister(BPenPtr)

class BImage(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BImage, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BImage, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_BImage,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_BImage):
        try:
            if self.thisown: destroy(self)
        except: pass
    def render(*args): return apply(_otk.BImage_render,args)
    def __repr__(self):
        return "<C BImage instance at %s>" % (self.this,)

class BImagePtr(BImage):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BImage
_otk.BImage_swigregister(BImagePtr)

class BImageControl(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BImageControl, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BImageControl, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_BImageControl,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_BImageControl):
        try:
            if self.thisown: destroy(self)
        except: pass
    def doDither(*args): return apply(_otk.BImageControl_doDither,args)
    def getScreenInfo(*args): return apply(_otk.BImageControl_getScreenInfo,args)
    def getDrawable(*args): return apply(_otk.BImageControl_getDrawable,args)
    def getVisual(*args): return apply(_otk.BImageControl_getVisual,args)
    def getBitsPerPixel(*args): return apply(_otk.BImageControl_getBitsPerPixel,args)
    def getDepth(*args): return apply(_otk.BImageControl_getDepth,args)
    def getColorsPerChannel(*args): return apply(_otk.BImageControl_getColorsPerChannel,args)
    def getSqrt(*args): return apply(_otk.BImageControl_getSqrt,args)
    def renderImage(*args): return apply(_otk.BImageControl_renderImage,args)
    def installRootColormap(*args): return apply(_otk.BImageControl_installRootColormap,args)
    def removeImage(*args): return apply(_otk.BImageControl_removeImage,args)
    def getColorTables(*args): return apply(_otk.BImageControl_getColorTables,args)
    def getXColorTable(*args): return apply(_otk.BImageControl_getXColorTable,args)
    def getGradientBuffers(*args): return apply(_otk.BImageControl_getGradientBuffers,args)
    def setDither(*args): return apply(_otk.BImageControl_setDither,args)
    def setColorsPerChannel(*args): return apply(_otk.BImageControl_setColorsPerChannel,args)
    __swig_getmethods__["timeout"] = lambda x: _otk.BImageControl_timeout
    if _newclass:timeout = staticmethod(_otk.BImageControl_timeout)
    def __repr__(self):
        return "<C BImageControl instance at %s>" % (self.this,)

class BImageControlPtr(BImageControl):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BImageControl
_otk.BImageControl_swigregister(BImageControlPtr)
BImageControl_timeout = _otk.BImageControl_timeout


class Point(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Point, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Point, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_Point,args)
        self.thisown = 1
    def setX(*args): return apply(_otk.Point_setX,args)
    def x(*args): return apply(_otk.Point_x,args)
    def setY(*args): return apply(_otk.Point_setY,args)
    def y(*args): return apply(_otk.Point_y,args)
    def setPoint(*args): return apply(_otk.Point_setPoint,args)
    def __del__(self, destroy= _otk.delete_Point):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C Point instance at %s>" % (self.this,)

class PointPtr(Point):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Point
_otk.Point_swigregister(PointPtr)

class OBProperty(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, OBProperty, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, OBProperty, name)
    Atom_Cardinal = _otk.OBProperty_Atom_Cardinal
    Atom_Window = _otk.OBProperty_Atom_Window
    Atom_Pixmap = _otk.OBProperty_Atom_Pixmap
    Atom_Atom = _otk.OBProperty_Atom_Atom
    Atom_String = _otk.OBProperty_Atom_String
    Atom_Utf8 = _otk.OBProperty_Atom_Utf8
    openbox_pid = _otk.OBProperty_openbox_pid
    wm_colormap_windows = _otk.OBProperty_wm_colormap_windows
    wm_protocols = _otk.OBProperty_wm_protocols
    wm_state = _otk.OBProperty_wm_state
    wm_delete_window = _otk.OBProperty_wm_delete_window
    wm_take_focus = _otk.OBProperty_wm_take_focus
    wm_change_state = _otk.OBProperty_wm_change_state
    wm_name = _otk.OBProperty_wm_name
    wm_icon_name = _otk.OBProperty_wm_icon_name
    wm_class = _otk.OBProperty_wm_class
    wm_window_role = _otk.OBProperty_wm_window_role
    motif_wm_hints = _otk.OBProperty_motif_wm_hints
    blackbox_attributes = _otk.OBProperty_blackbox_attributes
    blackbox_change_attributes = _otk.OBProperty_blackbox_change_attributes
    blackbox_hints = _otk.OBProperty_blackbox_hints
    blackbox_structure_messages = _otk.OBProperty_blackbox_structure_messages
    blackbox_notify_startup = _otk.OBProperty_blackbox_notify_startup
    blackbox_notify_window_add = _otk.OBProperty_blackbox_notify_window_add
    blackbox_notify_window_del = _otk.OBProperty_blackbox_notify_window_del
    blackbox_notify_window_focus = _otk.OBProperty_blackbox_notify_window_focus
    blackbox_notify_current_workspace = _otk.OBProperty_blackbox_notify_current_workspace
    blackbox_notify_workspace_count = _otk.OBProperty_blackbox_notify_workspace_count
    blackbox_notify_window_raise = _otk.OBProperty_blackbox_notify_window_raise
    blackbox_notify_window_lower = _otk.OBProperty_blackbox_notify_window_lower
    blackbox_change_workspace = _otk.OBProperty_blackbox_change_workspace
    blackbox_change_window_focus = _otk.OBProperty_blackbox_change_window_focus
    blackbox_cycle_window_focus = _otk.OBProperty_blackbox_cycle_window_focus
    openbox_show_root_menu = _otk.OBProperty_openbox_show_root_menu
    openbox_show_workspace_menu = _otk.OBProperty_openbox_show_workspace_menu
    net_supported = _otk.OBProperty_net_supported
    net_client_list = _otk.OBProperty_net_client_list
    net_client_list_stacking = _otk.OBProperty_net_client_list_stacking
    net_number_of_desktops = _otk.OBProperty_net_number_of_desktops
    net_desktop_geometry = _otk.OBProperty_net_desktop_geometry
    net_desktop_viewport = _otk.OBProperty_net_desktop_viewport
    net_current_desktop = _otk.OBProperty_net_current_desktop
    net_desktop_names = _otk.OBProperty_net_desktop_names
    net_active_window = _otk.OBProperty_net_active_window
    net_workarea = _otk.OBProperty_net_workarea
    net_supporting_wm_check = _otk.OBProperty_net_supporting_wm_check
    net_close_window = _otk.OBProperty_net_close_window
    net_wm_moveresize = _otk.OBProperty_net_wm_moveresize
    net_wm_name = _otk.OBProperty_net_wm_name
    net_wm_visible_name = _otk.OBProperty_net_wm_visible_name
    net_wm_icon_name = _otk.OBProperty_net_wm_icon_name
    net_wm_visible_icon_name = _otk.OBProperty_net_wm_visible_icon_name
    net_wm_desktop = _otk.OBProperty_net_wm_desktop
    net_wm_window_type = _otk.OBProperty_net_wm_window_type
    net_wm_state = _otk.OBProperty_net_wm_state
    net_wm_strut = _otk.OBProperty_net_wm_strut
    net_wm_allowed_actions = _otk.OBProperty_net_wm_allowed_actions
    net_wm_window_type_desktop = _otk.OBProperty_net_wm_window_type_desktop
    net_wm_window_type_dock = _otk.OBProperty_net_wm_window_type_dock
    net_wm_window_type_toolbar = _otk.OBProperty_net_wm_window_type_toolbar
    net_wm_window_type_menu = _otk.OBProperty_net_wm_window_type_menu
    net_wm_window_type_utility = _otk.OBProperty_net_wm_window_type_utility
    net_wm_window_type_splash = _otk.OBProperty_net_wm_window_type_splash
    net_wm_window_type_dialog = _otk.OBProperty_net_wm_window_type_dialog
    net_wm_window_type_normal = _otk.OBProperty_net_wm_window_type_normal
    net_wm_moveresize_size_topleft = _otk.OBProperty_net_wm_moveresize_size_topleft
    net_wm_moveresize_size_topright = _otk.OBProperty_net_wm_moveresize_size_topright
    net_wm_moveresize_size_bottomleft = _otk.OBProperty_net_wm_moveresize_size_bottomleft
    net_wm_moveresize_size_bottomright = _otk.OBProperty_net_wm_moveresize_size_bottomright
    net_wm_moveresize_move = _otk.OBProperty_net_wm_moveresize_move
    net_wm_action_move = _otk.OBProperty_net_wm_action_move
    net_wm_action_resize = _otk.OBProperty_net_wm_action_resize
    net_wm_action_shade = _otk.OBProperty_net_wm_action_shade
    net_wm_action_maximize_horz = _otk.OBProperty_net_wm_action_maximize_horz
    net_wm_action_maximize_vert = _otk.OBProperty_net_wm_action_maximize_vert
    net_wm_action_change_desktop = _otk.OBProperty_net_wm_action_change_desktop
    net_wm_action_close = _otk.OBProperty_net_wm_action_close
    net_wm_state_modal = _otk.OBProperty_net_wm_state_modal
    net_wm_state_sticky = _otk.OBProperty_net_wm_state_sticky
    net_wm_state_maximized_vert = _otk.OBProperty_net_wm_state_maximized_vert
    net_wm_state_maximized_horz = _otk.OBProperty_net_wm_state_maximized_horz
    net_wm_state_shaded = _otk.OBProperty_net_wm_state_shaded
    net_wm_state_skip_taskbar = _otk.OBProperty_net_wm_state_skip_taskbar
    net_wm_state_skip_pager = _otk.OBProperty_net_wm_state_skip_pager
    net_wm_state_hidden = _otk.OBProperty_net_wm_state_hidden
    net_wm_state_fullscreen = _otk.OBProperty_net_wm_state_fullscreen
    net_wm_state_above = _otk.OBProperty_net_wm_state_above
    net_wm_state_below = _otk.OBProperty_net_wm_state_below
    kde_net_system_tray_windows = _otk.OBProperty_kde_net_system_tray_windows
    kde_net_wm_system_tray_window_for = _otk.OBProperty_kde_net_wm_system_tray_window_for
    kde_net_wm_window_type_override = _otk.OBProperty_kde_net_wm_window_type_override
    NUM_ATOMS = _otk.OBProperty_NUM_ATOMS
    ascii = _otk.OBProperty_ascii
    utf8 = _otk.OBProperty_utf8
    NUM_STRING_TYPE = _otk.OBProperty_NUM_STRING_TYPE
    def __init__(self,*args):
        self.this = apply(_otk.new_OBProperty,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_OBProperty):
        try:
            if self.thisown: destroy(self)
        except: pass
    def set(*args): return apply(_otk.OBProperty_set,args)
    def get(*args): return apply(_otk.OBProperty_get,args)
    def erase(*args): return apply(_otk.OBProperty_erase,args)
    def atom(*args): return apply(_otk.OBProperty_atom,args)
    def __repr__(self):
        return "<C OBProperty instance at %s>" % (self.this,)

class OBPropertyPtr(OBProperty):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OBProperty
_otk.OBProperty_swigregister(OBPropertyPtr)

class Rect(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Rect, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Rect, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_Rect,args)
        self.thisown = 1
    def left(*args): return apply(_otk.Rect_left,args)
    def top(*args): return apply(_otk.Rect_top,args)
    def right(*args): return apply(_otk.Rect_right,args)
    def bottom(*args): return apply(_otk.Rect_bottom,args)
    def x(*args): return apply(_otk.Rect_x,args)
    def y(*args): return apply(_otk.Rect_y,args)
    def location(*args): return apply(_otk.Rect_location,args)
    def setX(*args): return apply(_otk.Rect_setX,args)
    def setY(*args): return apply(_otk.Rect_setY,args)
    def setPos(*args): return apply(_otk.Rect_setPos,args)
    def width(*args): return apply(_otk.Rect_width,args)
    def height(*args): return apply(_otk.Rect_height,args)
    def size(*args): return apply(_otk.Rect_size,args)
    def setWidth(*args): return apply(_otk.Rect_setWidth,args)
    def setHeight(*args): return apply(_otk.Rect_setHeight,args)
    def setSize(*args): return apply(_otk.Rect_setSize,args)
    def setRect(*args): return apply(_otk.Rect_setRect,args)
    def setCoords(*args): return apply(_otk.Rect_setCoords,args)
    def equals(*args): return apply(_otk.Rect_equals,args)
    def valid(*args): return apply(_otk.Rect_valid,args)
    def intersects(*args): return apply(_otk.Rect_intersects,args)
    def contains(*args): return apply(_otk.Rect_contains,args)
    def __del__(self, destroy= _otk.delete_Rect):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C Rect instance at %s>" % (self.this,)

class RectPtr(Rect):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Rect
_otk.Rect_swigregister(RectPtr)

class ScreenInfo(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, ScreenInfo, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, ScreenInfo, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_ScreenInfo,args)
        self.thisown = 1
    def visual(*args): return apply(_otk.ScreenInfo_visual,args)
    def rootWindow(*args): return apply(_otk.ScreenInfo_rootWindow,args)
    def colormap(*args): return apply(_otk.ScreenInfo_colormap,args)
    def depth(*args): return apply(_otk.ScreenInfo_depth,args)
    def screen(*args): return apply(_otk.ScreenInfo_screen,args)
    def rect(*args): return apply(_otk.ScreenInfo_rect,args)
    def width(*args): return apply(_otk.ScreenInfo_width,args)
    def height(*args): return apply(_otk.ScreenInfo_height,args)
    def displayString(*args): return apply(_otk.ScreenInfo_displayString,args)
    def __del__(self, destroy= _otk.delete_ScreenInfo):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C ScreenInfo instance at %s>" % (self.this,)

class ScreenInfoPtr(ScreenInfo):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = ScreenInfo
_otk.ScreenInfo_swigregister(ScreenInfoPtr)

class Strut(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Strut, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Strut, name)
    __swig_setmethods__["top"] = _otk.Strut_top_set
    __swig_getmethods__["top"] = _otk.Strut_top_get
    if _newclass:top = property(_otk.Strut_top_get,_otk.Strut_top_set)
    __swig_setmethods__["bottom"] = _otk.Strut_bottom_set
    __swig_getmethods__["bottom"] = _otk.Strut_bottom_get
    if _newclass:bottom = property(_otk.Strut_bottom_get,_otk.Strut_bottom_set)
    __swig_setmethods__["left"] = _otk.Strut_left_set
    __swig_getmethods__["left"] = _otk.Strut_left_get
    if _newclass:left = property(_otk.Strut_left_get,_otk.Strut_left_set)
    __swig_setmethods__["right"] = _otk.Strut_right_set
    __swig_getmethods__["right"] = _otk.Strut_right_get
    if _newclass:right = property(_otk.Strut_right_get,_otk.Strut_right_set)
    def __init__(self,*args):
        self.this = apply(_otk.new_Strut,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_Strut):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C Strut instance at %s>" % (self.this,)

class StrutPtr(Strut):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Strut
_otk.Strut_swigregister(StrutPtr)

class PixmapMask(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, PixmapMask, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, PixmapMask, name)
    __swig_setmethods__["mask"] = _otk.PixmapMask_mask_set
    __swig_getmethods__["mask"] = _otk.PixmapMask_mask_get
    if _newclass:mask = property(_otk.PixmapMask_mask_get,_otk.PixmapMask_mask_set)
    __swig_setmethods__["w"] = _otk.PixmapMask_w_set
    __swig_getmethods__["w"] = _otk.PixmapMask_w_get
    if _newclass:w = property(_otk.PixmapMask_w_get,_otk.PixmapMask_w_set)
    __swig_setmethods__["h"] = _otk.PixmapMask_h_set
    __swig_getmethods__["h"] = _otk.PixmapMask_h_get
    if _newclass:h = property(_otk.PixmapMask_h_get,_otk.PixmapMask_h_set)
    def __init__(self,*args):
        self.this = apply(_otk.new_PixmapMask,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_PixmapMask):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C PixmapMask instance at %s>" % (self.this,)

class PixmapMaskPtr(PixmapMask):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = PixmapMask
_otk.PixmapMask_swigregister(PixmapMaskPtr)

class Style(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Style, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Style, name)
    ButtonFocus = _otk.Style_ButtonFocus
    ButtonUnfocus = _otk.Style_ButtonUnfocus
    TitleFocus = _otk.Style_TitleFocus
    TitleUnfocus = _otk.Style_TitleUnfocus
    LabelFocus = _otk.Style_LabelFocus
    LabelUnfocus = _otk.Style_LabelUnfocus
    HandleFocus = _otk.Style_HandleFocus
    HandleUnfocus = _otk.Style_HandleUnfocus
    GripFocus = _otk.Style_GripFocus
    GripUnfocus = _otk.Style_GripUnfocus
    LeftJustify = _otk.Style_LeftJustify
    RightJustify = _otk.Style_RightJustify
    CenterJustify = _otk.Style_CenterJustify
    RoundBullet = _otk.Style_RoundBullet
    TriangleBullet = _otk.Style_TriangleBullet
    SquareBullet = _otk.Style_SquareBullet
    NoBullet = _otk.Style_NoBullet
    __swig_setmethods__["image_control"] = _otk.Style_image_control_set
    __swig_getmethods__["image_control"] = _otk.Style_image_control_get
    if _newclass:image_control = property(_otk.Style_image_control_get,_otk.Style_image_control_set)
    __swig_setmethods__["l_text_focus"] = _otk.Style_l_text_focus_set
    __swig_getmethods__["l_text_focus"] = _otk.Style_l_text_focus_get
    if _newclass:l_text_focus = property(_otk.Style_l_text_focus_get,_otk.Style_l_text_focus_set)
    __swig_setmethods__["l_text_unfocus"] = _otk.Style_l_text_unfocus_set
    __swig_getmethods__["l_text_unfocus"] = _otk.Style_l_text_unfocus_get
    if _newclass:l_text_unfocus = property(_otk.Style_l_text_unfocus_get,_otk.Style_l_text_unfocus_set)
    __swig_setmethods__["b_pic_focus"] = _otk.Style_b_pic_focus_set
    __swig_getmethods__["b_pic_focus"] = _otk.Style_b_pic_focus_get
    if _newclass:b_pic_focus = property(_otk.Style_b_pic_focus_get,_otk.Style_b_pic_focus_set)
    __swig_setmethods__["b_pic_unfocus"] = _otk.Style_b_pic_unfocus_set
    __swig_getmethods__["b_pic_unfocus"] = _otk.Style_b_pic_unfocus_get
    if _newclass:b_pic_unfocus = property(_otk.Style_b_pic_unfocus_get,_otk.Style_b_pic_unfocus_set)
    __swig_setmethods__["border_color"] = _otk.Style_border_color_set
    __swig_getmethods__["border_color"] = _otk.Style_border_color_get
    if _newclass:border_color = property(_otk.Style_border_color_get,_otk.Style_border_color_set)
    __swig_setmethods__["font"] = _otk.Style_font_set
    __swig_getmethods__["font"] = _otk.Style_font_get
    if _newclass:font = property(_otk.Style_font_get,_otk.Style_font_set)
    __swig_setmethods__["f_focus"] = _otk.Style_f_focus_set
    __swig_getmethods__["f_focus"] = _otk.Style_f_focus_get
    if _newclass:f_focus = property(_otk.Style_f_focus_get,_otk.Style_f_focus_set)
    __swig_setmethods__["f_unfocus"] = _otk.Style_f_unfocus_set
    __swig_getmethods__["f_unfocus"] = _otk.Style_f_unfocus_get
    if _newclass:f_unfocus = property(_otk.Style_f_unfocus_get,_otk.Style_f_unfocus_set)
    __swig_setmethods__["t_focus"] = _otk.Style_t_focus_set
    __swig_getmethods__["t_focus"] = _otk.Style_t_focus_get
    if _newclass:t_focus = property(_otk.Style_t_focus_get,_otk.Style_t_focus_set)
    __swig_setmethods__["t_unfocus"] = _otk.Style_t_unfocus_set
    __swig_getmethods__["t_unfocus"] = _otk.Style_t_unfocus_get
    if _newclass:t_unfocus = property(_otk.Style_t_unfocus_get,_otk.Style_t_unfocus_set)
    __swig_setmethods__["l_focus"] = _otk.Style_l_focus_set
    __swig_getmethods__["l_focus"] = _otk.Style_l_focus_get
    if _newclass:l_focus = property(_otk.Style_l_focus_get,_otk.Style_l_focus_set)
    __swig_setmethods__["l_unfocus"] = _otk.Style_l_unfocus_set
    __swig_getmethods__["l_unfocus"] = _otk.Style_l_unfocus_get
    if _newclass:l_unfocus = property(_otk.Style_l_unfocus_get,_otk.Style_l_unfocus_set)
    __swig_setmethods__["h_focus"] = _otk.Style_h_focus_set
    __swig_getmethods__["h_focus"] = _otk.Style_h_focus_get
    if _newclass:h_focus = property(_otk.Style_h_focus_get,_otk.Style_h_focus_set)
    __swig_setmethods__["h_unfocus"] = _otk.Style_h_unfocus_set
    __swig_getmethods__["h_unfocus"] = _otk.Style_h_unfocus_get
    if _newclass:h_unfocus = property(_otk.Style_h_unfocus_get,_otk.Style_h_unfocus_set)
    __swig_setmethods__["b_focus"] = _otk.Style_b_focus_set
    __swig_getmethods__["b_focus"] = _otk.Style_b_focus_get
    if _newclass:b_focus = property(_otk.Style_b_focus_get,_otk.Style_b_focus_set)
    __swig_setmethods__["b_unfocus"] = _otk.Style_b_unfocus_set
    __swig_getmethods__["b_unfocus"] = _otk.Style_b_unfocus_get
    if _newclass:b_unfocus = property(_otk.Style_b_unfocus_get,_otk.Style_b_unfocus_set)
    __swig_setmethods__["b_pressed_focus"] = _otk.Style_b_pressed_focus_set
    __swig_getmethods__["b_pressed_focus"] = _otk.Style_b_pressed_focus_get
    if _newclass:b_pressed_focus = property(_otk.Style_b_pressed_focus_get,_otk.Style_b_pressed_focus_set)
    __swig_setmethods__["b_pressed_unfocus"] = _otk.Style_b_pressed_unfocus_set
    __swig_getmethods__["b_pressed_unfocus"] = _otk.Style_b_pressed_unfocus_get
    if _newclass:b_pressed_unfocus = property(_otk.Style_b_pressed_unfocus_get,_otk.Style_b_pressed_unfocus_set)
    __swig_setmethods__["g_focus"] = _otk.Style_g_focus_set
    __swig_getmethods__["g_focus"] = _otk.Style_g_focus_get
    if _newclass:g_focus = property(_otk.Style_g_focus_get,_otk.Style_g_focus_set)
    __swig_setmethods__["g_unfocus"] = _otk.Style_g_unfocus_set
    __swig_getmethods__["g_unfocus"] = _otk.Style_g_unfocus_get
    if _newclass:g_unfocus = property(_otk.Style_g_unfocus_get,_otk.Style_g_unfocus_set)
    __swig_setmethods__["close_button"] = _otk.Style_close_button_set
    __swig_getmethods__["close_button"] = _otk.Style_close_button_get
    if _newclass:close_button = property(_otk.Style_close_button_get,_otk.Style_close_button_set)
    __swig_setmethods__["max_button"] = _otk.Style_max_button_set
    __swig_getmethods__["max_button"] = _otk.Style_max_button_get
    if _newclass:max_button = property(_otk.Style_max_button_get,_otk.Style_max_button_set)
    __swig_setmethods__["icon_button"] = _otk.Style_icon_button_set
    __swig_getmethods__["icon_button"] = _otk.Style_icon_button_get
    if _newclass:icon_button = property(_otk.Style_icon_button_get,_otk.Style_icon_button_set)
    __swig_setmethods__["stick_button"] = _otk.Style_stick_button_set
    __swig_getmethods__["stick_button"] = _otk.Style_stick_button_get
    if _newclass:stick_button = property(_otk.Style_stick_button_get,_otk.Style_stick_button_set)
    __swig_setmethods__["justify"] = _otk.Style_justify_set
    __swig_getmethods__["justify"] = _otk.Style_justify_get
    if _newclass:justify = property(_otk.Style_justify_get,_otk.Style_justify_set)
    __swig_setmethods__["bullet_type"] = _otk.Style_bullet_type_set
    __swig_getmethods__["bullet_type"] = _otk.Style_bullet_type_get
    if _newclass:bullet_type = property(_otk.Style_bullet_type_get,_otk.Style_bullet_type_set)
    __swig_setmethods__["handle_width"] = _otk.Style_handle_width_set
    __swig_getmethods__["handle_width"] = _otk.Style_handle_width_get
    if _newclass:handle_width = property(_otk.Style_handle_width_get,_otk.Style_handle_width_set)
    __swig_setmethods__["bevel_width"] = _otk.Style_bevel_width_set
    __swig_getmethods__["bevel_width"] = _otk.Style_bevel_width_get
    if _newclass:bevel_width = property(_otk.Style_bevel_width_get,_otk.Style_bevel_width_set)
    __swig_setmethods__["frame_width"] = _otk.Style_frame_width_set
    __swig_getmethods__["frame_width"] = _otk.Style_frame_width_get
    if _newclass:frame_width = property(_otk.Style_frame_width_get,_otk.Style_frame_width_set)
    __swig_setmethods__["border_width"] = _otk.Style_border_width_set
    __swig_getmethods__["border_width"] = _otk.Style_border_width_get
    if _newclass:border_width = property(_otk.Style_border_width_get,_otk.Style_border_width_set)
    __swig_setmethods__["screen_number"] = _otk.Style_screen_number_set
    __swig_getmethods__["screen_number"] = _otk.Style_screen_number_get
    if _newclass:screen_number = property(_otk.Style_screen_number_get,_otk.Style_screen_number_set)
    __swig_setmethods__["shadow_fonts"] = _otk.Style_shadow_fonts_set
    __swig_getmethods__["shadow_fonts"] = _otk.Style_shadow_fonts_get
    if _newclass:shadow_fonts = property(_otk.Style_shadow_fonts_get,_otk.Style_shadow_fonts_set)
    __swig_setmethods__["aa_fonts"] = _otk.Style_aa_fonts_set
    __swig_getmethods__["aa_fonts"] = _otk.Style_aa_fonts_get
    if _newclass:aa_fonts = property(_otk.Style_aa_fonts_get,_otk.Style_aa_fonts_set)
    def __init__(self,*args):
        self.this = apply(_otk.new_Style,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_Style):
        try:
            if self.thisown: destroy(self)
        except: pass
    def readDatabaseMask(*args): return apply(_otk.Style_readDatabaseMask,args)
    def readDatabaseTexture(*args): return apply(_otk.Style_readDatabaseTexture,args)
    def readDatabaseColor(*args): return apply(_otk.Style_readDatabaseColor,args)
    def readDatabaseFont(*args): return apply(_otk.Style_readDatabaseFont,args)
    def load(*args): return apply(_otk.Style_load,args)
    def getCloseButtonMask(*args): return apply(_otk.Style_getCloseButtonMask,args)
    def getMaximizeButtonMask(*args): return apply(_otk.Style_getMaximizeButtonMask,args)
    def getIconifyButtonMask(*args): return apply(_otk.Style_getIconifyButtonMask,args)
    def getStickyButtonMask(*args): return apply(_otk.Style_getStickyButtonMask,args)
    def getTextFocus(*args): return apply(_otk.Style_getTextFocus,args)
    def getTextUnfocus(*args): return apply(_otk.Style_getTextUnfocus,args)
    def getButtonPicFocus(*args): return apply(_otk.Style_getButtonPicFocus,args)
    def getButtonPicUnfocus(*args): return apply(_otk.Style_getButtonPicUnfocus,args)
    def getTitleFocus(*args): return apply(_otk.Style_getTitleFocus,args)
    def getTitleUnfocus(*args): return apply(_otk.Style_getTitleUnfocus,args)
    def getLabelFocus(*args): return apply(_otk.Style_getLabelFocus,args)
    def getLabelUnfocus(*args): return apply(_otk.Style_getLabelUnfocus,args)
    def getHandleFocus(*args): return apply(_otk.Style_getHandleFocus,args)
    def getHandleUnfocus(*args): return apply(_otk.Style_getHandleUnfocus,args)
    def getButtonFocus(*args): return apply(_otk.Style_getButtonFocus,args)
    def getButtonUnfocus(*args): return apply(_otk.Style_getButtonUnfocus,args)
    def getButtonPressedFocus(*args): return apply(_otk.Style_getButtonPressedFocus,args)
    def getButtonPressedUnfocus(*args): return apply(_otk.Style_getButtonPressedUnfocus,args)
    def getGripFocus(*args): return apply(_otk.Style_getGripFocus,args)
    def getGripUnfocus(*args): return apply(_otk.Style_getGripUnfocus,args)
    def getHandleWidth(*args): return apply(_otk.Style_getHandleWidth,args)
    def getBevelWidth(*args): return apply(_otk.Style_getBevelWidth,args)
    def getFrameWidth(*args): return apply(_otk.Style_getFrameWidth,args)
    def getBorderWidth(*args): return apply(_otk.Style_getBorderWidth,args)
    def getFont(*args): return apply(_otk.Style_getFont,args)
    def setShadowFonts(*args): return apply(_otk.Style_setShadowFonts,args)
    def hasShadowFonts(*args): return apply(_otk.Style_hasShadowFonts,args)
    def setAAFonts(*args): return apply(_otk.Style_setAAFonts,args)
    def hasAAFonts(*args): return apply(_otk.Style_hasAAFonts,args)
    def textJustify(*args): return apply(_otk.Style_textJustify,args)
    def bulletType(*args): return apply(_otk.Style_bulletType,args)
    def getBorderColor(*args): return apply(_otk.Style_getBorderColor,args)
    def getFrameFocus(*args): return apply(_otk.Style_getFrameFocus,args)
    def getFrameUnfocus(*args): return apply(_otk.Style_getFrameUnfocus,args)
    def setImageControl(*args): return apply(_otk.Style_setImageControl,args)
    def getScreen(*args): return apply(_otk.Style_getScreen,args)
    def __repr__(self):
        return "<C Style instance at %s>" % (self.this,)

class StylePtr(Style):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Style
_otk.Style_swigregister(StylePtr)

class BTexture(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, BTexture, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, BTexture, name)
    NoTexture = _otk.BTexture_NoTexture
    Flat = _otk.BTexture_Flat
    Sunken = _otk.BTexture_Sunken
    Raised = _otk.BTexture_Raised
    Solid = _otk.BTexture_Solid
    Gradient = _otk.BTexture_Gradient
    Horizontal = _otk.BTexture_Horizontal
    Vertical = _otk.BTexture_Vertical
    Diagonal = _otk.BTexture_Diagonal
    CrossDiagonal = _otk.BTexture_CrossDiagonal
    Rectangle = _otk.BTexture_Rectangle
    Pyramid = _otk.BTexture_Pyramid
    PipeCross = _otk.BTexture_PipeCross
    Elliptic = _otk.BTexture_Elliptic
    Bevel1 = _otk.BTexture_Bevel1
    Bevel2 = _otk.BTexture_Bevel2
    Border = _otk.BTexture_Border
    Invert = _otk.BTexture_Invert
    Parent_Relative = _otk.BTexture_Parent_Relative
    Interlaced = _otk.BTexture_Interlaced
    def __init__(self,*args):
        self.this = apply(_otk.new_BTexture,args)
        self.thisown = 1
    def setColor(*args): return apply(_otk.BTexture_setColor,args)
    def setColorTo(*args): return apply(_otk.BTexture_setColorTo,args)
    def setBorderColor(*args): return apply(_otk.BTexture_setBorderColor,args)
    def color(*args): return apply(_otk.BTexture_color,args)
    def colorTo(*args): return apply(_otk.BTexture_colorTo,args)
    def lightColor(*args): return apply(_otk.BTexture_lightColor,args)
    def shadowColor(*args): return apply(_otk.BTexture_shadowColor,args)
    def borderColor(*args): return apply(_otk.BTexture_borderColor,args)
    def texture(*args): return apply(_otk.BTexture_texture,args)
    def setTexture(*args): return apply(_otk.BTexture_setTexture,args)
    def addTexture(*args): return apply(_otk.BTexture_addTexture,args)
    def equals(*args): return apply(_otk.BTexture_equals,args)
    def screen(*args): return apply(_otk.BTexture_screen,args)
    def setScreen(*args): return apply(_otk.BTexture_setScreen,args)
    def setImageControl(*args): return apply(_otk.BTexture_setImageControl,args)
    def description(*args): return apply(_otk.BTexture_description,args)
    def setDescription(*args): return apply(_otk.BTexture_setDescription,args)
    def render(*args): return apply(_otk.BTexture_render,args)
    def __del__(self, destroy= _otk.delete_BTexture):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C BTexture instance at %s>" % (self.this,)

class BTexturePtr(BTexture):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = BTexture
_otk.BTexture_swigregister(BTexturePtr)

class OBTimer(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, OBTimer, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, OBTimer, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_OBTimer,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_OBTimer):
        try:
            if self.thisown: destroy(self)
        except: pass
    def fire(*args): return apply(_otk.OBTimer_fire,args)
    def timing(*args): return apply(_otk.OBTimer_timing,args)
    def recurring(*args): return apply(_otk.OBTimer_recurring,args)
    def timeout(*args): return apply(_otk.OBTimer_timeout,args)
    def startTime(*args): return apply(_otk.OBTimer_startTime,args)
    def remainingTime(*args): return apply(_otk.OBTimer_remainingTime,args)
    def shouldFire(*args): return apply(_otk.OBTimer_shouldFire,args)
    def endTime(*args): return apply(_otk.OBTimer_endTime,args)
    def setRecurring(*args): return apply(_otk.OBTimer_setRecurring,args)
    def setTimeout(*args): return apply(_otk.OBTimer_setTimeout,args)
    def start(*args): return apply(_otk.OBTimer_start,args)
    def stop(*args): return apply(_otk.OBTimer_stop,args)
    def __repr__(self):
        return "<C OBTimer instance at %s>" % (self.this,)

class OBTimerPtr(OBTimer):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OBTimer
_otk.OBTimer_swigregister(OBTimerPtr)

class OBTimerQueueManager(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, OBTimerQueueManager, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, OBTimerQueueManager, name)
    def __init__(self,*args):
        self.this = apply(_otk.new_OBTimerQueueManager,args)
        self.thisown = 1
    def __del__(self, destroy= _otk.delete_OBTimerQueueManager):
        try:
            if self.thisown: destroy(self)
        except: pass
    def fire(*args): return apply(_otk.OBTimerQueueManager_fire,args)
    def addTimer(*args): return apply(_otk.OBTimerQueueManager_addTimer,args)
    def removeTimer(*args): return apply(_otk.OBTimerQueueManager_removeTimer,args)
    def __repr__(self):
        return "<C OBTimerQueueManager instance at %s>" % (self.this,)

class OBTimerQueueManagerPtr(OBTimerQueueManager):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = OBTimerQueueManager
_otk.OBTimerQueueManager_swigregister(OBTimerQueueManagerPtr)

expandTilde = _otk.expandTilde

bexec = _otk.bexec

textPropertyToString = _otk.textPropertyToString

itostring_unsigned_long = _otk.itostring_unsigned_long

itostring_long = _otk.itostring_long

itostring_unsigned = _otk.itostring_unsigned

itostring = _otk.itostring

basename = _otk.basename

X_PROTOCOL = _otk.X_PROTOCOL
X_PROTOCOL_REVISION = _otk.X_PROTOCOL_REVISION
None = _otk.None
ParentRelative = _otk.ParentRelative
CopyFromParent = _otk.CopyFromParent
PointerWindow = _otk.PointerWindow
InputFocus = _otk.InputFocus
PointerRoot = _otk.PointerRoot
AnyPropertyType = _otk.AnyPropertyType
AnyKey = _otk.AnyKey
AnyButton = _otk.AnyButton
AllTemporary = _otk.AllTemporary
CurrentTime = _otk.CurrentTime
NoSymbol = _otk.NoSymbol
NoEventMask = _otk.NoEventMask
KeyPressMask = _otk.KeyPressMask
KeyReleaseMask = _otk.KeyReleaseMask
ButtonPressMask = _otk.ButtonPressMask
ButtonReleaseMask = _otk.ButtonReleaseMask
EnterWindowMask = _otk.EnterWindowMask
LeaveWindowMask = _otk.LeaveWindowMask
PointerMotionMask = _otk.PointerMotionMask
PointerMotionHintMask = _otk.PointerMotionHintMask
Button1MotionMask = _otk.Button1MotionMask
Button2MotionMask = _otk.Button2MotionMask
Button3MotionMask = _otk.Button3MotionMask
Button4MotionMask = _otk.Button4MotionMask
Button5MotionMask = _otk.Button5MotionMask
ButtonMotionMask = _otk.ButtonMotionMask
KeymapStateMask = _otk.KeymapStateMask
ExposureMask = _otk.ExposureMask
VisibilityChangeMask = _otk.VisibilityChangeMask
StructureNotifyMask = _otk.StructureNotifyMask
ResizeRedirectMask = _otk.ResizeRedirectMask
SubstructureNotifyMask = _otk.SubstructureNotifyMask
SubstructureRedirectMask = _otk.SubstructureRedirectMask
FocusChangeMask = _otk.FocusChangeMask
PropertyChangeMask = _otk.PropertyChangeMask
ColormapChangeMask = _otk.ColormapChangeMask
OwnerGrabButtonMask = _otk.OwnerGrabButtonMask
KeyPress = _otk.KeyPress
KeyRelease = _otk.KeyRelease
ButtonPress = _otk.ButtonPress
ButtonRelease = _otk.ButtonRelease
MotionNotify = _otk.MotionNotify
EnterNotify = _otk.EnterNotify
LeaveNotify = _otk.LeaveNotify
FocusIn = _otk.FocusIn
FocusOut = _otk.FocusOut
KeymapNotify = _otk.KeymapNotify
Expose = _otk.Expose
GraphicsExpose = _otk.GraphicsExpose
NoExpose = _otk.NoExpose
VisibilityNotify = _otk.VisibilityNotify
CreateNotify = _otk.CreateNotify
DestroyNotify = _otk.DestroyNotify
UnmapNotify = _otk.UnmapNotify
MapNotify = _otk.MapNotify
MapRequest = _otk.MapRequest
ReparentNotify = _otk.ReparentNotify
ConfigureNotify = _otk.ConfigureNotify
ConfigureRequest = _otk.ConfigureRequest
GravityNotify = _otk.GravityNotify
ResizeRequest = _otk.ResizeRequest
CirculateNotify = _otk.CirculateNotify
CirculateRequest = _otk.CirculateRequest
PropertyNotify = _otk.PropertyNotify
SelectionClear = _otk.SelectionClear
SelectionRequest = _otk.SelectionRequest
SelectionNotify = _otk.SelectionNotify
ColormapNotify = _otk.ColormapNotify
ClientMessage = _otk.ClientMessage
MappingNotify = _otk.MappingNotify
LASTEvent = _otk.LASTEvent
ShiftMask = _otk.ShiftMask
LockMask = _otk.LockMask
ControlMask = _otk.ControlMask
Mod1Mask = _otk.Mod1Mask
Mod2Mask = _otk.Mod2Mask
Mod3Mask = _otk.Mod3Mask
Mod4Mask = _otk.Mod4Mask
Mod5Mask = _otk.Mod5Mask
ShiftMapIndex = _otk.ShiftMapIndex
LockMapIndex = _otk.LockMapIndex
ControlMapIndex = _otk.ControlMapIndex
Mod1MapIndex = _otk.Mod1MapIndex
Mod2MapIndex = _otk.Mod2MapIndex
Mod3MapIndex = _otk.Mod3MapIndex
Mod4MapIndex = _otk.Mod4MapIndex
Mod5MapIndex = _otk.Mod5MapIndex
Button1Mask = _otk.Button1Mask
Button2Mask = _otk.Button2Mask
Button3Mask = _otk.Button3Mask
Button4Mask = _otk.Button4Mask
Button5Mask = _otk.Button5Mask
AnyModifier = _otk.AnyModifier
Button1 = _otk.Button1
Button2 = _otk.Button2
Button3 = _otk.Button3
Button4 = _otk.Button4
Button5 = _otk.Button5
NotifyNormal = _otk.NotifyNormal
NotifyGrab = _otk.NotifyGrab
NotifyUngrab = _otk.NotifyUngrab
NotifyWhileGrabbed = _otk.NotifyWhileGrabbed
NotifyHint = _otk.NotifyHint
NotifyAncestor = _otk.NotifyAncestor
NotifyVirtual = _otk.NotifyVirtual
NotifyInferior = _otk.NotifyInferior
NotifyNonlinear = _otk.NotifyNonlinear
NotifyNonlinearVirtual = _otk.NotifyNonlinearVirtual
NotifyPointer = _otk.NotifyPointer
NotifyPointerRoot = _otk.NotifyPointerRoot
NotifyDetailNone = _otk.NotifyDetailNone
VisibilityUnobscured = _otk.VisibilityUnobscured
VisibilityPartiallyObscured = _otk.VisibilityPartiallyObscured
VisibilityFullyObscured = _otk.VisibilityFullyObscured
PlaceOnTop = _otk.PlaceOnTop
PlaceOnBottom = _otk.PlaceOnBottom
FamilyInternet = _otk.FamilyInternet
FamilyDECnet = _otk.FamilyDECnet
FamilyChaos = _otk.FamilyChaos
PropertyNewValue = _otk.PropertyNewValue
PropertyDelete = _otk.PropertyDelete
ColormapUninstalled = _otk.ColormapUninstalled
ColormapInstalled = _otk.ColormapInstalled
GrabModeSync = _otk.GrabModeSync
GrabModeAsync = _otk.GrabModeAsync
GrabSuccess = _otk.GrabSuccess
AlreadyGrabbed = _otk.AlreadyGrabbed
GrabInvalidTime = _otk.GrabInvalidTime
GrabNotViewable = _otk.GrabNotViewable
GrabFrozen = _otk.GrabFrozen
AsyncPointer = _otk.AsyncPointer
SyncPointer = _otk.SyncPointer
ReplayPointer = _otk.ReplayPointer
AsyncKeyboard = _otk.AsyncKeyboard
SyncKeyboard = _otk.SyncKeyboard
ReplayKeyboard = _otk.ReplayKeyboard
AsyncBoth = _otk.AsyncBoth
SyncBoth = _otk.SyncBoth
RevertToParent = _otk.RevertToParent
Success = _otk.Success
BadRequest = _otk.BadRequest
BadValue = _otk.BadValue
BadWindow = _otk.BadWindow
BadPixmap = _otk.BadPixmap
BadAtom = _otk.BadAtom
BadCursor = _otk.BadCursor
BadFont = _otk.BadFont
BadMatch = _otk.BadMatch
BadDrawable = _otk.BadDrawable
BadAccess = _otk.BadAccess
BadAlloc = _otk.BadAlloc
BadColor = _otk.BadColor
BadGC = _otk.BadGC
BadIDChoice = _otk.BadIDChoice
BadName = _otk.BadName
BadLength = _otk.BadLength
BadImplementation = _otk.BadImplementation
FirstExtensionError = _otk.FirstExtensionError
LastExtensionError = _otk.LastExtensionError
InputOutput = _otk.InputOutput
InputOnly = _otk.InputOnly
CWBackPixmap = _otk.CWBackPixmap
CWBackPixel = _otk.CWBackPixel
CWBorderPixmap = _otk.CWBorderPixmap
CWBorderPixel = _otk.CWBorderPixel
CWBitGravity = _otk.CWBitGravity
CWWinGravity = _otk.CWWinGravity
CWBackingStore = _otk.CWBackingStore
CWBackingPlanes = _otk.CWBackingPlanes
CWBackingPixel = _otk.CWBackingPixel
CWOverrideRedirect = _otk.CWOverrideRedirect
CWSaveUnder = _otk.CWSaveUnder
CWEventMask = _otk.CWEventMask
CWDontPropagate = _otk.CWDontPropagate
CWColormap = _otk.CWColormap
CWCursor = _otk.CWCursor
CWX = _otk.CWX
CWY = _otk.CWY
CWWidth = _otk.CWWidth
CWHeight = _otk.CWHeight
CWBorderWidth = _otk.CWBorderWidth
CWSibling = _otk.CWSibling
CWStackMode = _otk.CWStackMode
ForgetGravity = _otk.ForgetGravity
NorthWestGravity = _otk.NorthWestGravity
NorthGravity = _otk.NorthGravity
NorthEastGravity = _otk.NorthEastGravity
WestGravity = _otk.WestGravity
CenterGravity = _otk.CenterGravity
EastGravity = _otk.EastGravity
SouthWestGravity = _otk.SouthWestGravity
SouthGravity = _otk.SouthGravity
SouthEastGravity = _otk.SouthEastGravity
StaticGravity = _otk.StaticGravity
UnmapGravity = _otk.UnmapGravity
NotUseful = _otk.NotUseful
WhenMapped = _otk.WhenMapped
Always = _otk.Always
IsUnmapped = _otk.IsUnmapped
IsUnviewable = _otk.IsUnviewable
IsViewable = _otk.IsViewable
SetModeInsert = _otk.SetModeInsert
SetModeDelete = _otk.SetModeDelete
DestroyAll = _otk.DestroyAll
RetainPermanent = _otk.RetainPermanent
RetainTemporary = _otk.RetainTemporary
Above = _otk.Above
Below = _otk.Below
TopIf = _otk.TopIf
BottomIf = _otk.BottomIf
Opposite = _otk.Opposite
RaiseLowest = _otk.RaiseLowest
LowerHighest = _otk.LowerHighest
PropModeReplace = _otk.PropModeReplace
PropModePrepend = _otk.PropModePrepend
PropModeAppend = _otk.PropModeAppend
GXclear = _otk.GXclear
GXand = _otk.GXand
GXandReverse = _otk.GXandReverse
GXcopy = _otk.GXcopy
GXandInverted = _otk.GXandInverted
GXnoop = _otk.GXnoop
GXxor = _otk.GXxor
GXor = _otk.GXor
GXnor = _otk.GXnor
GXequiv = _otk.GXequiv
GXinvert = _otk.GXinvert
GXorReverse = _otk.GXorReverse
GXcopyInverted = _otk.GXcopyInverted
GXorInverted = _otk.GXorInverted
GXnand = _otk.GXnand
GXset = _otk.GXset
LineSolid = _otk.LineSolid
LineOnOffDash = _otk.LineOnOffDash
LineDoubleDash = _otk.LineDoubleDash
CapNotLast = _otk.CapNotLast
CapButt = _otk.CapButt
CapRound = _otk.CapRound
CapProjecting = _otk.CapProjecting
JoinMiter = _otk.JoinMiter
JoinRound = _otk.JoinRound
JoinBevel = _otk.JoinBevel
FillSolid = _otk.FillSolid
FillTiled = _otk.FillTiled
FillStippled = _otk.FillStippled
FillOpaqueStippled = _otk.FillOpaqueStippled
EvenOddRule = _otk.EvenOddRule
WindingRule = _otk.WindingRule
ClipByChildren = _otk.ClipByChildren
IncludeInferiors = _otk.IncludeInferiors
Unsorted = _otk.Unsorted
YSorted = _otk.YSorted
YXSorted = _otk.YXSorted
YXBanded = _otk.YXBanded
CoordModeOrigin = _otk.CoordModeOrigin
CoordModePrevious = _otk.CoordModePrevious
Complex = _otk.Complex
Nonconvex = _otk.Nonconvex
Convex = _otk.Convex
ArcChord = _otk.ArcChord
ArcPieSlice = _otk.ArcPieSlice
GCFunction = _otk.GCFunction
GCPlaneMask = _otk.GCPlaneMask
GCForeground = _otk.GCForeground
GCBackground = _otk.GCBackground
GCLineWidth = _otk.GCLineWidth
GCLineStyle = _otk.GCLineStyle
GCCapStyle = _otk.GCCapStyle
GCJoinStyle = _otk.GCJoinStyle
GCFillStyle = _otk.GCFillStyle
GCFillRule = _otk.GCFillRule
GCTile = _otk.GCTile
GCStipple = _otk.GCStipple
GCTileStipXOrigin = _otk.GCTileStipXOrigin
GCTileStipYOrigin = _otk.GCTileStipYOrigin
GCFont = _otk.GCFont
GCSubwindowMode = _otk.GCSubwindowMode
GCGraphicsExposures = _otk.GCGraphicsExposures
GCClipXOrigin = _otk.GCClipXOrigin
GCClipYOrigin = _otk.GCClipYOrigin
GCClipMask = _otk.GCClipMask
GCDashOffset = _otk.GCDashOffset
GCDashList = _otk.GCDashList
GCArcMode = _otk.GCArcMode
GCLastBit = _otk.GCLastBit
FontLeftToRight = _otk.FontLeftToRight
FontRightToLeft = _otk.FontRightToLeft
FontChange = _otk.FontChange
XYBitmap = _otk.XYBitmap
XYPixmap = _otk.XYPixmap
ZPixmap = _otk.ZPixmap
AllocNone = _otk.AllocNone
AllocAll = _otk.AllocAll
DoRed = _otk.DoRed
DoGreen = _otk.DoGreen
DoBlue = _otk.DoBlue
CursorShape = _otk.CursorShape
TileShape = _otk.TileShape
StippleShape = _otk.StippleShape
AutoRepeatModeOff = _otk.AutoRepeatModeOff
AutoRepeatModeOn = _otk.AutoRepeatModeOn
AutoRepeatModeDefault = _otk.AutoRepeatModeDefault
LedModeOff = _otk.LedModeOff
LedModeOn = _otk.LedModeOn
KBKeyClickPercent = _otk.KBKeyClickPercent
KBBellPercent = _otk.KBBellPercent
KBBellPitch = _otk.KBBellPitch
KBBellDuration = _otk.KBBellDuration
KBLed = _otk.KBLed
KBLedMode = _otk.KBLedMode
KBKey = _otk.KBKey
KBAutoRepeatMode = _otk.KBAutoRepeatMode
MappingSuccess = _otk.MappingSuccess
MappingBusy = _otk.MappingBusy
MappingFailed = _otk.MappingFailed
MappingModifier = _otk.MappingModifier
MappingKeyboard = _otk.MappingKeyboard
MappingPointer = _otk.MappingPointer
DontPreferBlanking = _otk.DontPreferBlanking
PreferBlanking = _otk.PreferBlanking
DefaultBlanking = _otk.DefaultBlanking
DisableScreenSaver = _otk.DisableScreenSaver
DisableScreenInterval = _otk.DisableScreenInterval
DontAllowExposures = _otk.DontAllowExposures
AllowExposures = _otk.AllowExposures
DefaultExposures = _otk.DefaultExposures
ScreenSaverReset = _otk.ScreenSaverReset
ScreenSaverActive = _otk.ScreenSaverActive
HostInsert = _otk.HostInsert
HostDelete = _otk.HostDelete
EnableAccess = _otk.EnableAccess
DisableAccess = _otk.DisableAccess
StaticGray = _otk.StaticGray
GrayScale = _otk.GrayScale
StaticColor = _otk.StaticColor
PseudoColor = _otk.PseudoColor
TrueColor = _otk.TrueColor
DirectColor = _otk.DirectColor
LSBFirst = _otk.LSBFirst
MSBFirst = _otk.MSBFirst
cvar = _otk.cvar
BSENTINEL = cvar.BSENTINEL

