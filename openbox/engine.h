#ifndef __engine_h
#define __engine_h

#include "../engines/engineinterface.h"

void engine_startup(char *engine);
void engine_shutdown();

EngineFrameNew *engine_frame_new;

EngineFrameGrabClient *engine_frame_grab_client;
EngineFrameReleaseClient *engine_frame_release_client;

EngineFrameAdjustSize *engine_frame_adjust_size;
EngineFrameAdjustPosition *engine_frame_adjust_position;
EngineFrameAdjustShape *engine_frame_adjust_shape;
EngineFrameAdjustState *engine_frame_adjust_state;
EngineFrameAdjustFocus *engine_frame_adjust_focus;
EngineFrameAdjustTitle *engine_frame_adjust_title;
EngineFrameAdjustIcon *engine_frame_adjust_icon;

EngineFrameShow *engine_frame_show;
EngineFrameHide *engine_frame_hide;

EngineGetContext *engine_get_context;

EngineMouseEnter *engine_mouse_enter;
EngineMouseLeave *engine_mouse_leave;
EngineMousePress *engine_mouse_press;
EngineMouseRelease *engine_mouse_release;

#endif
