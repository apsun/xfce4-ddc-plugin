#include "ddcplugin_settings_dialog.h"
#include <config.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/libxfce4panel.h>
#include "ddcplugin.h"
#include "ddcplugin_settings.h"
#include "ddcplugin_settings_dialog_ui.h"

static void
ddcplugin_settings_dialog_response(DdcPlugin *ddcplugin, int response)
{
    if (response == GTK_RESPONSE_CLOSE) {
        ddcplugin_settings_dialog_destroy(ddcplugin);
        return;
    }

    g_warning("unknown response code: %d", response);
}

void
ddcplugin_settings_dialog_destroy(DdcPlugin *ddcplugin)
{
    if (ddcplugin->settings_dialog != NULL) {
        gtk_widget_destroy(ddcplugin->settings_dialog);
        ddcplugin->settings_dialog = NULL;
    }
}

void
ddcplugin_settings_dialog_show(DdcPlugin *ddcplugin)
{
    GtkWidget *dialog;
    GtkBuilder *builder;

    // Ensure single dialog instance
    if (ddcplugin->settings_dialog != NULL) {
        g_info("dialog already exists");
        return;
    }

    // Create dialog with empty content area
    dialog = xfce_titled_dialog_new_with_mixed_buttons(
        _("DDC Plugin"),
        GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(ddcplugin->plugin))),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        "window-close", _("_Close"), GTK_RESPONSE_CLOSE,
        NULL);
    ddcplugin->settings_dialog = dialog;
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_window_set_icon_name(GTK_WINDOW(dialog), "video-display");

    // Inflate controls from .glade file
    builder = gtk_builder_new();
    gtk_builder_set_translation_domain(builder, GETTEXT_PACKAGE);
    gtk_builder_add_from_string(
        builder,
        ddcplugin_settings_dialog_ui,
        ddcplugin_settings_dialog_ui_length,
        NULL);

    // Add controls into dialog content area
    gtk_box_pack_start(
        GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
        GTK_WIDGET(gtk_builder_get_object(builder, "content")),
        TRUE, TRUE, 0);

    // Add handler for close response
    g_signal_connect_swapped(
        G_OBJECT(dialog),
        "response",
        G_CALLBACK(ddcplugin_settings_dialog_response),
        ddcplugin);

    // Bind controls to settings object properties
    g_object_bind_property(
        ddcplugin->settings,
        ENABLE_KEYBIND_BRIGHTNESS,
        gtk_builder_get_object(builder, "switch-enable-hotkeys-brightness"),
        "active",
        G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

    g_object_bind_property(
        ddcplugin->settings,
        ENABLE_KEYBIND_VOLUME,
        gtk_builder_get_object(builder, "switch-enable-hotkeys-volume"),
        "active",
        G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

    g_object_bind_property(
        ddcplugin->settings,
        STEP_SIZE_BRIGHTNESS,
        gtk_builder_get_object(builder, "adjustment-step-size-brightness"),
        "value",
        G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

    g_object_bind_property(
        ddcplugin->settings,
        STEP_SIZE_VOLUME,
        gtk_builder_get_object(builder, "adjustment-step-size-volume"),
        "value",
        G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

    // Go!
    gtk_widget_show_all(dialog);
}
