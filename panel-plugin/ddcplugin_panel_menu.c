#include "ddcplugin_panel_menu.h"
#include <config.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include "ddcplugin_display.h"
#include "ddcplugin_display_menuitem_ui.h"

struct _DdcPluginPanelMenu {
    GtkMenu __parent__;

    DdcPluginDisplay *display_list;
};

G_DEFINE_TYPE(DdcPluginPanelMenu, ddcplugin_panel_menu, GTK_TYPE_MENU);

static void
ddcplugin_panel_menu_init(DdcPluginPanelMenu *panel_menu)
{
    panel_menu->display_list = NULL;
}

static void
ddcplugin_panel_menu_destroy(GtkWidget *widget)
{
    DdcPluginPanelMenu *panel_menu = DDCPLUGIN_PANEL_MENU(widget);

    if (panel_menu->display_list != NULL) {
        g_object_unref(panel_menu->display_list);
        panel_menu->display_list = NULL;
    }

    GTK_WIDGET_CLASS(ddcplugin_panel_menu_parent_class)->destroy(widget);
}

DdcPluginPanelMenu *
ddcplugin_panel_menu_new(DdcPluginDisplay *display_list)
{
    DdcPluginPanelMenu *panel_menu;
    DdcPluginDisplay *display;
    GtkBuilder *builder;
    GtkWidget *content;
    GtkWidget *menuitem;

    panel_menu = g_object_new(DDCPLUGIN_TYPE_PANEL_MENU, NULL);
    panel_menu->display_list = g_object_ref(display_list);
    gtk_menu_set_reserve_toggle_size(GTK_MENU(panel_menu), FALSE);

    display = display_list;
    while (display != NULL) {
        builder = gtk_builder_new();
        gtk_builder_set_translation_domain(builder, GETTEXT_PACKAGE);
        gtk_builder_add_from_string(
            builder,
            ddcplugin_display_menuitem_ui,
            ddcplugin_display_menuitem_ui_length,
            NULL);

        content = GTK_WIDGET(gtk_builder_get_object(builder, "content"));
        menuitem = gtk_menu_item_new();
        gtk_container_add(GTK_CONTAINER(menuitem), content);
        gtk_widget_show_all(menuitem);
        gtk_menu_shell_append(GTK_MENU_SHELL(panel_menu), menuitem);

        g_object_unref(builder);

        display = ddcplugin_display_get_next(display);
    }

    return panel_menu;
}

static void
ddcplugin_panel_menu_class_init(DdcPluginPanelMenuClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->destroy = ddcplugin_panel_menu_destroy;
}
