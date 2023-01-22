#ifndef DDCPLUGIN_PANEL_WIDGET_H
#define DDCPLUGIN_PANEL_WIDGET_H

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include "ddcplugin_display.h"

G_DECLARE_FINAL_TYPE(
    DdcPluginPanelWidget,
    ddcplugin_panel_widget,
    DDCPLUGIN,
    PANEL_WIDGET,
    GtkBin);
#define DDCPLUGIN_TYPE_PANEL_WIDGET ddcplugin_panel_widget_get_type()

DdcPluginPanelWidget *ddcplugin_panel_widget_new(
    XfcePanelPlugin *plugin,
    DdcPluginDisplay *display_list);

#endif // DDCPLUGIN_PANEL_WIDGET_H
