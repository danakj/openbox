#ifndef __engine_h
#define __engine_h

#include "../engines/engineinterface.h"

void engine_startup();
void engine_shutdown();

EngineFrameNew *engine_frame_new;

EngineFrameGrabClient *engine_frame_grab_client;
EngineFrameReleaseClient *engine_frame_release_client;

EngineFrameAdjustArea *engine_frame_adjust_area;
EngineFrameAdjustShape *engine_frame_adjust_shape;
EngineFrameAdjustState *engine_frame_adjust_state;
EngineFrameAdjustFocus *engine_frame_adjust_focus;
EngineFrameAdjustTitle *engine_frame_adjust_title;
EngineFrameAdjustIcon *engine_frame_adjust_icon;

EngineFrameShow *engine_frame_show;
EngineFrameHide *engine_frame_hide;

EngineGetContext *engine_get_context;

#endif
