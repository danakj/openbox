#include "obconf.h"
#include "plugins/obconf_interface.h"

#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <gmodule.h>

typedef struct ConfigPlugin {
    GModule *module;
    char *fname;
    char *name;
    char *plugin_name;

    PluginStartupFunc start;
    PluginShutdownFunc stop;
    PluginInterfaceVersionFunc interface;
    PluginNameFunc getname;
    PluginPluginNameFunc getpname;
    PluginIconFunc icon;
    PluginToplevelWidgetFunc toplevel;
    PluginEditedFunc edited;
    PluginLoadFunc load;
    PluginSaveFunc save;
} ConfigPlugin;

GSList *plugins_list = NULL;

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

static void add_plugin(ConfigPlugin *p)
{
    GtkTreeIter it;

    gtk_list_store_append(obconf_sections_store, &it);
    gtk_list_store_set(obconf_sections_store, &it, 0, p->name, -1);
    gtk_notebook_append_page(obconf_options, p->toplevel(), NULL);
}

void load_dir(char *path)
{
    char *fpath;
    DIR *dir;
    struct dirent *dirp;
    ConfigPlugin *p;
    GModule *mod;
    GSList *it;
    char *suffix;

    suffix = g_strconcat("-config.", G_MODULE_SUFFIX, NULL);

    if (!(dir = opendir(path)))
        return;
    while ((dirp = readdir(dir))) {
        if (g_strrstr(dirp->d_name, suffix)) {
            /* look for duplicates */
            for (it = plugins_list; it; it = it->next)
                if (!strcmp(((ConfigPlugin*)it->data)->fname, dirp->d_name))
                    break;
            if (!it) {
                fpath = g_build_filename(path, dirp->d_name, NULL);
        
                if ((mod = g_module_open(fpath, 0))) {
                    p = g_new(ConfigPlugin, 1);
                    p->module = mod;
                    p->fname = g_strdup(dirp->d_name);

                    p->interface = (PluginInterfaceVersionFunc)
                        load_sym(p->module, p->fname,
                                 "plugin_interface_version",
                                 FALSE);
                    p->start = (PluginStartupFunc)
                        load_sym(p->module, p->fname, "plugin_startup", FALSE);
                    p->stop = (PluginShutdownFunc)
                        load_sym(p->module, p->fname, "plugin_shutdown",FALSE);
                    p->getname = (PluginNameFunc)
                        load_sym(p->module, p->fname, "plugin_name", FALSE);
                    p->getpname = (PluginNameFunc)
                        load_sym(p->module, p->fname, "plugin_plugin_name",
                                 FALSE);
                    p->icon = (PluginIconFunc)
                        load_sym(p->module, p->fname, "plugin_icon", FALSE);
                    p->toplevel = (PluginToplevelWidgetFunc)
                        load_sym(p->module, p->fname, "plugin_toplevel_widget",
                                 FALSE);
                    p->edited = (PluginEditedFunc)
                        load_sym(p->module, p->fname, "plugin_edited", FALSE);
                    p->load = (PluginLoadFunc)
                        load_sym(p->module, p->fname, "plugin_load", FALSE);
                    p->save = (PluginSaveFunc)
                        load_sym(p->module, p->fname, "plugin_save", FALSE);

                    if (!(p->start &&
                          p->stop &&
                          p->interface &&
                          p->name &&
                          p->icon &&
                          p->toplevel &&
                          p->edited &&
                          p->load &&
                          p->save)) {
                        g_module_close(p->module);
                        g_free(p->fname);
                        g_free(p);
                    } else {
                        p->start();
                        p->name = p->getname();
                        p->plugin_name = p->getpname();
                        plugins_list = g_slist_append(plugins_list, p);

                        add_plugin(p); /* add to the gui */
                    }
                }
                g_free(fpath);
            }
        }              
    }

    g_free(suffix);
}

void plugins_load()
{
    char *path;

    path = g_build_filename(g_get_home_dir(), ".openbox", "plugins", NULL);
    load_dir(path);
    g_free(path);

    load_dir(PLUGINDIR);
}

gboolean plugins_edited(ConfigPlugin *p)
{
    return p->edited();
}

void plugins_load_settings(ConfigPlugin *p, xmlDocPtr doc, xmlNodePtr root)
{
    p->load(doc, root);
}

void plugins_save_settings(ConfigPlugin *p, xmlDocPtr doc, xmlNodePtr root)
{
    p->save(doc, root);
}
