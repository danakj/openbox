#include "plugins/interface.h"
#include "parser/parse.h"

#include <glib.h>
#include <gmodule.h>

typedef struct {
    GModule *module;
    char *name;

    PluginSetupConfig config;
    PluginStartup startup;
    PluginShutdown shutdown;
    PluginCreate create;
    PluginDestroy destroy;
} Plugin;

static gpointer load_sym(GModule *module, char *name, char *symbol,
			 gboolean allow_fail)
{
    gpointer var;
    if (!g_module_symbol(module, symbol, &var)) {
        if (!allow_fail)
	    g_warning("Failed to load symbol '%s' from plugin '%s'",
		      symbol, name);
        var = NULL;
    }
    return var;
}

static Plugin *plugin_new(char *name)
{
    Plugin *p;
    char *path;
   
    p = g_new(Plugin, 1);

    path = g_build_filename(g_get_home_dir(), ".openbox", "plugins", name,
                            NULL);
    p->module = g_module_open(path, 0);
    g_free(path);

    if (p->module == NULL) {
        path = g_build_filename(PLUGINDIR, name, NULL);
        p->module = g_module_open(path, 0);
        g_free(path);
    }

    if (p->module == NULL) {
        g_warning(g_module_error());
        g_free(p);
        return NULL;
    }

    p->config = (PluginSetupConfig)load_sym(p->module, name,
                                            "plugin_setup_config", FALSE);
    p->startup = (PluginStartup)load_sym(p->module, name, "plugin_startup",
					 FALSE);
    p->shutdown = (PluginShutdown)load_sym(p->module, name, "plugin_shutdown",
					   FALSE);
    p->create = (PluginCreate)load_sym(p->module, name, "plugin_create", TRUE);
    p->destroy = (PluginDestroy)load_sym(p->module, name, "plugin_destroy",
					 TRUE);

    if (p->config == NULL || p->startup == NULL || p->shutdown == NULL) {
        g_module_close(p->module);
        g_free(p);
        return NULL;
    }

    p->name = g_strdup(name);
    return p;
}

static void plugin_free(Plugin *p)
{
    p->shutdown();

    g_free(p->name);
    g_module_close(p->module);
}


static GData *plugins = NULL;

void plugin_startup()
{
    g_datalist_init(&plugins);
}

void plugin_shutdown()
{
    g_datalist_clear(&plugins);
}

gboolean plugin_open_full(char *name, gboolean reopen, ObParseInst *i)
{
    Plugin *p;

    if (g_datalist_get_data(&plugins, name) != NULL) {
	if (!reopen) 
	    g_warning("plugin '%s' already loaded, can't load again", name);
        return TRUE;
    }

    p = plugin_new(name);
    if (p == NULL) {
        g_warning("failed to load plugin '%s'", name);
        return FALSE;
    }
    p->config(i);

    g_datalist_set_data_full(&plugins, name, p, (GDestroyNotify) plugin_free);
    return TRUE;
}

gboolean plugin_open(char *name, ObParseInst *i) {
    return plugin_open_full(name, FALSE, i);
}

gboolean plugin_open_reopen(char *name, ObParseInst *i) {
    return plugin_open_full(name, TRUE, i);
}

void plugin_close(char *name)
{
    g_datalist_remove_data(&plugins, name);
}

static void foreach_start(GQuark key, Plugin *p, gpointer *foo)
{
    p->startup();
}

void plugin_startall()
{
    g_datalist_foreach(&plugins, (GDataForeachFunc)foreach_start, NULL);
}

void plugin_loadall(ObParseInst *i)
{
    GIOChannel *io;
    GError *err;
    char *path, *name;

    path = g_build_filename(g_get_home_dir(), ".openbox", "pluginrc", NULL);
    err = NULL;
    io = g_io_channel_new_file(path, "r", &err);
    g_free(path);

    if (io == NULL) {
        path = g_build_filename(RCDIR, "pluginrc", NULL);
        err = NULL;
        io = g_io_channel_new_file(path, "r", &err);
        g_free(path);
    }

    if (io == NULL) {
        /* load the default plugins */
        plugin_open("placement", i);
        plugin_open("resistance", i);

        /* XXX rm me when the parser loads me magically */
        plugin_open("client_menu", i);
    } else {
        /* load the plugins in the rc file */
        while (g_io_channel_read_line(io, &name, NULL, NULL, &err) ==
               G_IO_STATUS_NORMAL) {
            g_strstrip(name);
            if (name[0] != '\0' && name[0] != '#')
                plugin_open(name, i);
            g_free(name);
        }
        g_io_channel_unref(io);
    }
}

void *plugin_create(char *name, void *data)
{
    Plugin *p = (Plugin *)g_datalist_get_data(&plugins, name);

    if (p == NULL) {
	g_warning("Unable to find plugin for create: %s", name);
	return NULL;
    }

    if (p->create == NULL || p->destroy == NULL) {
	g_critical("Unsupported create/destroy: %s", name);
	return NULL;
    }

    return p->create(data);
}

void plugin_destroy(char *name, void *data)
{
    Plugin *p = (Plugin *)g_datalist_get_data(&plugins, name);

    if (p == NULL) {
	g_critical("Unable to find plugin for destroy: %s", name);
	/* really shouldn't happen, but attempt to free something anyway? */
	g_free(data);
	return;
    }

    if (p->destroy == NULL || p->create == NULL) {
	g_critical("Unsupported create/destroy: %s", name);
	/* really, really shouldn't happen, but attempt to free anyway? */
	g_free(data);
	return;
    }

    p->destroy(data);
}
