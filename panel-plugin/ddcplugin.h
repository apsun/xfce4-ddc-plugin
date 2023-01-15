#ifndef DDCPLUGIN_H
#define DDCPLUGIN_H

#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include "ddcplugin_display.h"
#include "ddcplugin_settings.h"

typedef struct {
    XfcePanelPlugin *plugin;
    DdcDisplay *displays;
    GtkWidget *widget;
    DdcPluginSettings *settings;
    GtkWidget *settings_dialog;
} DdcPlugin;

#endif // DDCPLUGIN_H
