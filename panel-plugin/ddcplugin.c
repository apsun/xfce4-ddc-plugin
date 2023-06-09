#include <config.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/libxfce4panel.h>
#include "ddcdisplay.h"
#include "ddcplugin_display.h"
#include "ddcplugin_settings.h"
#include "ddcplugin_keybind.h"
#include "ddcplugin_panel_widget.h"
#include "ddcplugin_settings_dialog.h"

typedef struct {
    XfcePanelPlugin *plugin;
    DdcDisplay *raw_display_list;
    DdcPluginDisplay *display_list;
    DdcPluginSettings *settings;
    DdcPluginKeybind *keybind;
    DdcPluginPanelWidget *panel_widget;
    DdcPluginSettingsDialog *settings_dialog;
} DdcPlugin;

static void
ddcplugin_free(DdcPlugin *ddcplugin)
{
    // Destroy settings dialog
    if (ddcplugin->settings_dialog != NULL) {
        g_object_unref(ddcplugin->settings_dialog);
    }

    // Destroy panel widget
    if (ddcplugin->panel_widget != NULL) {
        gtk_widget_destroy(GTK_WIDGET(ddcplugin->panel_widget));
    }

    // Unregister keybinds
    if (ddcplugin->keybind != NULL) {
        g_object_unref(ddcplugin->keybind);
    }

    // Destroy settings (this will remove notify:: listeners)
    if (ddcplugin->settings != NULL) {
        g_object_unref(ddcplugin->settings);
    }

    // Release display GObject wrappers (each node owns its next
    // pointer, so we only need to unref the head)
    if (ddcplugin->display_list != NULL) {
        g_object_unref(ddcplugin->display_list);
    }

    // Release display resources
    ddcdisplay_list_destroy(ddcplugin->raw_display_list);

    // Free the plugin object
    g_free(ddcplugin);

    g_info("xfce4-ddc-plugin finalized");
}

static void
ddcplugin_new(XfcePanelPlugin *plugin)
{
    DdcPlugin *ddcplugin;

    // Initialize locale
    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    // Create the plugin object
    ddcplugin = g_malloc(sizeof(*ddcplugin));
    ddcplugin->plugin = plugin;
    ddcplugin->raw_display_list = NULL;
    ddcplugin->display_list = NULL;
    ddcplugin->settings = NULL;
    ddcplugin->keybind = NULL;
    ddcplugin->panel_widget = NULL;
    ddcplugin->settings_dialog = NULL;
    g_signal_connect_swapped(
        G_OBJECT(plugin),
        "free-data",
        G_CALLBACK(ddcplugin_free),
        ddcplugin);

    // Acquire display resources
    if (ddcdisplay_list_create(&ddcplugin->raw_display_list) < 0) {
         xfce_dialog_show_warning(
            NULL,
            _("Please ensure your monitor supports DDC and the i2c-dev "
              "kernel module is loaded."),
            _("xfce4-ddc-plugin could not get display list"));
    }

    // Create GObject wrappers for display list
    ddcplugin->display_list = ddcplugin_display_new(ddcplugin->raw_display_list);

    // Load settings
    ddcplugin->settings = ddcplugin_settings_new(
        xfce_panel_plugin_get_property_base(plugin));

    // Register keybinds
    ddcplugin->keybind = ddcplugin_keybind_new(
        ddcplugin->display_list,
        ddcplugin->settings);

    // Create panel widget
    ddcplugin->panel_widget = ddcplugin_panel_widget_new(plugin, ddcplugin->display_list);
    gtk_container_add(GTK_CONTAINER(plugin), GTK_WIDGET(ddcplugin->panel_widget));
    gtk_widget_show_all(GTK_WIDGET(plugin));
    xfce_panel_plugin_add_action_widget(plugin, GTK_WIDGET(ddcplugin->panel_widget));

    // Enable settings dialog menu item
    ddcplugin->settings_dialog = ddcplugin_settings_dialog_new(ddcplugin->settings);
    xfce_panel_plugin_menu_show_configure(plugin);
    g_signal_connect_swapped(
        G_OBJECT(plugin),
        "configure-plugin",
        G_CALLBACK(ddcplugin_settings_dialog_show),
        ddcplugin->settings_dialog);

    g_info("xfce4-ddc-plugin initialized");
}

XFCE_PANEL_PLUGIN_REGISTER(ddcplugin_new);
