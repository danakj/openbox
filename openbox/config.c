#include "config.h"
#include "parse.h"

gboolean config_focus_new;
gboolean config_focus_follow;
gboolean config_focus_last;
gboolean config_focus_last_on_desktop;

char *config_engine_name;
char *config_engine_theme;
char *config_engine_layout;
char *config_engine_font;
gboolean config_engine_shadow;
int config_engine_shadow_offset;
int config_engine_shadow_tint;

int config_desktops_num;
GSList *config_desktops_names;

static void parse_focus(char *name, ParseToken *value)
{
    if (!g_ascii_strcasecmp(name, "focusnew")) {
        if (value->type != TOKEN_BOOL)
            yyerror("invalid value");
        else {
            config_focus_new = value->data.bool;
        }
    } else if (!g_ascii_strcasecmp(name, "followmouse")) {
        if (value->type != TOKEN_BOOL)
            yyerror("invalid value");
        else {
            config_focus_follow = value->data.bool;
        }
    } else if (!g_ascii_strcasecmp(name, "focuslast")) {
        if (value->type != TOKEN_BOOL)
            yyerror("invalid value");
        else {
            config_focus_last = value->data.bool;
        }
    } else if (!g_ascii_strcasecmp(name, "focuslastondesktop")) {
        if (value->type != TOKEN_BOOL)
            yyerror("invalid value");
        else {
            config_focus_last_on_desktop = value->data.bool;
        }
    } else
        yyerror("invalid option");
    parse_free_token(value);
}

static void parse_engine(char *name, ParseToken *value)
{
    if (!g_ascii_strcasecmp(name, "engine")) {
        if (value->type != TOKEN_STRING)
            yyerror("invalid value");
        else {
            g_free(config_engine_name);
            config_engine_name = g_strdup(value->data.string);
        }
    } else if (!g_ascii_strcasecmp(name, "theme")) {
        if (value->type != TOKEN_STRING)
            yyerror("invalid value");
        else {
            g_free(config_engine_theme);
            config_engine_theme = g_strdup(value->data.string);
        }
    } else if (!g_ascii_strcasecmp(name, "titlebarlayout")) {
        if (value->type != TOKEN_STRING)
            yyerror("invalid value");
        else {
            g_free(config_engine_layout);
            config_engine_layout = g_strdup(value->data.string);
        }
    } else if (!g_ascii_strcasecmp(name, "font.title")) {
        if (value->type != TOKEN_STRING)
            yyerror("invalid value");
        else {
            g_free(config_engine_font);
            config_engine_font = g_strdup(value->data.string);
        }
    } else if (!g_ascii_strcasecmp(name, "font.title.shadow")) {
        if (value->type != TOKEN_BOOL)
            yyerror("invalid value");
        else {
            config_engine_shadow = value->data.bool;
        }
    } else if (!g_ascii_strcasecmp(name, "font.title.shadow.offset")) {
        if (value->type != TOKEN_INTEGER)
            yyerror("invalid value");
        else {
            config_engine_shadow_offset = value->data.integer;
        }
    } else if (!g_ascii_strcasecmp(name, "font.title.shadow.tint")) {
        if (value->type != TOKEN_INTEGER)
            yyerror("invalid value");
        else {
            config_engine_shadow_tint = value->data.integer;
            if (config_engine_shadow_tint < -100)
                config_engine_shadow_tint = -100;
            else if (config_engine_shadow_tint > 100)
                config_engine_shadow_tint = 100;
        }
    } else
        yyerror("invalid option");
    parse_free_token(value);
}

static void parse_desktops(char *name, ParseToken *value)
{
    GList *it;

    if (!g_ascii_strcasecmp(name, "number")) {
        if (value->type != TOKEN_INTEGER)
            yyerror("invalid value");
        else {
            config_desktops_num = value->data.integer;
        }
    } else if (!g_ascii_strcasecmp(name, "names")) {
        if (value->type == TOKEN_LIST) {
            for (it = value->data.list; it; it = it->next)
                if (((ParseToken*)it->data)->type != TOKEN_STRING) break;
            if (it == NULL) {
                /* build a string list */
                g_free(config_desktops_names);
                for (it = value->data.list; it; it = it->next)
                    config_desktops_names =
                        g_slist_append(config_desktops_names,
                                       g_strdup
                                       (((ParseToken*)it->data)->data.string));
            } else {
                yyerror("invalid string in names list");
            }
        } else {
            yyerror("syntax error (expected list of strings)");
        }
    } else
        yyerror("invalid option");
    parse_free_token(value);
}

void config_startup()
{
    config_focus_new = TRUE;
    config_focus_follow = FALSE;
    config_focus_last = TRUE;
    config_focus_last_on_desktop = TRUE;

    parse_reg_section("focus", NULL, parse_focus);

    config_engine_name = g_strdup(DEFAULT_ENGINE);
    config_engine_theme = NULL;
    config_engine_layout = g_strdup("NLIMC");
    config_engine_font = g_strdup("Sans-7");
    config_engine_shadow = FALSE;
    config_engine_shadow_offset = 1;
    config_engine_shadow_tint = 25;

    parse_reg_section("engine", NULL, parse_engine);

    config_desktops_num = 4;
    config_desktops_names = NULL;

    parse_reg_section("desktops", NULL, parse_desktops);
}

void config_shutdown()
{
    GSList *it;

    g_free(config_engine_name);
    g_free(config_engine_layout);
    g_free(config_engine_font);

    for (it = config_desktops_names; it; it = it->next)
        g_free(it->data);
    g_slist_free(config_desktops_names);
}
