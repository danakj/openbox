/*#include "obconf.h"
  #include "plugins.h"*/
#include "parser/parse.h"
#include "gettext.h"

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#define OB_ICON "openbox-icon"

static GtkWidget *mainwin;
static GtkWidget *mainlist;
static GtkListStore *mainstore;
static GtkWidget *mainworkarea;
static GdkPixbuf *ob_icon;

enum {
    NAME_COLUMN,
    N_COLUMNS
};

gboolean on_mainwindow_delete_event(GtkWidget *w, GdkEvent *e, gpointer d);
void on_quit_activate(GtkMenuItem *item, gpointer d);
void on_applybutton_clicked(GtkButton *but, gpointer d);
void on_revertbutton_clicked(GtkButton *but, gpointer d);
void on_helpbutton_clicked(GtkButton *but, gpointer d);
void on_selection_changed(GtkTreeSelection *selection, gpointer data);

static void obconf_error(GError *e)
{
    GtkWidget *d;

    d = gtk_message_dialog_new(mainwin ? GTK_WINDOW(mainwin) : NULL,
                               GTK_DIALOG_DESTROY_WITH_PARENT,
                               GTK_MESSAGE_ERROR,
                               GTK_BUTTONS_CLOSE,
                               "%s", e->message);
    g_signal_connect_swapped(GTK_OBJECT(d), "response",
                             G_CALLBACK(gtk_widget_destroy),
                             GTK_OBJECT(d));
    gtk_widget_show(d);
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

GtkWidget* build_menu(GtkAccelGroup *accel)
{
    GtkWidget *menu;
    GtkWidget *submenu;
    GtkWidget *item;

    menu = gtk_menu_bar_new();

    /* File menu */

    submenu = gtk_menu_new();
    gtk_menu_set_accel_group(GTK_MENU(submenu), accel);

    item = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, accel);
    g_signal_connect(item, "activate", G_CALLBACK(on_quit_activate), NULL);
    gtk_menu_append(GTK_MENU(submenu), item);

    item = gtk_menu_item_new_with_mnemonic("_File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);
    gtk_menu_bar_append(GTK_MENU_BAR(menu), item);

    /* About menu */

    submenu = gtk_menu_new();
    gtk_menu_set_accel_group(GTK_MENU(submenu), accel);

    item = gtk_menu_item_new_with_mnemonic("_About");
    gtk_menu_append(GTK_MENU(submenu), item);

    item = gtk_menu_item_new_with_mnemonic("_Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);
    gtk_menu_bar_append(GTK_MENU_BAR(menu), item);

    gtk_widget_show_all(menu);

    return menu;
}

GtkWidget* build_list(GtkListStore **model)
{
    GtkWidget *list;
    GtkListStore *store;
    GtkCellRenderer *ren;
    GtkTreeViewColumn *col;
    GtkTreeSelection *sel;

    store = gtk_list_store_new(N_COLUMNS,
                               G_TYPE_STRING);

    list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), FALSE);

    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);
    g_signal_connect(sel, "changed", G_CALLBACK(on_selection_changed), NULL);
    
    ren = gtk_cell_renderer_text_new();
    col = gtk_tree_view_column_new_with_attributes("Name",
                                                   ren,
                                                   "text",
                                                   NAME_COLUMN,
                                                   NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), col);

    *model = store;
    return list;
}

int main(int argc, char **argv)
{
    GtkWidget *menu;
    GtkWidget *vbox;
    GtkWidget *hpane;
    GtkAccelGroup *accel;

    gtk_set_locale();
    gtk_init(&argc, &argv);

    mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(mainwin), "Obconf");
    gtk_window_set_wmclass(GTK_WINDOW(mainwin), "obconf", "Obconf");
    gtk_window_set_role(GTK_WINDOW(mainwin), "main window");

    g_signal_connect(mainwin, "delete-event",
                     G_CALLBACK(on_mainwindow_delete_event), NULL);

    accel = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(mainwin), accel);

    vbox = gtk_vbox_new(FALSE, 3);
    gtk_container_add(GTK_CONTAINER(mainwin), vbox);

    /* Menu */

    menu = build_menu(accel);
    gtk_box_pack_start(GTK_BOX(vbox), menu, FALSE, FALSE, 0);

    hpane = gtk_hpaned_new();
    gtk_box_pack_start(GTK_BOX(vbox), hpane, TRUE, TRUE, 0);

    /* List */

    mainlist = build_list(&mainstore);
    gtk_container_add(GTK_CONTAINER(hpane), mainlist);

    /* Main work area */

    mainworkarea = gtk_vbox_new(FALSE, 1);
    gtk_container_add(GTK_CONTAINER(hpane), mainworkarea);

    gtk_widget_show_all(mainwin);

    load_stock();
    if (ob_icon) gtk_window_set_icon(GTK_WINDOW(mainwin), ob_icon);

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
    g_message("apply");
}

void on_revertbutton_clicked(GtkButton *but, gpointer d)
{
    g_message("revert");
}

void on_helpbutton_clicked(GtkButton *but, gpointer d)
{
    g_message("help");
}

void on_selection_changed(GtkTreeSelection *sel, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;

    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
        g_message("activated");
    } else {
        g_message("none activated");
    }
}
