#include "engine.h"
#include "parse.h"

#include <glib.h>
#include <gmodule.h>
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

char *engine_name;
char *engine_theme;
char *engine_layout;
char *engine_font;
gboolean engine_shadow;
int engine_shadow_offset;
int engine_shadow_tint;

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

    path = g_build_filename(ENGINEDIR, name, NULL);
    module = g_module_open(path, 0);
    g_free(path);

    if (module == NULL) {
	path = g_build_filename(g_get_home_dir(), ".openbox", "engines", name,
				NULL);
	module = g_module_open(path, 0);
	g_free(path);
    }

    if (module == NULL)
	return FALSE;

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

    if (!estartup())
	return FALSE;

    return TRUE;
}

static void parse_assign(char *name, ParseToken *value)
{
    if (!g_ascii_strcasecmp(name, "engine")) {
        if (value->type != TOKEN_STRING)
            yyerror("invalid value");
        else {
            g_free(engine_name);
            engine_name = g_strdup(value->data.string);
        }
    } else if (!g_ascii_strcasecmp(name, "theme")) {
        if (value->type != TOKEN_STRING)
            yyerror("invalid value");
        else {
            g_free(engine_theme);
            engine_theme = g_strdup(value->data.string);
        }
    } else if (!g_ascii_strcasecmp(name, "titlebarlayout")) {
        if (value->type != TOKEN_STRING)
            yyerror("invalid value");
        else {
            g_free(engine_layout);
            engine_layout = g_strdup(value->data.string);
        }
    } else if (!g_ascii_strcasecmp(name, "font.title")) {
        if (value->type != TOKEN_STRING)
            yyerror("invalid value");
        else {
            g_free(engine_font);
            engine_font = g_strdup(value->data.string);
        }
    } else if (!g_ascii_strcasecmp(name, "font.title.shadow")) {
        if (value->type != TOKEN_BOOL)
            yyerror("invalid value");
        else {
            engine_shadow = value->data.bool;
        }
    } else if (!g_ascii_strcasecmp(name, "font.title.shadow.offset")) {
        if (value->type != TOKEN_INTEGER)
            yyerror("invalid value");
        else {
            engine_shadow_offset = value->data.integer;
        }
    } else if (!g_ascii_strcasecmp(name, "font.title.shadow.tint")) {
        if (value->type != TOKEN_INTEGER)
            yyerror("invalid value");
        else {
            engine_shadow_tint = value->data.integer;
            if (engine_shadow_tint < -100) engine_shadow_tint = -100;
            else if (engine_shadow_tint > 100) engine_shadow_tint = 100;
        }
    } else
        yyerror("invalid option");
    parse_free_token(value);
}

void engine_startup()
{
    module = NULL;
    engine_name = g_strdup(DEFAULT_ENGINE);
    engine_theme = NULL;
    engine_layout = g_strdup("NDSLIMC");
    engine_font = g_strdup("Sans-7");
    engine_shadow = FALSE;
    engine_shadow_offset = 1;
    engine_shadow_tint = 25;

    parse_reg_section("engine", NULL, parse_assign);
}

void engine_load()
{
    if (load(engine_name))
        return;
    g_warning("Failed to load the engine '%s'", engine_name);
    g_message("Falling back to the default: '%s'", DEFAULT_ENGINE);
    if (!load(DEFAULT_ENGINE)) {
	g_critical("Failed to load the engine '%s'. Aborting", DEFAULT_ENGINE);
	exit(1);
    }
}

void engine_shutdown()
{
    g_free(engine_name);
    if (module != NULL) {
	eshutdown();
	g_module_close(module);
    }
}
