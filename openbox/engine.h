#ifndef __engine_h
#define __engine_h

#include "../engines/engineinterface.h"

/* The engine to load */
extern char *engine_name;
/* The theme to load */
extern char *engine_theme;
/* The titlebar layout */
extern char *engine_layout;
/* The titlebar font */
extern char *engine_font;
/* The titlebar font's shadow */
extern gboolean engine_shadow;
/* The titlebar font's shadow offset */
extern int engine_shadow_offset;
/* The titlebar font's shadow transparency */
extern int engine_shadow_tint;

void engine_startup();
void engine_load();
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
