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

extern EngineFrameNew *engine_frame_new;

extern EngineFrameGrabClient *engine_frame_grab_client;
extern EngineFrameReleaseClient *engine_frame_release_client;

extern EngineFrameAdjustArea *engine_frame_adjust_area;
extern EngineFrameAdjustShape *engine_frame_adjust_shape;
extern EngineFrameAdjustState *engine_frame_adjust_state;
extern EngineFrameAdjustFocus *engine_frame_adjust_focus;
extern EngineFrameAdjustTitle *engine_frame_adjust_title;
extern EngineFrameAdjustIcon *engine_frame_adjust_icon;

extern EngineFrameShow *engine_frame_show;
extern EngineFrameHide *engine_frame_hide;

extern EngineGetContext *engine_get_context;

extern EngineRenderLabel *engine_render_label;
extern EngineSizeLabel *engine_size_label;

#endif
