#include "ddcplugin_panel_widget.h"
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include "ddcplugin_display.h"
#include "ddcplugin_panel_menu.h"

struct _DdcPluginPanelWidget {
    GtkBin __parent__;

    XfcePanelPlugin *plugin;
    DdcPluginDisplay *display_list;
    DdcPluginPanelMenu *menu;
    GtkWidget *button;

    gulong signal_handler_deactivate;
};

G_DEFINE_TYPE(DdcPluginPanelWidget, ddcplugin_panel_widget, GTK_TYPE_BIN);

static void
ddcplugin_panel_widget_menu_deactivate(DdcPluginPanelWidget *panel_widget)
{
    if (panel_widget->signal_handler_deactivate > 0) {
        g_signal_handler_disconnect(
            panel_widget->menu,
            panel_widget->signal_handler_deactivate);
        panel_widget->signal_handler_deactivate = 0;
    }

    if (panel_widget->menu != NULL) {
        gtk_menu_popdown(GTK_MENU(panel_widget->menu));
        gtk_menu_detach(GTK_MENU(panel_widget->menu));
        panel_widget->menu = NULL;
    }

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(panel_widget->button), FALSE);
}

static void
ddcplugin_panel_widget_menu_activate(DdcPluginPanelWidget *panel_widget)
{
    if (panel_widget->menu != NULL) {
        g_info("menu already exists");
        return;
    }

    panel_widget->menu = ddcplugin_panel_menu_new(panel_widget->display_list);

    panel_widget->signal_handler_deactivate = g_signal_connect_swapped(
        panel_widget->menu,
        "deactivate",
        G_CALLBACK(ddcplugin_panel_widget_menu_deactivate),
        panel_widget);

    gtk_menu_attach_to_widget(
        GTK_MENU(panel_widget->menu),
        panel_widget->button,
        NULL);

    xfce_panel_plugin_popup_menu(
        panel_widget->plugin,
        GTK_MENU(panel_widget->menu),
        panel_widget->button,
        NULL);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(panel_widget->button), TRUE);
}

static void
ddcplugin_panel_widget_button_toggled(DdcPluginPanelWidget *panel_widget)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(panel_widget->button))) {
        ddcplugin_panel_widget_menu_activate(panel_widget);
    } else {
        ddcplugin_panel_widget_menu_deactivate(panel_widget);
    }
}

static void
ddcplugin_panel_widget_init(DdcPluginPanelWidget *panel_widget)
{
    GtkWidget *icon;

    panel_widget->plugin = NULL;
    panel_widget->display_list = NULL;
    panel_widget->menu = NULL;

    panel_widget->button = xfce_panel_create_toggle_button();
    gtk_container_add(GTK_CONTAINER(panel_widget), panel_widget->button);
    g_signal_connect_swapped(
        panel_widget->button,
        "toggled",
        G_CALLBACK(ddcplugin_panel_widget_button_toggled),
        panel_widget);

    icon = gtk_image_new_from_icon_name("video-display", GTK_ICON_SIZE_BUTTON);
    gtk_container_add(GTK_CONTAINER(panel_widget->button), icon);

    panel_widget->signal_handler_deactivate = 0;
}

DdcPluginPanelWidget *
ddcplugin_panel_widget_new(XfcePanelPlugin *plugin, DdcPluginDisplay *display_list)
{
    DdcPluginPanelWidget *panel_widget;

    panel_widget = g_object_new(DDCPLUGIN_TYPE_PANEL_WIDGET, NULL);
    panel_widget->plugin = g_object_ref(plugin);
    panel_widget->display_list = g_object_ref(display_list);

    return panel_widget;
}

static void
ddcplugin_panel_widget_destroy(GtkWidget *widget)
{
    DdcPluginPanelWidget *panel_widget = DDCPLUGIN_PANEL_WIDGET(widget);

    ddcplugin_panel_widget_menu_deactivate(panel_widget);

    if (panel_widget->button != NULL) {
        gtk_widget_destroy(panel_widget->button);
        panel_widget->button = NULL;
    }

    if (panel_widget->display_list != NULL) {
        g_object_unref(panel_widget->display_list);
        panel_widget->display_list = NULL;
    }

    if (panel_widget->plugin != NULL) {
        g_object_unref(panel_widget->plugin);
        panel_widget->plugin = NULL;
    }

    GTK_WIDGET_CLASS(ddcplugin_panel_widget_parent_class)->destroy(widget);
}

static void
ddcplugin_panel_widget_class_init(DdcPluginPanelWidgetClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->destroy = ddcplugin_panel_widget_destroy;
}
