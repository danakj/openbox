#include "config.h"
#include "parse.h"

gboolean config_focus_new;
gboolean config_focus_follow;
gboolean config_focus_last;
gboolean config_focus_last_on_desktop;
gboolean config_focus_popup;

char *config_theme;

int config_desktops_num;
GSList *config_desktops_names;

gboolean config_opaque_move;
gboolean config_opaque_resize;

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
    } else if (!g_ascii_strcasecmp(name, "cyclingdialog")) {
        if (value->type != TOKEN_BOOL)
            yyerror("invalid value");
        else {
            config_focus_popup = value->data.bool;
        }
    } else
        yyerror("invalid option");
    parse_free_token(value);
}

static void parse_theme(char *name, ParseToken *value)
{
    if (!g_ascii_strcasecmp(name, "theme")) {
        if (value->type != TOKEN_STRING)
            yyerror("invalid value");
        else {
            g_free(config_theme);
            config_theme = g_strdup(value->data.string);
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

static void parse_moveresize(char *name, ParseToken *value)
{
    if (!g_ascii_strcasecmp(name, "opaque_move")) {
        if (value->type != TOKEN_BOOL)
            yyerror("invalid value");
        else {
            config_opaque_move = value->data.integer;
        }
    } else if (!g_ascii_strcasecmp(name, "opaque_resize")) {
        if (value->type != TOKEN_BOOL)
            yyerror("invalid value");
        else {
            config_opaque_resize = value->data.integer;
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
    config_focus_popup = TRUE;

    parse_reg_section("focus", NULL, parse_focus);

    config_theme = NULL;

    parse_reg_section("theme", NULL, parse_theme);

    config_desktops_num = 4;
    config_desktops_names = NULL;

    parse_reg_section("desktops", NULL, parse_desktops);

    config_opaque_move = TRUE;
    config_opaque_resize = TRUE;

    parse_reg_section("moveresize", NULL, parse_moveresize);
}

void config_shutdown()
{
    GSList *it;

    g_free(config_theme);

    for (it = config_desktops_names; it; it = it->next)
        g_free(it->data);
    g_slist_free(config_desktops_names);
}
