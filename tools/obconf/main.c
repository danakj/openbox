#include "kernel/debug.h"
#include "obconf.h"
#include "plugins.h"
#include "parser/parse.h"

#include <gtk/gtk.h>
#include <glade/glade.h>

/*#include <X11/Xlib.h>
Display *ob_display;
int ob_screen;
Window ob_root;*/

GtkWindow *obconf_win;
GtkWindow *obconf_about = NULL;

GtkTreeView *obconf_sections;
GtkListStore *obconf_sections_store;
static GtkCellRenderer *obconf_sections_renderer;
static GtkTreeViewColumn *obconf_sections_column;

GtkNotebook *obconf_options;

static xmlDocPtr doc;
static xmlNodePtr root;

int main(int argc, char **argv)
{
    GladeXML *xml;

    gtk_init(&argc, &argv);

    xml = glade_xml_new("obconf.glade", NULL, NULL);
    glade_xml_signal_autoconnect(xml);

    obconf_win = GTK_WINDOW(glade_xml_get_widget(xml, "mainwindow"));
    gtk_window_set_role(obconf_win, "main");
    obconf_about = GTK_WINDOW(glade_xml_get_widget(xml, "aboutdialog"));
    gtk_window_set_role(obconf_about, "about");
    gtk_window_set_transient_for(obconf_about, obconf_win);
    obconf_sections = GTK_TREE_VIEW(glade_xml_get_widget(xml, "sectiontree"));
    obconf_options = GTK_NOTEBOOK(glade_xml_get_widget(xml,"optionsnotebook"));

    obconf_sections_store = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_tree_view_set_model(obconf_sections,
                            GTK_TREE_MODEL(obconf_sections_store));
    obconf_sections_renderer = gtk_cell_renderer_text_new();
    obconf_sections_column = gtk_tree_view_column_new_with_attributes
        ("Section", obconf_sections_renderer, "text", 0, NULL);
    gtk_tree_view_append_column (obconf_sections, obconf_sections_column);

    parse_load_rc(&doc, &root);

    plugins_load();

    gtk_widget_show(GTK_WIDGET(obconf_win));

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
    ob_debug("apply\n");
}

void on_revertbutton_clicked(GtkButton *but, gpointer d)
{
    ob_debug("revert\n");
}

void on_helpbutton_clicked(GtkButton *but, gpointer d)
{
    ob_debug("help\n");
}

void on_sectiontree_row_activated(GtkTreeView *tree, GtkTreePath *path,
                                  GtkTreeViewColumn *col, gpointer p)
{
    ob_debug("activated\n");
}
