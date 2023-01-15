#include "ddcplugin.h"
#include <config.h>
#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/libxfce4panel.h>
#include <keybinder.h>
#include "ddcplugin_display.h"
#include "ddcplugin_settings.h"
#include "ddcplugin_settings_dialog.h"

static DdcDisplay *
ddcplugin_pick_display(DdcPlugin *ddcplugin)
{
    DdcDisplay *display = ddcplugin->displays;
    if (display == NULL) {
        g_warning("no displays detected");
        return NULL;
    }

    if (display->next == NULL) {
        return display;
    }

    // TODO: handle multiple monitors
    g_warning("more than one display detected");
    return display;
}

static void __attribute__((unused))
ddcplugin_keybind_brightness_up(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    DdcDisplay *display = ddcplugin_pick_display(ddcplugin);
    if (display == NULL) {
        return;
    }

    ddcplugin_display_modify_brightness(display, ddcplugin->settings.step_size_brightness);
}

static void __attribute__((unused))
ddcplugin_keybind_brightness_down(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    DdcDisplay *display = ddcplugin_pick_display(ddcplugin);
    if (display == NULL) {
        return;
    }

    ddcplugin_display_modify_brightness(display, -ddcplugin->settings.step_size_brightness);
}

static void __attribute__((unused))
ddcplugin_keybind_volume_up(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    DdcDisplay *display = ddcplugin_pick_display(ddcplugin);
    if (display == NULL) {
        return;
    }

    ddcplugin_display_modify_volume(display, ddcplugin->settings.step_size_volume);
}

static void __attribute__((unused))
ddcplugin_keybind_volume_down(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    DdcDisplay *display = ddcplugin_pick_display(ddcplugin);
    if (display == NULL) {
        return;
    }

    ddcplugin_display_modify_volume(display, -ddcplugin->settings.step_size_volume);
}

static void __attribute__((unused))
ddcplugin_keybind_mute_toggle(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    DdcDisplay *display = ddcplugin_pick_display(ddcplugin);
    if (display == NULL) {
        return;
    }

    ddcplugin_display_toggle_mute(display);
}

static void
ddcplugin_keybind_unregister_brightness(void)
{
    keybinder_unbind("XF86MonBrightnessUp", ddcplugin_keybind_brightness_up);
    keybinder_unbind("XF86MonBrightnessDown", ddcplugin_keybind_brightness_down);
}

static void
ddcplugin_keybind_unregister_volume(void)
{
    keybinder_unbind("XF86AudioRaiseVolume", ddcplugin_keybind_volume_up);
    keybinder_unbind("XF86AudioLowerVolume", ddcplugin_keybind_volume_down);
    keybinder_unbind("XF86AudioMute", ddcplugin_keybind_mute_toggle);
}

static int
ddcplugin_keybind_register_brightness(DdcPlugin *ddcplugin)
{
    int rc = 0;

    keybinder_init();
    if (!keybinder_bind("XF86MonBrightnessUp", ddcplugin_keybind_brightness_up, ddcplugin) ||
        !keybinder_bind("XF86MonBrightnessDown", ddcplugin_keybind_brightness_down, ddcplugin))
    {
        g_warning("failed to bind brightness keys - already in use?");
        rc = -EBUSY;
        goto error;
    }

exit:
    return rc;

error:
    ddcplugin_keybind_unregister_brightness();
    goto exit;
}

static int
ddcplugin_keybind_register_volume(DdcPlugin *ddcplugin)
{
    int rc = 0;

    keybinder_init();
    if (!keybinder_bind("XF86AudioRaiseVolume", ddcplugin_keybind_volume_up, ddcplugin) ||
        !keybinder_bind("XF86AudioLowerVolume", ddcplugin_keybind_volume_down, ddcplugin) ||
        !keybinder_bind("XF86AudioMute", ddcplugin_keybind_mute_toggle, ddcplugin))
    {
        g_warning("failed to bind volume keys - already in use?");
        rc = -EBUSY;
        goto error;
    }

exit:
    return rc;

error:
    ddcplugin_keybind_unregister_volume();
    goto exit;
}

static void
ddcplugin_free(XfcePanelPlugin *plugin, DdcPlugin *ddcplugin)
{
    // Destroy the settings dialog if it's open
    ddcplugin_settings_dialog_destroy(ddcplugin);

    // Unregister keybinds
    ddcplugin_keybind_unregister_brightness();
    ddcplugin_keybind_unregister_volume();

    // Release display resources
    ddcplugin_display_list_destroy(ddcplugin->displays);

    // Destroy panel icon
    if (ddcplugin->widget != NULL) {
        gtk_widget_destroy(ddcplugin->widget);
    }

    // Free the plugin object
    g_free(ddcplugin);

    g_info("xfce4-ddc-plugin finalized");
}

static void
ddcplugin_new(XfcePanelPlugin *plugin)
{
    int rc = 0;
    DdcPlugin *ddcplugin;

    // Initialize locale
    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    // Create the plugin object
    ddcplugin = g_malloc(sizeof(*ddcplugin));
    ddcplugin->plugin = plugin;
    ddcplugin->widget = NULL;
    ddcplugin->settings_dialog = NULL;
    ddcplugin->displays = NULL;
    g_signal_connect(G_OBJECT(plugin), "free-data", G_CALLBACK(ddcplugin_free), ddcplugin);

    // Create panel icon
    ddcplugin->widget = gtk_image_new_from_icon_name("video-display", GTK_ICON_SIZE_BUTTON);
    gtk_container_add(GTK_CONTAINER(plugin), ddcplugin->widget);
    gtk_widget_show_all(GTK_WIDGET(plugin));
    xfce_panel_plugin_add_action_widget(plugin, ddcplugin->widget);

    // Load settings
    ddcplugin_settings_load(plugin, &ddcplugin->settings);
    g_signal_connect(
        G_OBJECT(plugin),
        "save",
        G_CALLBACK(ddcplugin_settings_save),
        &ddcplugin->settings);

    // Hook up settings dialog
    xfce_panel_plugin_menu_show_configure(plugin);
    g_signal_connect(
        G_OBJECT(plugin),
        "configure-plugin",
        G_CALLBACK(ddcplugin_settings_dialog_show),
        ddcplugin);

    // Acquire display resources
    rc = ddcplugin_display_list_create(&ddcplugin->displays);
    if (rc < 0) {
        g_error("failed to get display list");
    }

    // Register brightness keybinds
    if (ddcplugin->settings.enable_keybind_brightness) {
        rc = ddcplugin_keybind_register_brightness(ddcplugin);
        if (rc < 0) {
            g_error("failed to register brightness keybinds");
        }
    }

    // Register volume keybinds
    if (ddcplugin->settings.enable_keybind_volume) {
        rc = ddcplugin_keybind_register_volume(ddcplugin);
        if (rc < 0) {
            g_error("failed to register volume keybinds");
        }
    }

    g_info("xfce4-ddc-plugin initialized");
}

XFCE_PANEL_PLUGIN_REGISTER(ddcplugin_new);
