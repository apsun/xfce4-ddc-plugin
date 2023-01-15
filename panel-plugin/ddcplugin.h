#ifndef DDCPLUGIN_H
#define DDCPLUGIN_H

#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include "ddcplugin_display.h"
#include "ddcplugin_settings.h"

typedef struct {
    XfcePanelPlugin *plugin;
    DdcPluginSettings settings;
    GtkWidget *widget;
    GtkWidget *settings_dialog;
    DdcDisplay *displays;
} DdcPlugin;

#endif // DDCPLUGIN_H
