#include "engine.h"
#include "config.h"

#include <glib.h>
#include <gmodule.h>
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

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
EngineRenderLabel *engine_render_label;
EngineSizeLabel *engine_size_label;

static GModule *module = NULL;
static EngineStartup *estartup = NULL;
static EngineShutdown *eshutdown = NULL;

#define LOADSYM(name, var) \
    if (!g_module_symbol(module, #name, (gpointer*)&var)) { \
        g_warning("Failed to load symbol %s from engine", #name); \
        return FALSE; \
    }

static gboolean load(char *name)
{
    char *path;

    g_assert(module == NULL);

    path = g_build_filename(g_get_home_dir(), ".openbox", "engines", name,
                            NULL);
    module = g_module_open(path, 0);
    g_free(path);

    if (module == NULL) {
        path = g_build_filename(ENGINEDIR, name, NULL);
        module = g_module_open(path, 0);
        g_free(path);
    }

    if (module == NULL) {
        g_warning(g_module_error());
	return FALSE;
    }

    /* load the engine's symbols */
    LOADSYM(startup, estartup);
    LOADSYM(shutdown, eshutdown);
    LOADSYM(frame_new, engine_frame_new);
    LOADSYM(frame_grab_client, engine_frame_grab_client);
    LOADSYM(frame_release_client, engine_frame_release_client);
    LOADSYM(frame_adjust_area, engine_frame_adjust_area);
    LOADSYM(frame_adjust_shape, engine_frame_adjust_shape);
    LOADSYM(frame_adjust_state, engine_frame_adjust_state);
    LOADSYM(frame_adjust_focus, engine_frame_adjust_focus);
    LOADSYM(frame_adjust_title, engine_frame_adjust_title);
    LOADSYM(frame_adjust_icon, engine_frame_adjust_icon);
    LOADSYM(frame_show, engine_frame_show);
    LOADSYM(frame_hide, engine_frame_hide);
    LOADSYM(get_context, engine_get_context);
    LOADSYM(render_label, engine_render_label);
    LOADSYM(size_label, engine_size_label);

    if (!estartup())
	return FALSE;

    return TRUE;
}

void engine_startup()
{
    module = NULL;
}

void engine_load()
{
    if (load(config_engine_name))
        return;
    g_warning("Failed to load the engine '%s'", config_engine_name);
    g_message("Falling back to the default: '%s'", DEFAULT_ENGINE);
    if (module != NULL) {
	g_module_close(module);
        module = NULL;
    }
    if (!load(DEFAULT_ENGINE)) {
	g_critical("Failed to load the engine '%s'. Aborting", DEFAULT_ENGINE);
	exit(1);
    }
}

void engine_shutdown()
{
    if (module != NULL) {
	eshutdown();
	g_module_close(module);
    }
}
