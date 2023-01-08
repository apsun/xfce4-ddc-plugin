#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/libxfce4panel.h>

typedef struct {
    XfcePanelPlugin *plugin;
    GtkWidget *root;
} DdcPlugin;

static DdcPlugin *
ddcplugin_new(XfcePanelPlugin *plugin)
{
    DdcPlugin *ddcplugin;
    GtkWidget *label;

    ddcplugin = g_slice_new0(DdcPlugin);
    ddcplugin->plugin = plugin;

    ddcplugin->root = gtk_event_box_new();
    gtk_widget_show(ddcplugin->root);

    label = gtk_label_new(_("Hello world"));
    gtk_widget_show(label);
    gtk_container_add(GTK_CONTAINER(ddcplugin->root), label);

    return ddcplugin;
}

static void
ddcplugin_free(XfcePanelPlugin *plugin, DdcPlugin *ddcplugin)
{
    gtk_widget_destroy(ddcplugin->root);

    g_slice_free(DdcPlugin, ddcplugin);
}

static void
ddcplugin_construct(XfcePanelPlugin *plugin)
{
    DdcPlugin *ddcplugin;

    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    ddcplugin = ddcplugin_new(plugin);

    gtk_container_add(GTK_CONTAINER(plugin), ddcplugin->root);

    xfce_panel_plugin_add_action_widget(plugin, ddcplugin->root);

    g_signal_connect(G_OBJECT(plugin), "free-data", G_CALLBACK(ddcplugin_free), ddcplugin);
}

XFCE_PANEL_PLUGIN_REGISTER(ddcplugin_construct);
