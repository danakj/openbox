/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*- */

#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/config_value.h"
#include "openbox/client_set.h"
#include "openbox/debug.h"
#include <glib.h>

typedef struct {
    gchar   *str;
} PrintOptions;

typedef struct {
    gboolean toggle;
    gboolean set;
} DebugOptions;

static gpointer setup_print_func(GHashTable *config);
static void     free_print_func(gpointer options);
static gboolean run_print_func(const ObClientSet *set,
                               const ObActionListRun *data, gpointer options);
static gpointer setup_debug_func(GHashTable *config);
static void     free_debug_func(gpointer options);
static gboolean run_debug_func(const ObClientSet *set,
                               const ObActionListRun *data, gpointer options);

void action_debug_startup(void)
{
    action_register("Print", OB_ACTION_DEFAULT_FILTER_EMPTY,
                    setup_print_func, free_print_func, run_print_func);
    action_register("Debug", OB_ACTION_DEFAULT_FILTER_EMPTY,
                    setup_debug_func, free_debug_func, run_debug_func);
}

static gpointer setup_print_func(GHashTable *config)
{
    ObConfigValue *v;
    PrintOptions *o;

    o = g_slice_new0(PrintOptions);

    v = g_hash_table_lookup(config, "string");
    if (v && config_value_is_string(v))
        o->str = g_strdup(config_value_string(v));
    return o;
}

static void free_print_func(gpointer options)
{
    PrintOptions *o = options;
    g_free(o->str);
    g_slice_free(PrintOptions, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_print_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options)
{
    PrintOptions *o = options;
    if (o->str) g_print("%s\n", o->str);
    return FALSE;
}

static gpointer setup_debug_func(GHashTable *config)
{
    ObConfigValue *v;
    DebugOptions *o;

    o = g_slice_new0(DebugOptions);
    o->toggle = TRUE;
    o->set = TRUE;

    v = g_hash_table_lookup(config, "set");
    if (v && config_value_is_string(v)) {
        const gchar *s = config_value_string(v);
        if (!g_ascii_strcasecmp(s, "on"))
            o->toggle = FALSE;
        else if (!g_ascii_strcasecmp(s, "off")) {
            o->toggle = FALSE;
            o->set = FALSE;
        }
    }

    return o;
}

static void free_debug_func(gpointer o)
{
    g_slice_free(DebugOptions, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_debug_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options)
{
  gboolean toggle = !ob_debug_get_enabled(OB_DEBUG_NORMAL);
  if (!toggle) ob_debug("Debugging output disabled");
  ob_debug_enable(OB_DEBUG_NORMAL, toggle);
  if (toggle) ob_debug("Debugging output enabled");
  return FALSE;
}
