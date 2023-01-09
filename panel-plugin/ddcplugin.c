#include <config.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/libxfce4panel.h>
#include <keybinder.h>
#include <ddcutil_c_api.h>

typedef struct {
    XfcePanelPlugin *plugin;
    GtkWidget *widget;
} DdcPlugin;

static void
ddcplugin_volume_up(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    fprintf(stderr, "%s\n", keystring);
}

static void
ddcplugin_volume_down(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    fprintf(stderr, "%s\n", keystring);
}

static void
ddcplugin_mute_toggle(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    fprintf(stderr, "%s\n", keystring);
}

static void
ddcplugin_free(XfcePanelPlugin *plugin, DdcPlugin *ddcplugin)
{
    // Release keybindings
    keybinder_unbind("XF86AudioRaiseVolume", ddcplugin_volume_up);
    keybinder_unbind("XF86AudioLowerVolume", ddcplugin_volume_down);
    keybinder_unbind("XF86AudioMute", ddcplugin_mute_toggle);

    // Destroy the icon
    gtk_widget_destroy(ddcplugin->widget);
    ddcplugin->widget = NULL;

    // Destroy the plugin object
    g_slice_free(DdcPlugin, ddcplugin);
}

static void
ddcplugin_new(XfcePanelPlugin *plugin)
{
    DdcPlugin *ddcplugin;

    // Initialize locale
    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    // Create the plugin object
    ddcplugin = g_slice_new0(DdcPlugin);
    ddcplugin->plugin = plugin;
    g_signal_connect(G_OBJECT(plugin), "free-data", G_CALLBACK(ddcplugin_free), ddcplugin);

    // Create the icon
    ddcplugin->widget = gtk_image_new_from_icon_name("video-display", GTK_ICON_SIZE_BUTTON);
    gtk_container_add(GTK_CONTAINER(plugin), ddcplugin->widget);
    gtk_widget_show_all(GTK_WIDGET(plugin));
    xfce_panel_plugin_add_action_widget(plugin, ddcplugin->widget);

    // Set up keybindings
    keybinder_init();
    if (
        !keybinder_bind("XF86AudioRaiseVolume", ddcplugin_volume_up, ddcplugin) ||
        !keybinder_bind("XF86AudioLowerVolume", ddcplugin_volume_down, ddcplugin) ||
        !keybinder_bind("XF86AudioMute", ddcplugin_mute_toggle, ddcplugin))
    {
        fprintf(stderr, "failed to bind keys - already in use?\n");
    }

    fprintf(stderr, "xfce4-ddc-plugin initialized\n");
    fprintf(stderr, "ddcutil version: %s\n", ddca_ddcutil_version_string());
}

XFCE_PANEL_PLUGIN_REGISTER(ddcplugin_new);
