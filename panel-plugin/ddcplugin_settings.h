#ifndef DDCPLUGIN_SETTINGS_H
#define DDCPLUGIN_SETTINGS_H

#include <glib.h>
#include <glib-object.h>

#define ENABLE_KEYBIND_BRIGHTNESS "enable-keybind-brightness"
#define ENABLE_KEYBIND_VOLUME "enable-keybind-volume"
#define STEP_SIZE_BRIGHTNESS "step-size-brightness"
#define STEP_SIZE_VOLUME "step-size-volume"

G_DECLARE_FINAL_TYPE(DdcPluginSettings, ddcplugin_settings, DDCPLUGIN, SETTINGS, GObject);

DdcPluginSettings *ddcplugin_settings_new(const gchar *property_base);

#endif // DDCPLUGIN_SETTINGS_H
