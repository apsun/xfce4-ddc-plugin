#include "ddcplugin.h"
#include <config.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <libxfce4ui/libxfce4ui.h>
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
    DdcDisplay *display;
    gint step_size_brightness;

    display = ddcplugin_pick_display(ddcplugin);
    if (display == NULL) {
        return;
    }

    g_object_get(ddcplugin->settings, STEP_SIZE_BRIGHTNESS, &step_size_brightness, NULL);
    ddcplugin_display_modify_brightness(display, step_size_brightness);
}

static void __attribute__((unused))
ddcplugin_keybind_brightness_down(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    DdcDisplay *display;
    gint step_size_brightness;

    display = ddcplugin_pick_display(ddcplugin);
    if (display == NULL) {
        return;
    }

    g_object_get(ddcplugin->settings, STEP_SIZE_BRIGHTNESS, &step_size_brightness, NULL);
    ddcplugin_display_modify_brightness(display, -step_size_brightness);
}

static void __attribute__((unused))
ddcplugin_keybind_volume_up(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    DdcDisplay *display;
    gint step_size_volume;

    display = ddcplugin_pick_display(ddcplugin);
    if (display == NULL) {
        return;
    }

    g_object_get(ddcplugin->settings, STEP_SIZE_VOLUME, &step_size_volume, NULL);
    ddcplugin_display_modify_volume(display, step_size_volume);
}

static void __attribute__((unused))
ddcplugin_keybind_volume_down(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    DdcDisplay *display;
    gint step_size_volume;

    display = ddcplugin_pick_display(ddcplugin);
    if (display == NULL) {
        return;
    }

    g_object_get(ddcplugin->settings, STEP_SIZE_VOLUME, &step_size_volume, NULL);
    ddcplugin_display_modify_volume(display, -step_size_volume);
}

static void __attribute__((unused))
ddcplugin_keybind_mute_toggle(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    DdcDisplay *display;

    display = ddcplugin_pick_display(ddcplugin);
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
    g_info("unregistered brightness keybinds");
}

static void
ddcplugin_keybind_unregister_volume(void)
{
    keybinder_unbind("XF86AudioRaiseVolume", ddcplugin_keybind_volume_up);
    keybinder_unbind("XF86AudioLowerVolume", ddcplugin_keybind_volume_down);
    keybinder_unbind("XF86AudioMute", ddcplugin_keybind_mute_toggle);
    g_info("unregistered volume keybinds");
}

static int
ddcplugin_keybind_register_brightness(DdcPlugin *ddcplugin)
{
    int rc = 0;

    keybinder_init();
    if (!keybinder_bind("XF86MonBrightnessUp", ddcplugin_keybind_brightness_up, ddcplugin) ||
        !keybinder_bind("XF86MonBrightnessDown", ddcplugin_keybind_brightness_down, ddcplugin))
    {
        g_warning("failed to register brightness keybinds");
        rc = -EBUSY;
        goto error;
    }

    g_info("registered brightness keybinds");

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
        g_warning("failed to register volume keybinds");
        rc = -EBUSY;
        goto error;
    }

    g_info("registered volume keybinds");

exit:
    return rc;

error:
    ddcplugin_keybind_unregister_volume();
    goto exit;
}

static void
ddcplugin_keybind_update_brightness(DdcPlugin *ddcplugin)
{
    gboolean enable;
    g_object_get(ddcplugin->settings, ENABLE_KEYBIND_BRIGHTNESS, &enable, NULL);

    ddcplugin_keybind_unregister_brightness();
    if (enable && ddcplugin_keybind_register_brightness(ddcplugin) < 0) {
        xfce_dialog_show_warning(
            NULL,
            _("Failed to grab XF86MonBrightnessUp, XF86MonBrightnessDown keys.\n"
              "Is another program (e.g. xfce4-power-manager) currently using them?"),
            _("xfce4-ddc-plugin could not register brightness keys"));
    }
}

static void
ddcplugin_keybind_update_volume(DdcPlugin *ddcplugin)
{
    gboolean enable;
    g_object_get(ddcplugin->settings, ENABLE_KEYBIND_VOLUME, &enable, NULL);

    ddcplugin_keybind_unregister_volume();
    if (enable && ddcplugin_keybind_register_volume(ddcplugin) < 0) {
        xfce_dialog_show_warning(
            NULL,
            _("Failed to grab XF86AudioRaiseVolume, XF86AudioLowerVolume, "
              "XF86AudioMute keys.\nIs another program (e.g. xfce4-volumed, "
              "xfce4-pulseaudio-plugin) currently using them?"),
            _("xfce4-ddc-plugin could not register volume keys"));
    }
}

static void
ddcplugin_free(DdcPlugin *ddcplugin)
{
    // Destroy the settings dialog if it's open
    ddcplugin_settings_dialog_destroy(ddcplugin);

    // Unregister keybinds
    ddcplugin_keybind_unregister_brightness();
    ddcplugin_keybind_unregister_volume();

    // Destroy panel icon
    if (ddcplugin->widget != NULL) {
        gtk_widget_destroy(ddcplugin->widget);
    }

    // Destroy settings (this will remove notify:: listeners)
    if (ddcplugin->settings != NULL) {
        g_object_unref(ddcplugin->settings);
    }

    // Release display resources
    ddcplugin_display_list_destroy(ddcplugin->displays);

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
    ddcplugin->displays = NULL;
    ddcplugin->widget = NULL;
    ddcplugin->settings = NULL;
    ddcplugin->settings_dialog = NULL;
    g_signal_connect_swapped(
        G_OBJECT(plugin),
        "free-data",
        G_CALLBACK(ddcplugin_free),
        ddcplugin);

    // Acquire display resources
    rc = ddcplugin_display_list_create(&ddcplugin->displays);
    if (rc < 0) {
        g_error("failed to get display list");
    }

    // Create panel icon
    ddcplugin->widget = gtk_image_new_from_icon_name("video-display", GTK_ICON_SIZE_BUTTON);
    gtk_container_add(GTK_CONTAINER(plugin), ddcplugin->widget);
    gtk_widget_show_all(GTK_WIDGET(plugin));
    xfce_panel_plugin_add_action_widget(plugin, ddcplugin->widget);

    // Load settings and add listeners for changes
    ddcplugin->settings = ddcplugin_settings_new(
        xfce_panel_plugin_get_property_base(plugin));
    g_signal_connect_swapped(
        G_OBJECT(ddcplugin->settings),
        "notify::" ENABLE_KEYBIND_BRIGHTNESS,
        G_CALLBACK(ddcplugin_keybind_update_brightness),
        ddcplugin);
    g_signal_connect_swapped(
        G_OBJECT(ddcplugin->settings),
        "notify::" ENABLE_KEYBIND_VOLUME,
        G_CALLBACK(ddcplugin_keybind_update_volume),
        ddcplugin);

    // Register keybinds
    ddcplugin_keybind_update_brightness(ddcplugin);
    ddcplugin_keybind_update_volume(ddcplugin);

    // Enable settings dialog menu item
    xfce_panel_plugin_menu_show_configure(plugin);
    g_signal_connect_swapped(
        G_OBJECT(plugin),
        "configure-plugin",
        G_CALLBACK(ddcplugin_settings_dialog_show),
        ddcplugin);

    g_info("xfce4-ddc-plugin initialized");
}

XFCE_PANEL_PLUGIN_REGISTER(ddcplugin_new);
