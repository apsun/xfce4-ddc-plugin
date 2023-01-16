#ifndef DDCPLUGIN_DISPLAY_H
#define DDCPLUGIN_DISPLAY_H

#include <glib.h>
#include <glib-object.h>
#include "ddcdisplay.h"

#define MODEL "model"
#define SERIAL "serial"
#define BRIGHTNESS "brightness"
#define VOLUME "volume"
#define MUTED "muted"

G_DECLARE_FINAL_TYPE(DdcPluginDisplay, ddcplugin_display, DDCPLUGIN, DISPLAY, GObject);

DdcPluginDisplay *ddcplugin_display_list_new(DdcDisplay *raw_display_list);
void ddcplugin_display_list_destroy(DdcPluginDisplay *display_list);

DdcPluginDisplay *ddcplugin_display_get_next(DdcPluginDisplay *display);
void ddcplugin_display_modify_brightness(DdcPluginDisplay *display, gint delta);
void ddcplugin_display_modify_volume(DdcPluginDisplay *display, gint delta);
void ddcplugin_display_toggle_muted(DdcPluginDisplay *display);

#endif // DDCPLUGIN_DISPLAY_H
