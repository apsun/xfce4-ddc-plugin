#ifndef DDCPLUGIN_SETTINGS_H
#define DDCPLUGIN_SETTINGS_H

#include <stdbool.h>
#include <libxfce4panel/libxfce4panel.h>

typedef struct {
	bool enable_keybind_brightness;
	bool enable_keybind_volume;
	int step_size_brightness;
	int step_size_volume;
} DdcPluginSettings;

void ddcplugin_settings_load(XfcePanelPlugin *plugin, DdcPluginSettings *settings);

void ddcplugin_settings_save(XfcePanelPlugin *plugin, const DdcPluginSettings *settings);

#endif // DDCPLUGIN_SETTINGS_H
