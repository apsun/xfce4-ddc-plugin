#include "ddcplugin_panel_widget.h"
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include "ddcplugin_display.h"

struct _DdcPluginPanelWidget {
    GtkBin __parent__;

    GtkWidget *button;

    DdcPluginDisplay *display_list;
};

G_DEFINE_TYPE(DdcPluginPanelWidget, ddcplugin_panel_widget, GTK_TYPE_BIN);

static void
ddcplugin_panel_widget_init(DdcPluginPanelWidget *panel_widget)
{
    GtkWidget *icon;

    panel_widget->button = xfce_panel_create_toggle_button();
    gtk_container_add(GTK_CONTAINER(panel_widget), panel_widget->button);

    icon = gtk_image_new_from_icon_name("video-display", GTK_ICON_SIZE_BUTTON);
    gtk_container_add(GTK_CONTAINER(panel_widget->button), icon);
}

DdcPluginPanelWidget *
ddcplugin_panel_widget_new(DdcPluginDisplay *display_list)
{
    DdcPluginPanelWidget *panel_widget;

    panel_widget = g_object_new(DDCPLUGIN_TYPE_PANEL_WIDGET, NULL);
    panel_widget->display_list = g_object_ref(display_list);

    return panel_widget;
}

static void
ddcplugin_panel_widget_destroy(GtkWidget *widget)
{
    DdcPluginPanelWidget *panel_widget = DDCPLUGIN_PANEL_WIDGET(widget);

    if (panel_widget->button != NULL) {
        gtk_widget_destroy(panel_widget->button);
        panel_widget->button = NULL;
    }

    if (panel_widget->display_list != NULL) {
        g_object_unref(panel_widget->display_list);
        panel_widget->display_list = NULL;
    }

    GTK_WIDGET_CLASS(ddcplugin_panel_widget_parent_class)->destroy(widget);
}

static void
ddcplugin_panel_widget_class_init(DdcPluginPanelWidgetClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->destroy = ddcplugin_panel_widget_destroy;
}
