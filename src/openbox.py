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

Display_instance = _openbox.Display_instance

Property_atoms = _openbox.Property_atoms

class Display(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Display, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Display, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_Display,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_Display):
        try:
            if self.thisown: destroy(self)
        except: pass
    def screenInfo(*args): return apply(_openbox.Display_screenInfo,args)
    def findScreen(*args): return apply(_openbox.Display_findScreen,args)
    def renderControl(*args): return apply(_openbox.Display_renderControl,args)
    def xkb(*args): return apply(_openbox.Display_xkb,args)
    def xkbEventBase(*args): return apply(_openbox.Display_xkbEventBase,args)
    def shape(*args): return apply(_openbox.Display_shape,args)
    def shapeEventBase(*args): return apply(_openbox.Display_shapeEventBase,args)
    def xinerama(*args): return apply(_openbox.Display_xinerama,args)
    def numLockMask(*args): return apply(_openbox.Display_numLockMask,args)
    def scrollLockMask(*args): return apply(_openbox.Display_scrollLockMask,args)
    def __mul__(*args): return apply(_openbox.Display___mul__,args)
    def grab(*args): return apply(_openbox.Display_grab,args)
    def ungrab(*args): return apply(_openbox.Display_ungrab,args)
    def grabButton(*args): return apply(_openbox.Display_grabButton,args)
    def ungrabButton(*args): return apply(_openbox.Display_ungrabButton,args)
    def grabKey(*args): return apply(_openbox.Display_grabKey,args)
    def ungrabKey(*args): return apply(_openbox.Display_ungrabKey,args)
    def __repr__(self):
        return "<C Display instance at %s>" % (self.this,)

class DisplayPtr(Display):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Display
_openbox.Display_swigregister(DisplayPtr)

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

class Atoms(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Atoms, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Atoms, name)
    __swig_setmethods__["cardinal"] = _openbox.Atoms_cardinal_set
    __swig_getmethods__["cardinal"] = _openbox.Atoms_cardinal_get
    if _newclass:cardinal = property(_openbox.Atoms_cardinal_get,_openbox.Atoms_cardinal_set)
    __swig_setmethods__["window"] = _openbox.Atoms_window_set
    __swig_getmethods__["window"] = _openbox.Atoms_window_get
    if _newclass:window = property(_openbox.Atoms_window_get,_openbox.Atoms_window_set)
    __swig_setmethods__["pixmap"] = _openbox.Atoms_pixmap_set
    __swig_getmethods__["pixmap"] = _openbox.Atoms_pixmap_get
    if _newclass:pixmap = property(_openbox.Atoms_pixmap_get,_openbox.Atoms_pixmap_set)
    __swig_setmethods__["atom"] = _openbox.Atoms_atom_set
    __swig_getmethods__["atom"] = _openbox.Atoms_atom_get
    if _newclass:atom = property(_openbox.Atoms_atom_get,_openbox.Atoms_atom_set)
    __swig_setmethods__["string"] = _openbox.Atoms_string_set
    __swig_getmethods__["string"] = _openbox.Atoms_string_get
    if _newclass:string = property(_openbox.Atoms_string_get,_openbox.Atoms_string_set)
    __swig_setmethods__["utf8"] = _openbox.Atoms_utf8_set
    __swig_getmethods__["utf8"] = _openbox.Atoms_utf8_get
    if _newclass:utf8 = property(_openbox.Atoms_utf8_get,_openbox.Atoms_utf8_set)
    __swig_setmethods__["openbox_pid"] = _openbox.Atoms_openbox_pid_set
    __swig_getmethods__["openbox_pid"] = _openbox.Atoms_openbox_pid_get
    if _newclass:openbox_pid = property(_openbox.Atoms_openbox_pid_get,_openbox.Atoms_openbox_pid_set)
    __swig_setmethods__["wm_colormap_windows"] = _openbox.Atoms_wm_colormap_windows_set
    __swig_getmethods__["wm_colormap_windows"] = _openbox.Atoms_wm_colormap_windows_get
    if _newclass:wm_colormap_windows = property(_openbox.Atoms_wm_colormap_windows_get,_openbox.Atoms_wm_colormap_windows_set)
    __swig_setmethods__["wm_protocols"] = _openbox.Atoms_wm_protocols_set
    __swig_getmethods__["wm_protocols"] = _openbox.Atoms_wm_protocols_get
    if _newclass:wm_protocols = property(_openbox.Atoms_wm_protocols_get,_openbox.Atoms_wm_protocols_set)
    __swig_setmethods__["wm_state"] = _openbox.Atoms_wm_state_set
    __swig_getmethods__["wm_state"] = _openbox.Atoms_wm_state_get
    if _newclass:wm_state = property(_openbox.Atoms_wm_state_get,_openbox.Atoms_wm_state_set)
    __swig_setmethods__["wm_delete_window"] = _openbox.Atoms_wm_delete_window_set
    __swig_getmethods__["wm_delete_window"] = _openbox.Atoms_wm_delete_window_get
    if _newclass:wm_delete_window = property(_openbox.Atoms_wm_delete_window_get,_openbox.Atoms_wm_delete_window_set)
    __swig_setmethods__["wm_take_focus"] = _openbox.Atoms_wm_take_focus_set
    __swig_getmethods__["wm_take_focus"] = _openbox.Atoms_wm_take_focus_get
    if _newclass:wm_take_focus = property(_openbox.Atoms_wm_take_focus_get,_openbox.Atoms_wm_take_focus_set)
    __swig_setmethods__["wm_change_state"] = _openbox.Atoms_wm_change_state_set
    __swig_getmethods__["wm_change_state"] = _openbox.Atoms_wm_change_state_get
    if _newclass:wm_change_state = property(_openbox.Atoms_wm_change_state_get,_openbox.Atoms_wm_change_state_set)
    __swig_setmethods__["wm_name"] = _openbox.Atoms_wm_name_set
    __swig_getmethods__["wm_name"] = _openbox.Atoms_wm_name_get
    if _newclass:wm_name = property(_openbox.Atoms_wm_name_get,_openbox.Atoms_wm_name_set)
    __swig_setmethods__["wm_icon_name"] = _openbox.Atoms_wm_icon_name_set
    __swig_getmethods__["wm_icon_name"] = _openbox.Atoms_wm_icon_name_get
    if _newclass:wm_icon_name = property(_openbox.Atoms_wm_icon_name_get,_openbox.Atoms_wm_icon_name_set)
    __swig_setmethods__["wm_class"] = _openbox.Atoms_wm_class_set
    __swig_getmethods__["wm_class"] = _openbox.Atoms_wm_class_get
    if _newclass:wm_class = property(_openbox.Atoms_wm_class_get,_openbox.Atoms_wm_class_set)
    __swig_setmethods__["wm_window_role"] = _openbox.Atoms_wm_window_role_set
    __swig_getmethods__["wm_window_role"] = _openbox.Atoms_wm_window_role_get
    if _newclass:wm_window_role = property(_openbox.Atoms_wm_window_role_get,_openbox.Atoms_wm_window_role_set)
    __swig_setmethods__["motif_wm_hints"] = _openbox.Atoms_motif_wm_hints_set
    __swig_getmethods__["motif_wm_hints"] = _openbox.Atoms_motif_wm_hints_get
    if _newclass:motif_wm_hints = property(_openbox.Atoms_motif_wm_hints_get,_openbox.Atoms_motif_wm_hints_set)
    __swig_setmethods__["openbox_show_root_menu"] = _openbox.Atoms_openbox_show_root_menu_set
    __swig_getmethods__["openbox_show_root_menu"] = _openbox.Atoms_openbox_show_root_menu_get
    if _newclass:openbox_show_root_menu = property(_openbox.Atoms_openbox_show_root_menu_get,_openbox.Atoms_openbox_show_root_menu_set)
    __swig_setmethods__["openbox_show_workspace_menu"] = _openbox.Atoms_openbox_show_workspace_menu_set
    __swig_getmethods__["openbox_show_workspace_menu"] = _openbox.Atoms_openbox_show_workspace_menu_get
    if _newclass:openbox_show_workspace_menu = property(_openbox.Atoms_openbox_show_workspace_menu_get,_openbox.Atoms_openbox_show_workspace_menu_set)
    __swig_setmethods__["net_supported"] = _openbox.Atoms_net_supported_set
    __swig_getmethods__["net_supported"] = _openbox.Atoms_net_supported_get
    if _newclass:net_supported = property(_openbox.Atoms_net_supported_get,_openbox.Atoms_net_supported_set)
    __swig_setmethods__["net_client_list"] = _openbox.Atoms_net_client_list_set
    __swig_getmethods__["net_client_list"] = _openbox.Atoms_net_client_list_get
    if _newclass:net_client_list = property(_openbox.Atoms_net_client_list_get,_openbox.Atoms_net_client_list_set)
    __swig_setmethods__["net_client_list_stacking"] = _openbox.Atoms_net_client_list_stacking_set
    __swig_getmethods__["net_client_list_stacking"] = _openbox.Atoms_net_client_list_stacking_get
    if _newclass:net_client_list_stacking = property(_openbox.Atoms_net_client_list_stacking_get,_openbox.Atoms_net_client_list_stacking_set)
    __swig_setmethods__["net_number_of_desktops"] = _openbox.Atoms_net_number_of_desktops_set
    __swig_getmethods__["net_number_of_desktops"] = _openbox.Atoms_net_number_of_desktops_get
    if _newclass:net_number_of_desktops = property(_openbox.Atoms_net_number_of_desktops_get,_openbox.Atoms_net_number_of_desktops_set)
    __swig_setmethods__["net_desktop_geometry"] = _openbox.Atoms_net_desktop_geometry_set
    __swig_getmethods__["net_desktop_geometry"] = _openbox.Atoms_net_desktop_geometry_get
    if _newclass:net_desktop_geometry = property(_openbox.Atoms_net_desktop_geometry_get,_openbox.Atoms_net_desktop_geometry_set)
    __swig_setmethods__["net_desktop_viewport"] = _openbox.Atoms_net_desktop_viewport_set
    __swig_getmethods__["net_desktop_viewport"] = _openbox.Atoms_net_desktop_viewport_get
    if _newclass:net_desktop_viewport = property(_openbox.Atoms_net_desktop_viewport_get,_openbox.Atoms_net_desktop_viewport_set)
    __swig_setmethods__["net_current_desktop"] = _openbox.Atoms_net_current_desktop_set
    __swig_getmethods__["net_current_desktop"] = _openbox.Atoms_net_current_desktop_get
    if _newclass:net_current_desktop = property(_openbox.Atoms_net_current_desktop_get,_openbox.Atoms_net_current_desktop_set)
    __swig_setmethods__["net_desktop_names"] = _openbox.Atoms_net_desktop_names_set
    __swig_getmethods__["net_desktop_names"] = _openbox.Atoms_net_desktop_names_get
    if _newclass:net_desktop_names = property(_openbox.Atoms_net_desktop_names_get,_openbox.Atoms_net_desktop_names_set)
    __swig_setmethods__["net_active_window"] = _openbox.Atoms_net_active_window_set
    __swig_getmethods__["net_active_window"] = _openbox.Atoms_net_active_window_get
    if _newclass:net_active_window = property(_openbox.Atoms_net_active_window_get,_openbox.Atoms_net_active_window_set)
    __swig_setmethods__["net_workarea"] = _openbox.Atoms_net_workarea_set
    __swig_getmethods__["net_workarea"] = _openbox.Atoms_net_workarea_get
    if _newclass:net_workarea = property(_openbox.Atoms_net_workarea_get,_openbox.Atoms_net_workarea_set)
    __swig_setmethods__["net_supporting_wm_check"] = _openbox.Atoms_net_supporting_wm_check_set
    __swig_getmethods__["net_supporting_wm_check"] = _openbox.Atoms_net_supporting_wm_check_get
    if _newclass:net_supporting_wm_check = property(_openbox.Atoms_net_supporting_wm_check_get,_openbox.Atoms_net_supporting_wm_check_set)
    __swig_setmethods__["net_close_window"] = _openbox.Atoms_net_close_window_set
    __swig_getmethods__["net_close_window"] = _openbox.Atoms_net_close_window_get
    if _newclass:net_close_window = property(_openbox.Atoms_net_close_window_get,_openbox.Atoms_net_close_window_set)
    __swig_setmethods__["net_wm_moveresize"] = _openbox.Atoms_net_wm_moveresize_set
    __swig_getmethods__["net_wm_moveresize"] = _openbox.Atoms_net_wm_moveresize_get
    if _newclass:net_wm_moveresize = property(_openbox.Atoms_net_wm_moveresize_get,_openbox.Atoms_net_wm_moveresize_set)
    __swig_setmethods__["net_wm_name"] = _openbox.Atoms_net_wm_name_set
    __swig_getmethods__["net_wm_name"] = _openbox.Atoms_net_wm_name_get
    if _newclass:net_wm_name = property(_openbox.Atoms_net_wm_name_get,_openbox.Atoms_net_wm_name_set)
    __swig_setmethods__["net_wm_visible_name"] = _openbox.Atoms_net_wm_visible_name_set
    __swig_getmethods__["net_wm_visible_name"] = _openbox.Atoms_net_wm_visible_name_get
    if _newclass:net_wm_visible_name = property(_openbox.Atoms_net_wm_visible_name_get,_openbox.Atoms_net_wm_visible_name_set)
    __swig_setmethods__["net_wm_icon_name"] = _openbox.Atoms_net_wm_icon_name_set
    __swig_getmethods__["net_wm_icon_name"] = _openbox.Atoms_net_wm_icon_name_get
    if _newclass:net_wm_icon_name = property(_openbox.Atoms_net_wm_icon_name_get,_openbox.Atoms_net_wm_icon_name_set)
    __swig_setmethods__["net_wm_visible_icon_name"] = _openbox.Atoms_net_wm_visible_icon_name_set
    __swig_getmethods__["net_wm_visible_icon_name"] = _openbox.Atoms_net_wm_visible_icon_name_get
    if _newclass:net_wm_visible_icon_name = property(_openbox.Atoms_net_wm_visible_icon_name_get,_openbox.Atoms_net_wm_visible_icon_name_set)
    __swig_setmethods__["net_wm_desktop"] = _openbox.Atoms_net_wm_desktop_set
    __swig_getmethods__["net_wm_desktop"] = _openbox.Atoms_net_wm_desktop_get
    if _newclass:net_wm_desktop = property(_openbox.Atoms_net_wm_desktop_get,_openbox.Atoms_net_wm_desktop_set)
    __swig_setmethods__["net_wm_window_type"] = _openbox.Atoms_net_wm_window_type_set
    __swig_getmethods__["net_wm_window_type"] = _openbox.Atoms_net_wm_window_type_get
    if _newclass:net_wm_window_type = property(_openbox.Atoms_net_wm_window_type_get,_openbox.Atoms_net_wm_window_type_set)
    __swig_setmethods__["net_wm_state"] = _openbox.Atoms_net_wm_state_set
    __swig_getmethods__["net_wm_state"] = _openbox.Atoms_net_wm_state_get
    if _newclass:net_wm_state = property(_openbox.Atoms_net_wm_state_get,_openbox.Atoms_net_wm_state_set)
    __swig_setmethods__["net_wm_strut"] = _openbox.Atoms_net_wm_strut_set
    __swig_getmethods__["net_wm_strut"] = _openbox.Atoms_net_wm_strut_get
    if _newclass:net_wm_strut = property(_openbox.Atoms_net_wm_strut_get,_openbox.Atoms_net_wm_strut_set)
    __swig_setmethods__["net_wm_allowed_actions"] = _openbox.Atoms_net_wm_allowed_actions_set
    __swig_getmethods__["net_wm_allowed_actions"] = _openbox.Atoms_net_wm_allowed_actions_get
    if _newclass:net_wm_allowed_actions = property(_openbox.Atoms_net_wm_allowed_actions_get,_openbox.Atoms_net_wm_allowed_actions_set)
    __swig_setmethods__["net_wm_window_type_desktop"] = _openbox.Atoms_net_wm_window_type_desktop_set
    __swig_getmethods__["net_wm_window_type_desktop"] = _openbox.Atoms_net_wm_window_type_desktop_get
    if _newclass:net_wm_window_type_desktop = property(_openbox.Atoms_net_wm_window_type_desktop_get,_openbox.Atoms_net_wm_window_type_desktop_set)
    __swig_setmethods__["net_wm_window_type_dock"] = _openbox.Atoms_net_wm_window_type_dock_set
    __swig_getmethods__["net_wm_window_type_dock"] = _openbox.Atoms_net_wm_window_type_dock_get
    if _newclass:net_wm_window_type_dock = property(_openbox.Atoms_net_wm_window_type_dock_get,_openbox.Atoms_net_wm_window_type_dock_set)
    __swig_setmethods__["net_wm_window_type_toolbar"] = _openbox.Atoms_net_wm_window_type_toolbar_set
    __swig_getmethods__["net_wm_window_type_toolbar"] = _openbox.Atoms_net_wm_window_type_toolbar_get
    if _newclass:net_wm_window_type_toolbar = property(_openbox.Atoms_net_wm_window_type_toolbar_get,_openbox.Atoms_net_wm_window_type_toolbar_set)
    __swig_setmethods__["net_wm_window_type_menu"] = _openbox.Atoms_net_wm_window_type_menu_set
    __swig_getmethods__["net_wm_window_type_menu"] = _openbox.Atoms_net_wm_window_type_menu_get
    if _newclass:net_wm_window_type_menu = property(_openbox.Atoms_net_wm_window_type_menu_get,_openbox.Atoms_net_wm_window_type_menu_set)
    __swig_setmethods__["net_wm_window_type_utility"] = _openbox.Atoms_net_wm_window_type_utility_set
    __swig_getmethods__["net_wm_window_type_utility"] = _openbox.Atoms_net_wm_window_type_utility_get
    if _newclass:net_wm_window_type_utility = property(_openbox.Atoms_net_wm_window_type_utility_get,_openbox.Atoms_net_wm_window_type_utility_set)
    __swig_setmethods__["net_wm_window_type_splash"] = _openbox.Atoms_net_wm_window_type_splash_set
    __swig_getmethods__["net_wm_window_type_splash"] = _openbox.Atoms_net_wm_window_type_splash_get
    if _newclass:net_wm_window_type_splash = property(_openbox.Atoms_net_wm_window_type_splash_get,_openbox.Atoms_net_wm_window_type_splash_set)
    __swig_setmethods__["net_wm_window_type_dialog"] = _openbox.Atoms_net_wm_window_type_dialog_set
    __swig_getmethods__["net_wm_window_type_dialog"] = _openbox.Atoms_net_wm_window_type_dialog_get
    if _newclass:net_wm_window_type_dialog = property(_openbox.Atoms_net_wm_window_type_dialog_get,_openbox.Atoms_net_wm_window_type_dialog_set)
    __swig_setmethods__["net_wm_window_type_normal"] = _openbox.Atoms_net_wm_window_type_normal_set
    __swig_getmethods__["net_wm_window_type_normal"] = _openbox.Atoms_net_wm_window_type_normal_get
    if _newclass:net_wm_window_type_normal = property(_openbox.Atoms_net_wm_window_type_normal_get,_openbox.Atoms_net_wm_window_type_normal_set)
    __swig_setmethods__["net_wm_moveresize_size_topleft"] = _openbox.Atoms_net_wm_moveresize_size_topleft_set
    __swig_getmethods__["net_wm_moveresize_size_topleft"] = _openbox.Atoms_net_wm_moveresize_size_topleft_get
    if _newclass:net_wm_moveresize_size_topleft = property(_openbox.Atoms_net_wm_moveresize_size_topleft_get,_openbox.Atoms_net_wm_moveresize_size_topleft_set)
    __swig_setmethods__["net_wm_moveresize_size_topright"] = _openbox.Atoms_net_wm_moveresize_size_topright_set
    __swig_getmethods__["net_wm_moveresize_size_topright"] = _openbox.Atoms_net_wm_moveresize_size_topright_get
    if _newclass:net_wm_moveresize_size_topright = property(_openbox.Atoms_net_wm_moveresize_size_topright_get,_openbox.Atoms_net_wm_moveresize_size_topright_set)
    __swig_setmethods__["net_wm_moveresize_size_bottomleft"] = _openbox.Atoms_net_wm_moveresize_size_bottomleft_set
    __swig_getmethods__["net_wm_moveresize_size_bottomleft"] = _openbox.Atoms_net_wm_moveresize_size_bottomleft_get
    if _newclass:net_wm_moveresize_size_bottomleft = property(_openbox.Atoms_net_wm_moveresize_size_bottomleft_get,_openbox.Atoms_net_wm_moveresize_size_bottomleft_set)
    __swig_setmethods__["net_wm_moveresize_size_bottomright"] = _openbox.Atoms_net_wm_moveresize_size_bottomright_set
    __swig_getmethods__["net_wm_moveresize_size_bottomright"] = _openbox.Atoms_net_wm_moveresize_size_bottomright_get
    if _newclass:net_wm_moveresize_size_bottomright = property(_openbox.Atoms_net_wm_moveresize_size_bottomright_get,_openbox.Atoms_net_wm_moveresize_size_bottomright_set)
    __swig_setmethods__["net_wm_moveresize_move"] = _openbox.Atoms_net_wm_moveresize_move_set
    __swig_getmethods__["net_wm_moveresize_move"] = _openbox.Atoms_net_wm_moveresize_move_get
    if _newclass:net_wm_moveresize_move = property(_openbox.Atoms_net_wm_moveresize_move_get,_openbox.Atoms_net_wm_moveresize_move_set)
    __swig_setmethods__["net_wm_action_move"] = _openbox.Atoms_net_wm_action_move_set
    __swig_getmethods__["net_wm_action_move"] = _openbox.Atoms_net_wm_action_move_get
    if _newclass:net_wm_action_move = property(_openbox.Atoms_net_wm_action_move_get,_openbox.Atoms_net_wm_action_move_set)
    __swig_setmethods__["net_wm_action_resize"] = _openbox.Atoms_net_wm_action_resize_set
    __swig_getmethods__["net_wm_action_resize"] = _openbox.Atoms_net_wm_action_resize_get
    if _newclass:net_wm_action_resize = property(_openbox.Atoms_net_wm_action_resize_get,_openbox.Atoms_net_wm_action_resize_set)
    __swig_setmethods__["net_wm_action_minimize"] = _openbox.Atoms_net_wm_action_minimize_set
    __swig_getmethods__["net_wm_action_minimize"] = _openbox.Atoms_net_wm_action_minimize_get
    if _newclass:net_wm_action_minimize = property(_openbox.Atoms_net_wm_action_minimize_get,_openbox.Atoms_net_wm_action_minimize_set)
    __swig_setmethods__["net_wm_action_shade"] = _openbox.Atoms_net_wm_action_shade_set
    __swig_getmethods__["net_wm_action_shade"] = _openbox.Atoms_net_wm_action_shade_get
    if _newclass:net_wm_action_shade = property(_openbox.Atoms_net_wm_action_shade_get,_openbox.Atoms_net_wm_action_shade_set)
    __swig_setmethods__["net_wm_action_stick"] = _openbox.Atoms_net_wm_action_stick_set
    __swig_getmethods__["net_wm_action_stick"] = _openbox.Atoms_net_wm_action_stick_get
    if _newclass:net_wm_action_stick = property(_openbox.Atoms_net_wm_action_stick_get,_openbox.Atoms_net_wm_action_stick_set)
    __swig_setmethods__["net_wm_action_maximize_horz"] = _openbox.Atoms_net_wm_action_maximize_horz_set
    __swig_getmethods__["net_wm_action_maximize_horz"] = _openbox.Atoms_net_wm_action_maximize_horz_get
    if _newclass:net_wm_action_maximize_horz = property(_openbox.Atoms_net_wm_action_maximize_horz_get,_openbox.Atoms_net_wm_action_maximize_horz_set)
    __swig_setmethods__["net_wm_action_maximize_vert"] = _openbox.Atoms_net_wm_action_maximize_vert_set
    __swig_getmethods__["net_wm_action_maximize_vert"] = _openbox.Atoms_net_wm_action_maximize_vert_get
    if _newclass:net_wm_action_maximize_vert = property(_openbox.Atoms_net_wm_action_maximize_vert_get,_openbox.Atoms_net_wm_action_maximize_vert_set)
    __swig_setmethods__["net_wm_action_fullscreen"] = _openbox.Atoms_net_wm_action_fullscreen_set
    __swig_getmethods__["net_wm_action_fullscreen"] = _openbox.Atoms_net_wm_action_fullscreen_get
    if _newclass:net_wm_action_fullscreen = property(_openbox.Atoms_net_wm_action_fullscreen_get,_openbox.Atoms_net_wm_action_fullscreen_set)
    __swig_setmethods__["net_wm_action_change_desktop"] = _openbox.Atoms_net_wm_action_change_desktop_set
    __swig_getmethods__["net_wm_action_change_desktop"] = _openbox.Atoms_net_wm_action_change_desktop_get
    if _newclass:net_wm_action_change_desktop = property(_openbox.Atoms_net_wm_action_change_desktop_get,_openbox.Atoms_net_wm_action_change_desktop_set)
    __swig_setmethods__["net_wm_action_close"] = _openbox.Atoms_net_wm_action_close_set
    __swig_getmethods__["net_wm_action_close"] = _openbox.Atoms_net_wm_action_close_get
    if _newclass:net_wm_action_close = property(_openbox.Atoms_net_wm_action_close_get,_openbox.Atoms_net_wm_action_close_set)
    __swig_setmethods__["net_wm_state_modal"] = _openbox.Atoms_net_wm_state_modal_set
    __swig_getmethods__["net_wm_state_modal"] = _openbox.Atoms_net_wm_state_modal_get
    if _newclass:net_wm_state_modal = property(_openbox.Atoms_net_wm_state_modal_get,_openbox.Atoms_net_wm_state_modal_set)
    __swig_setmethods__["net_wm_state_sticky"] = _openbox.Atoms_net_wm_state_sticky_set
    __swig_getmethods__["net_wm_state_sticky"] = _openbox.Atoms_net_wm_state_sticky_get
    if _newclass:net_wm_state_sticky = property(_openbox.Atoms_net_wm_state_sticky_get,_openbox.Atoms_net_wm_state_sticky_set)
    __swig_setmethods__["net_wm_state_maximized_vert"] = _openbox.Atoms_net_wm_state_maximized_vert_set
    __swig_getmethods__["net_wm_state_maximized_vert"] = _openbox.Atoms_net_wm_state_maximized_vert_get
    if _newclass:net_wm_state_maximized_vert = property(_openbox.Atoms_net_wm_state_maximized_vert_get,_openbox.Atoms_net_wm_state_maximized_vert_set)
    __swig_setmethods__["net_wm_state_maximized_horz"] = _openbox.Atoms_net_wm_state_maximized_horz_set
    __swig_getmethods__["net_wm_state_maximized_horz"] = _openbox.Atoms_net_wm_state_maximized_horz_get
    if _newclass:net_wm_state_maximized_horz = property(_openbox.Atoms_net_wm_state_maximized_horz_get,_openbox.Atoms_net_wm_state_maximized_horz_set)
    __swig_setmethods__["net_wm_state_shaded"] = _openbox.Atoms_net_wm_state_shaded_set
    __swig_getmethods__["net_wm_state_shaded"] = _openbox.Atoms_net_wm_state_shaded_get
    if _newclass:net_wm_state_shaded = property(_openbox.Atoms_net_wm_state_shaded_get,_openbox.Atoms_net_wm_state_shaded_set)
    __swig_setmethods__["net_wm_state_skip_taskbar"] = _openbox.Atoms_net_wm_state_skip_taskbar_set
    __swig_getmethods__["net_wm_state_skip_taskbar"] = _openbox.Atoms_net_wm_state_skip_taskbar_get
    if _newclass:net_wm_state_skip_taskbar = property(_openbox.Atoms_net_wm_state_skip_taskbar_get,_openbox.Atoms_net_wm_state_skip_taskbar_set)
    __swig_setmethods__["net_wm_state_skip_pager"] = _openbox.Atoms_net_wm_state_skip_pager_set
    __swig_getmethods__["net_wm_state_skip_pager"] = _openbox.Atoms_net_wm_state_skip_pager_get
    if _newclass:net_wm_state_skip_pager = property(_openbox.Atoms_net_wm_state_skip_pager_get,_openbox.Atoms_net_wm_state_skip_pager_set)
    __swig_setmethods__["net_wm_state_hidden"] = _openbox.Atoms_net_wm_state_hidden_set
    __swig_getmethods__["net_wm_state_hidden"] = _openbox.Atoms_net_wm_state_hidden_get
    if _newclass:net_wm_state_hidden = property(_openbox.Atoms_net_wm_state_hidden_get,_openbox.Atoms_net_wm_state_hidden_set)
    __swig_setmethods__["net_wm_state_fullscreen"] = _openbox.Atoms_net_wm_state_fullscreen_set
    __swig_getmethods__["net_wm_state_fullscreen"] = _openbox.Atoms_net_wm_state_fullscreen_get
    if _newclass:net_wm_state_fullscreen = property(_openbox.Atoms_net_wm_state_fullscreen_get,_openbox.Atoms_net_wm_state_fullscreen_set)
    __swig_setmethods__["net_wm_state_above"] = _openbox.Atoms_net_wm_state_above_set
    __swig_getmethods__["net_wm_state_above"] = _openbox.Atoms_net_wm_state_above_get
    if _newclass:net_wm_state_above = property(_openbox.Atoms_net_wm_state_above_get,_openbox.Atoms_net_wm_state_above_set)
    __swig_setmethods__["net_wm_state_below"] = _openbox.Atoms_net_wm_state_below_set
    __swig_getmethods__["net_wm_state_below"] = _openbox.Atoms_net_wm_state_below_get
    if _newclass:net_wm_state_below = property(_openbox.Atoms_net_wm_state_below_get,_openbox.Atoms_net_wm_state_below_set)
    __swig_setmethods__["kde_net_system_tray_windows"] = _openbox.Atoms_kde_net_system_tray_windows_set
    __swig_getmethods__["kde_net_system_tray_windows"] = _openbox.Atoms_kde_net_system_tray_windows_get
    if _newclass:kde_net_system_tray_windows = property(_openbox.Atoms_kde_net_system_tray_windows_get,_openbox.Atoms_kde_net_system_tray_windows_set)
    __swig_setmethods__["kde_net_wm_system_tray_window_for"] = _openbox.Atoms_kde_net_wm_system_tray_window_for_set
    __swig_getmethods__["kde_net_wm_system_tray_window_for"] = _openbox.Atoms_kde_net_wm_system_tray_window_for_get
    if _newclass:kde_net_wm_system_tray_window_for = property(_openbox.Atoms_kde_net_wm_system_tray_window_for_get,_openbox.Atoms_kde_net_wm_system_tray_window_for_set)
    __swig_setmethods__["kde_net_wm_window_type_override"] = _openbox.Atoms_kde_net_wm_window_type_override_set
    __swig_getmethods__["kde_net_wm_window_type_override"] = _openbox.Atoms_kde_net_wm_window_type_override_get
    if _newclass:kde_net_wm_window_type_override = property(_openbox.Atoms_kde_net_wm_window_type_override_get,_openbox.Atoms_kde_net_wm_window_type_override_set)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C Atoms instance at %s>" % (self.this,)

class AtomsPtr(Atoms):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Atoms
_openbox.Atoms_swigregister(AtomsPtr)

class Property(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Property, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Property, name)
    ascii = _openbox.Property_ascii
    utf8 = _openbox.Property_utf8
    NUM_STRING_TYPE = _openbox.Property_NUM_STRING_TYPE
    __swig_getmethods__["initialize"] = lambda x: _openbox.Property_initialize
    if _newclass:initialize = staticmethod(_openbox.Property_initialize)
    __swig_getmethods__["set"] = lambda x: _openbox.Property_set
    if _newclass:set = staticmethod(_openbox.Property_set)
    __swig_getmethods__["set"] = lambda x: _openbox.Property_set
    if _newclass:set = staticmethod(_openbox.Property_set)
    __swig_getmethods__["set"] = lambda x: _openbox.Property_set
    if _newclass:set = staticmethod(_openbox.Property_set)
    __swig_getmethods__["set"] = lambda x: _openbox.Property_set
    if _newclass:set = staticmethod(_openbox.Property_set)
    __swig_getmethods__["get"] = lambda x: _openbox.Property_get
    if _newclass:get = staticmethod(_openbox.Property_get)
    __swig_getmethods__["get"] = lambda x: _openbox.Property_get
    if _newclass:get = staticmethod(_openbox.Property_get)
    __swig_getmethods__["get"] = lambda x: _openbox.Property_get
    if _newclass:get = staticmethod(_openbox.Property_get)
    __swig_getmethods__["get"] = lambda x: _openbox.Property_get
    if _newclass:get = staticmethod(_openbox.Property_get)
    __swig_getmethods__["erase"] = lambda x: _openbox.Property_erase
    if _newclass:erase = staticmethod(_openbox.Property_erase)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C Property instance at %s>" % (self.this,)

class PropertyPtr(Property):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Property
_openbox.Property_swigregister(PropertyPtr)
Property_initialize = _openbox.Property_initialize

Property_set = _openbox.Property_set

Property_get = _openbox.Property_get

Property_erase = _openbox.Property_erase


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

class EventHandler(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, EventHandler, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, EventHandler, name)
    def handle(*args): return apply(_openbox.EventHandler_handle,args)
    def keyPressHandler(*args): return apply(_openbox.EventHandler_keyPressHandler,args)
    def keyReleaseHandler(*args): return apply(_openbox.EventHandler_keyReleaseHandler,args)
    def buttonPressHandler(*args): return apply(_openbox.EventHandler_buttonPressHandler,args)
    def buttonReleaseHandler(*args): return apply(_openbox.EventHandler_buttonReleaseHandler,args)
    def motionHandler(*args): return apply(_openbox.EventHandler_motionHandler,args)
    def enterHandler(*args): return apply(_openbox.EventHandler_enterHandler,args)
    def leaveHandler(*args): return apply(_openbox.EventHandler_leaveHandler,args)
    def focusHandler(*args): return apply(_openbox.EventHandler_focusHandler,args)
    def unfocusHandler(*args): return apply(_openbox.EventHandler_unfocusHandler,args)
    def exposeHandler(*args): return apply(_openbox.EventHandler_exposeHandler,args)
    def graphicsExposeHandler(*args): return apply(_openbox.EventHandler_graphicsExposeHandler,args)
    def noExposeEventHandler(*args): return apply(_openbox.EventHandler_noExposeEventHandler,args)
    def circulateRequestHandler(*args): return apply(_openbox.EventHandler_circulateRequestHandler,args)
    def configureRequestHandler(*args): return apply(_openbox.EventHandler_configureRequestHandler,args)
    def mapRequestHandler(*args): return apply(_openbox.EventHandler_mapRequestHandler,args)
    def resizeRequestHandler(*args): return apply(_openbox.EventHandler_resizeRequestHandler,args)
    def circulateHandler(*args): return apply(_openbox.EventHandler_circulateHandler,args)
    def configureHandler(*args): return apply(_openbox.EventHandler_configureHandler,args)
    def createHandler(*args): return apply(_openbox.EventHandler_createHandler,args)
    def destroyHandler(*args): return apply(_openbox.EventHandler_destroyHandler,args)
    def gravityHandler(*args): return apply(_openbox.EventHandler_gravityHandler,args)
    def mapHandler(*args): return apply(_openbox.EventHandler_mapHandler,args)
    def mappingHandler(*args): return apply(_openbox.EventHandler_mappingHandler,args)
    def reparentHandler(*args): return apply(_openbox.EventHandler_reparentHandler,args)
    def unmapHandler(*args): return apply(_openbox.EventHandler_unmapHandler,args)
    def visibilityHandler(*args): return apply(_openbox.EventHandler_visibilityHandler,args)
    def colorMapHandler(*args): return apply(_openbox.EventHandler_colorMapHandler,args)
    def propertyHandler(*args): return apply(_openbox.EventHandler_propertyHandler,args)
    def selectionClearHandler(*args): return apply(_openbox.EventHandler_selectionClearHandler,args)
    def selectionHandler(*args): return apply(_openbox.EventHandler_selectionHandler,args)
    def selectionRequestHandler(*args): return apply(_openbox.EventHandler_selectionRequestHandler,args)
    def clientMessageHandler(*args): return apply(_openbox.EventHandler_clientMessageHandler,args)
    def __del__(self, destroy= _openbox.delete_EventHandler):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C EventHandler instance at %s>" % (self.this,)

class EventHandlerPtr(EventHandler):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = EventHandler
_openbox.EventHandler_swigregister(EventHandlerPtr)

class EventDispatcher(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, EventDispatcher, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, EventDispatcher, name)
    def __init__(self,*args):
        self.this = apply(_openbox.new_EventDispatcher,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_EventDispatcher):
        try:
            if self.thisown: destroy(self)
        except: pass
    def clearAllHandlers(*args): return apply(_openbox.EventDispatcher_clearAllHandlers,args)
    def registerHandler(*args): return apply(_openbox.EventDispatcher_registerHandler,args)
    def clearHandler(*args): return apply(_openbox.EventDispatcher_clearHandler,args)
    def dispatchEvents(*args): return apply(_openbox.EventDispatcher_dispatchEvents,args)
    def setFallbackHandler(*args): return apply(_openbox.EventDispatcher_setFallbackHandler,args)
    def getFallbackHandler(*args): return apply(_openbox.EventDispatcher_getFallbackHandler,args)
    def setMasterHandler(*args): return apply(_openbox.EventDispatcher_setMasterHandler,args)
    def getMasterHandler(*args): return apply(_openbox.EventDispatcher_getMasterHandler,args)
    def findHandler(*args): return apply(_openbox.EventDispatcher_findHandler,args)
    def lastTime(*args): return apply(_openbox.EventDispatcher_lastTime,args)
    def __repr__(self):
        return "<C EventDispatcher instance at %s>" % (self.this,)

class EventDispatcherPtr(EventDispatcher):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = EventDispatcher
_openbox.EventDispatcher_swigregister(EventDispatcherPtr)

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

class Openbox(EventDispatcher,EventHandler):
    __swig_setmethods__ = {}
    for _s in [EventDispatcher,EventHandler]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, Openbox, name, value)
    __swig_getmethods__ = {}
    for _s in [EventDispatcher,EventHandler]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, Openbox, name)
    State_Starting = _openbox.Openbox_State_Starting
    State_Normal = _openbox.Openbox_State_Normal
    State_Exiting = _openbox.Openbox_State_Exiting
    def state(*args): return apply(_openbox.Openbox_state,args)
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

class Screen(EventHandler,):
    __swig_setmethods__ = {}
    for _s in [EventHandler,]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, Screen, name, value)
    __swig_getmethods__ = {}
    for _s in [EventHandler,]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, Screen, name)
    def client(*args): return apply(_openbox.Screen_client,args)
    def clientCount(*args): return apply(_openbox.Screen_clientCount,args)
    event_mask = _openbox.Screen_event_mask
    def number(*args): return apply(_openbox.Screen_number,args)
    def managed(*args): return apply(_openbox.Screen_managed,args)
    def area(*args): return apply(_openbox.Screen_area,args)
    def style(*args): return apply(_openbox.Screen_style,args)
    def focuswindow(*args): return apply(_openbox.Screen_focuswindow,args)
    def desktop(*args): return apply(_openbox.Screen_desktop,args)
    def numDesktops(*args): return apply(_openbox.Screen_numDesktops,args)
    def updateStrut(*args): return apply(_openbox.Screen_updateStrut,args)
    def manageExisting(*args): return apply(_openbox.Screen_manageExisting,args)
    def manageWindow(*args): return apply(_openbox.Screen_manageWindow,args)
    def unmanageWindow(*args): return apply(_openbox.Screen_unmanageWindow,args)
    def raiseWindow(*args): return apply(_openbox.Screen_raiseWindow,args)
    def lowerWindow(*args): return apply(_openbox.Screen_lowerWindow,args)
    def setDesktopName(*args): return apply(_openbox.Screen_setDesktopName,args)
    def propertyHandler(*args): return apply(_openbox.Screen_propertyHandler,args)
    def clientMessageHandler(*args): return apply(_openbox.Screen_clientMessageHandler,args)
    def mapRequestHandler(*args): return apply(_openbox.Screen_mapRequestHandler,args)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C Screen instance at %s>" % (self.this,)

class ScreenPtr(Screen):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Screen
_openbox.Screen_swigregister(ScreenPtr)

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

class Client(EventHandler,):
    __swig_setmethods__ = {}
    for _s in [EventHandler,]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, Client, name, value)
    __swig_getmethods__ = {}
    for _s in [EventHandler,]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, Client, name)
    __swig_setmethods__["frame"] = _openbox.Client_frame_set
    __swig_getmethods__["frame"] = _openbox.Client_frame_get
    if _newclass:frame = property(_openbox.Client_frame_get,_openbox.Client_frame_set)
    Layer_Icon = _openbox.Client_Layer_Icon
    Layer_Desktop = _openbox.Client_Layer_Desktop
    Layer_Below = _openbox.Client_Layer_Below
    Layer_Normal = _openbox.Client_Layer_Normal
    Layer_Above = _openbox.Client_Layer_Above
    Layer_Top = _openbox.Client_Layer_Top
    Layer_Fullscreen = _openbox.Client_Layer_Fullscreen
    Layer_Internal = _openbox.Client_Layer_Internal
    NUM_LAYERS = _openbox.Client_NUM_LAYERS
    TopLeft = _openbox.Client_TopLeft
    TopRight = _openbox.Client_TopRight
    BottomLeft = _openbox.Client_BottomLeft
    BottomRight = _openbox.Client_BottomRight
    Type_Desktop = _openbox.Client_Type_Desktop
    Type_Dock = _openbox.Client_Type_Dock
    Type_Toolbar = _openbox.Client_Type_Toolbar
    Type_Menu = _openbox.Client_Type_Menu
    Type_Utility = _openbox.Client_Type_Utility
    Type_Splash = _openbox.Client_Type_Splash
    Type_Dialog = _openbox.Client_Type_Dialog
    Type_Normal = _openbox.Client_Type_Normal
    MwmFlag_Functions = _openbox.Client_MwmFlag_Functions
    MwmFlag_Decorations = _openbox.Client_MwmFlag_Decorations
    MwmFunc_All = _openbox.Client_MwmFunc_All
    MwmFunc_Resize = _openbox.Client_MwmFunc_Resize
    MwmFunc_Move = _openbox.Client_MwmFunc_Move
    MwmFunc_Iconify = _openbox.Client_MwmFunc_Iconify
    MwmFunc_Maximize = _openbox.Client_MwmFunc_Maximize
    MwmDecor_All = _openbox.Client_MwmDecor_All
    MwmDecor_Border = _openbox.Client_MwmDecor_Border
    MwmDecor_Handle = _openbox.Client_MwmDecor_Handle
    MwmDecor_Title = _openbox.Client_MwmDecor_Title
    MwmDecor_Iconify = _openbox.Client_MwmDecor_Iconify
    MwmDecor_Maximize = _openbox.Client_MwmDecor_Maximize
    Func_Resize = _openbox.Client_Func_Resize
    Func_Move = _openbox.Client_Func_Move
    Func_Iconify = _openbox.Client_Func_Iconify
    Func_Maximize = _openbox.Client_Func_Maximize
    Func_Shade = _openbox.Client_Func_Shade
    Func_Fullscreen = _openbox.Client_Func_Fullscreen
    Func_Close = _openbox.Client_Func_Close
    Decor_Titlebar = _openbox.Client_Decor_Titlebar
    Decor_Handle = _openbox.Client_Decor_Handle
    Decor_Border = _openbox.Client_Decor_Border
    Decor_Iconify = _openbox.Client_Decor_Iconify
    Decor_Maximize = _openbox.Client_Decor_Maximize
    Decor_AllDesktops = _openbox.Client_Decor_AllDesktops
    Decor_Close = _openbox.Client_Decor_Close
    State_Remove = _openbox.Client_State_Remove
    State_Add = _openbox.Client_State_Add
    State_Toggle = _openbox.Client_State_Toggle
    event_mask = _openbox.Client_event_mask
    no_propagate_mask = _openbox.Client_no_propagate_mask
    ICONIC_DESKTOP = _openbox.Client_ICONIC_DESKTOP
    __swig_setmethods__["ignore_unmaps"] = _openbox.Client_ignore_unmaps_set
    __swig_getmethods__["ignore_unmaps"] = _openbox.Client_ignore_unmaps_get
    if _newclass:ignore_unmaps = property(_openbox.Client_ignore_unmaps_get,_openbox.Client_ignore_unmaps_set)
    def screen(*args): return apply(_openbox.Client_screen,args)
    def window(*args): return apply(_openbox.Client_window,args)
    def type(*args): return apply(_openbox.Client_type,args)
    def normal(*args): return apply(_openbox.Client_normal,args)
    def desktop(*args): return apply(_openbox.Client_desktop,args)
    def title(*args): return apply(_openbox.Client_title,args)
    def iconTitle(*args): return apply(_openbox.Client_iconTitle,args)
    def appName(*args): return apply(_openbox.Client_appName,args)
    def appClass(*args): return apply(_openbox.Client_appClass,args)
    def role(*args): return apply(_openbox.Client_role,args)
    def canFocus(*args): return apply(_openbox.Client_canFocus,args)
    def urgent(*args): return apply(_openbox.Client_urgent,args)
    def focusNotify(*args): return apply(_openbox.Client_focusNotify,args)
    def shaped(*args): return apply(_openbox.Client_shaped,args)
    def gravity(*args): return apply(_openbox.Client_gravity,args)
    def positionRequested(*args): return apply(_openbox.Client_positionRequested,args)
    def decorations(*args): return apply(_openbox.Client_decorations,args)
    def funtions(*args): return apply(_openbox.Client_funtions,args)
    def transientFor(*args): return apply(_openbox.Client_transientFor,args)
    def modal(*args): return apply(_openbox.Client_modal,args)
    def shaded(*args): return apply(_openbox.Client_shaded,args)
    def fullscreen(*args): return apply(_openbox.Client_fullscreen,args)
    def iconic(*args): return apply(_openbox.Client_iconic,args)
    def maxVert(*args): return apply(_openbox.Client_maxVert,args)
    def maxHorz(*args): return apply(_openbox.Client_maxHorz,args)
    def layer(*args): return apply(_openbox.Client_layer,args)
    def applyStartupState(*args): return apply(_openbox.Client_applyStartupState,args)
    def toggleClientBorder(*args): return apply(_openbox.Client_toggleClientBorder,args)
    def area(*args): return apply(_openbox.Client_area,args)
    def strut(*args): return apply(_openbox.Client_strut,args)
    def move(*args): return apply(_openbox.Client_move,args)
    def resize(*args): return apply(_openbox.Client_resize,args)
    def focus(*args): return apply(_openbox.Client_focus,args)
    def unfocus(*args): return apply(_openbox.Client_unfocus,args)
    def focusHandler(*args): return apply(_openbox.Client_focusHandler,args)
    def unfocusHandler(*args): return apply(_openbox.Client_unfocusHandler,args)
    def propertyHandler(*args): return apply(_openbox.Client_propertyHandler,args)
    def clientMessageHandler(*args): return apply(_openbox.Client_clientMessageHandler,args)
    def configureRequestHandler(*args): return apply(_openbox.Client_configureRequestHandler,args)
    def unmapHandler(*args): return apply(_openbox.Client_unmapHandler,args)
    def destroyHandler(*args): return apply(_openbox.Client_destroyHandler,args)
    def reparentHandler(*args): return apply(_openbox.Client_reparentHandler,args)
    def mapRequestHandler(*args): return apply(_openbox.Client_mapRequestHandler,args)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C Client instance at %s>" % (self.this,)

class ClientPtr(Client):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Client
_openbox.Client_swigregister(ClientPtr)

class Frame(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Frame, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Frame, name)
    event_mask = _openbox.Frame_event_mask
    def __init__(self,*args):
        self.this = apply(_openbox.new_Frame,args)
        self.thisown = 1
    def __del__(self, destroy= _openbox.delete_Frame):
        try:
            if self.thisown: destroy(self)
        except: pass
    def size(*args): return apply(_openbox.Frame_size,args)
    def setStyle(*args): return apply(_openbox.Frame_setStyle,args)
    def adjust(*args): return apply(_openbox.Frame_adjust,args)
    def focus(*args): return apply(_openbox.Frame_focus,args)
    def unfocus(*args): return apply(_openbox.Frame_unfocus,args)
    def setTitle(*args): return apply(_openbox.Frame_setTitle,args)
    def grabClient(*args): return apply(_openbox.Frame_grabClient,args)
    def releaseClient(*args): return apply(_openbox.Frame_releaseClient,args)
    def adjustSize(*args): return apply(_openbox.Frame_adjustSize,args)
    def adjustPosition(*args): return apply(_openbox.Frame_adjustPosition,args)
    def adjustShape(*args): return apply(_openbox.Frame_adjustShape,args)
    def adjustState(*args): return apply(_openbox.Frame_adjustState,args)
    def clientGravity(*args): return apply(_openbox.Frame_clientGravity,args)
    def frameGravity(*args): return apply(_openbox.Frame_frameGravity,args)
    def plate(*args): return apply(_openbox.Frame_plate,args)
    def titlebar(*args): return apply(_openbox.Frame_titlebar,args)
    def label(*args): return apply(_openbox.Frame_label,args)
    def button_close(*args): return apply(_openbox.Frame_button_close,args)
    def button_iconify(*args): return apply(_openbox.Frame_button_iconify,args)
    def button_max(*args): return apply(_openbox.Frame_button_max,args)
    def button_alldesk(*args): return apply(_openbox.Frame_button_alldesk,args)
    def handle(*args): return apply(_openbox.Frame_handle,args)
    def grip_left(*args): return apply(_openbox.Frame_grip_left,args)
    def grip_right(*args): return apply(_openbox.Frame_grip_right,args)
    def __repr__(self):
        return "<C Frame instance at %s>" % (self.this,)

class FramePtr(Frame):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Frame
_openbox.Frame_swigregister(FramePtr)

MC_Frame = _openbox.MC_Frame
MC_Titlebar = _openbox.MC_Titlebar
MC_Handle = _openbox.MC_Handle
MC_Window = _openbox.MC_Window
MC_MaximizeButton = _openbox.MC_MaximizeButton
MC_CloseButton = _openbox.MC_CloseButton
MC_IconifyButton = _openbox.MC_IconifyButton
MC_AllDesktopsButton = _openbox.MC_AllDesktopsButton
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
EventUrgentWindow = _openbox.EventUrgentWindow
EventStartup = _openbox.EventStartup
EventShutdown = _openbox.EventShutdown
EventKey = _openbox.EventKey
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
    __swig_setmethods__["action"] = _openbox.KeyData_action_set
    __swig_getmethods__["action"] = _openbox.KeyData_action_get
    if _newclass:action = property(_openbox.KeyData_action_get,_openbox.KeyData_action_set)
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

