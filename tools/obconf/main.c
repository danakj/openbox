#include "obconf.h"
#include "plugins.h"
#include "parser/parse.h"
#include "gettext.h"

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#define OB_ICON "openbox-icon"

static GtkWidget *mainwin;
static GdkPixbuf *ob_icon;

static void obconf_error(GError *e)
{
    GtkWidget *d;

    d = gtk_message_dialog_new(mainwin ? GTK_WINDOW(mainwin) : NULL,
                               GTK_DIALOG_DESTROY_WITH_PARENT,
                               GTK_MESSAGE_ERROR,
                               GTK_BUTTONS_CLOSE,
                               "%s", e->message);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

static void load_stock ()
{
    GtkIconFactory *factory;
    GError *e = NULL;

    gtk_icon_factory_add_default (factory = gtk_icon_factory_new ());

    ob_icon = gdk_pixbuf_new_from_file (PIXMAPDIR G_DIR_SEPARATOR_S
                                        "openbox.png", &e);
    if (!ob_icon) {
        gchar *msg = g_strdup_printf 
            (_("Failed to load the Openbox icon, Openbox is probably not "
               "installed correctly. The error given was '%s'."),
             e->message);
        g_free (e->message);
        e->message = msg;
        obconf_error (e);
    } else {
        GtkIconSet *set;

        set = gtk_icon_set_new_from_pixbuf (ob_icon);
        gtk_icon_factory_add (factory, OB_ICON, set);
        gtk_icon_set_unref (set);
    }
}

int main(int argc, char **argv)
{
    gtk_set_locale();
    gtk_init(&argc, &argv);

    load_stock();

    mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(mainwin), "Obconf");
    gtk_window_set_wmclass(GTK_WINDOW(mainwin), "obconf", "Obconf");
    gtk_window_set_role(GTK_WINDOW(mainwin), "main window");
    if (ob_icon) gtk_window_set_icon(GTK_WINDOW(mainwin), ob_icon);

    gtk_widget_show_all(mainwin);

    gtk_main();
    return 0;
}

gboolean on_mainwindow_delete_event(GtkWidget *w, GdkEvent *e, gpointer d)
{
    gtk_main_quit();
    return FALSE;
}

void on_quit_activate(GtkMenuItem *item, gpointer d)
{
    gtk_main_quit();
}

void on_applybutton_clicked(GtkButton *but, gpointer d)
{
    g_message("apply\n");
}

void on_revertbutton_clicked(GtkButton *but, gpointer d)
{
    g_message("revert\n");
}

void on_helpbutton_clicked(GtkButton *but, gpointer d)
{
    g_message("help\n");
}

void on_sectiontree_row_activated(GtkTreeView *tree, GtkTreePath *path,
                                  GtkTreeViewColumn *col, gpointer p)
{
    g_message("activated\n");
}
