#ifndef DDCPLUGIN_PANEL_MENU_H
#define DDCPLUGIN_PANEL_MENU_H

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include "ddcplugin_display.h"

G_DECLARE_FINAL_TYPE(
    DdcPluginPanelMenu,
    ddcplugin_panel_menu,
    DDCPLUGIN,
    PANEL_MENU,
    GtkMenu);
#define DDCPLUGIN_TYPE_PANEL_MENU ddcplugin_panel_menu_get_type()

DdcPluginPanelMenu *ddcplugin_panel_menu_new(DdcPluginDisplay *display_list);

#endif // DDCPLUGIN_PANEL_MENU_H
