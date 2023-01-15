#include "ddcplugin_settings.h"
#include <stdbool.h>
#include <stdlib.h>
#include <glib.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/libxfce4panel.h>

void
ddcplugin_settings_load(XfcePanelPlugin *plugin, DdcPluginSettings *settings)
{
    char *config_path = NULL;
    XfceRc *config = NULL;

    // Apply defaults
    settings->enable_keybind_brightness = true;
    settings->enable_keybind_volume = true;
    settings->step_size_brightness = 5;
    settings->step_size_volume = 5;

    config_path = xfce_panel_plugin_lookup_rc_file(plugin);
    if (config_path == NULL) {
        g_info("failed to get plugin config path, applying defaults");
        goto exit;
    }

    g_info("loading settings from %s", config_path);

    config = xfce_rc_simple_open(config_path, true);
    if (config == NULL) {
        g_warning("failed to open resource config file");
        goto exit;
    }

    settings->enable_keybind_brightness = xfce_rc_read_bool_entry(
        config, "enable-keybind-brightness", settings->enable_keybind_brightness);
    settings->enable_keybind_volume = xfce_rc_read_bool_entry(
        config, "enable-keybind-volume", settings->enable_keybind_volume);
    settings->step_size_brightness = xfce_rc_read_int_entry(
        config, "step-size-brightness", settings->step_size_brightness);
    settings->step_size_volume = xfce_rc_read_int_entry(
        config, "step-size-volume", settings->step_size_volume);

    g_info("successfully loaded settings");

exit:
    if (config != NULL) {
      xfce_rc_close(config);
    }

    g_free(config_path);
}

void
ddcplugin_settings_save(XfcePanelPlugin *plugin, const DdcPluginSettings *settings)
{
    char *config_path = NULL;
    XfceRc *config = NULL;

    config_path = xfce_panel_plugin_save_location(plugin, true);
    if (config_path == NULL) {
        g_warning("failed to get plugin config path");
        goto exit;
    }

    g_info("writing settings to %s", config_path);

    config = xfce_rc_simple_open(config_path, false);
    if (config == NULL) {
        g_warning("failed to open resource config file");
        goto exit;
    }

    xfce_rc_write_bool_entry(
        config, "enable-keybind-brightness", settings->enable_keybind_brightness);
    xfce_rc_write_bool_entry(
        config, "enable-keybind-volume", settings->enable_keybind_volume);
    xfce_rc_write_int_entry(
        config, "step-size-brightness", settings->step_size_brightness);
    xfce_rc_write_int_entry(
        config, "step-size-volume", settings->step_size_volume);

    g_info("successfully saved settings");

exit:
    if (config != NULL) {
      xfce_rc_close(config);
    }

    g_free(config_path);
}
