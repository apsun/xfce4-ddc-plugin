#ifndef DDCPLUGIN_KEYBIND_H
#define DDCPLUGIN_KEYBIND_H

#include <glib.h>
#include <glib-object.h>
#include "ddcplugin_display.h"
#include "ddcplugin_settings.h"

G_DECLARE_FINAL_TYPE(DdcPluginKeybind, ddcplugin_keybind, DDCPLUGIN, KEYBIND, GObject);

DdcPluginKeybind *ddcplugin_keybind_new(
    DdcPluginDisplay *display_list,
    DdcPluginSettings *settings);

#endif // DDCPLUGIN_KEYBIND_H
