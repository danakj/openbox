#include "obconf.h"

void on_about_activate(GtkMenuItem *item, gpointer d)
{
    gtk_widget_show(GTK_WIDGET(obconf_about));
}

gboolean on_aboutdialog_delete_event(GtkWidget *w, GdkEvent *e, gpointer d)
{
    gtk_widget_hide(GTK_WIDGET(obconf_about));
    return TRUE;
}

void on_about_closebutton_clicked(GtkButton *but, gpointer d)
{
    gtk_widget_hide(GTK_WIDGET(obconf_about));
}

