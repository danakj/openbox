#include "plugins/obconf_interface.h"
#include "parser/parse.h"
#include "resistance.h"
#include <gtk/gtk.h>
#include <glade/glade.h>

static GtkWidget *conf_widget;
static GtkCheckButton *conf_resist_windows;
static GtkSpinButton *conf_resist_strength;
static gboolean conf_edited = FALSE;

int plugin_interface_version() { return OBCONF_INTERFACE_VERSION; }

char *plugin_name() { return "Resistance"; }
char *plugin_plugin_name() { return "resistance"; }
void plugin_icon() {}

GtkWidget *plugin_toplevel_widget() { return conf_widget; }

gboolean plugin_edited() { return conf_edited; }

void plugin_load(xmlDocPtr doc, xmlNodePtr root)
{
    xmlNodePtr node, n;

    gtk_spin_button_set_value(conf_resist_strength, DEFAULT_RESISTANCE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(conf_resist_windows),
                                 DEFAULT_RESIST_WINDOWS);

    node = parse_find_node("resistance", root);
    while (node) {
        if ((n = parse_find_node("strength", node)))
            gtk_spin_button_set_value(conf_resist_strength,
                                      parse_int(doc, n));
        if ((n = parse_find_node("windows", node)))
            gtk_toggle_button_set_active
                (GTK_TOGGLE_BUTTON(conf_resist_windows),
                 parse_bool(doc, n));

        node = parse_find_node("resistance", node->next);
    }
}

void plugin_save(xmlDocPtr doc, xmlNodePtr root)
{
}

void plugin_startup()
{
    GladeXML *xml;

    xml = glade_xml_new("obconf.glade", NULL, NULL);
    glade_xml_signal_autoconnect(xml);

    conf_widget = glade_xml_get_widget(xml, "resistwindow");
    conf_resist_strength =
        GTK_SPIN_BUTTON(glade_xml_get_widget(xml, "resist_strength"));
    conf_resist_windows =
        GTK_CHECK_BUTTON(glade_xml_get_widget(xml, "resist_windows"));
}

void plugin_shutdown()
{
}
