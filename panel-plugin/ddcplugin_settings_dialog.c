#include "ddcplugin_settings_dialog.h"
#include <config.h>
#include <stdbool.h>
#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/libxfce4panel.h>
#include "ddcplugin.h"
#include "ddcplugin_settings_dialog_ui.h"

static void
ddcplugin_settings_dialog_response(DdcPlugin *ddcplugin, int response)
{
    if (response == GTK_RESPONSE_CLOSE) {
        ddcplugin_settings_save(ddcplugin->plugin, &ddcplugin->settings);
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
    GtkBox *content_area;
    GtkWidget *content;

    // Ensure single dialog instance
    dialog = ddcplugin->settings_dialog;
    if (dialog != NULL) {
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
    content = GTK_WIDGET(gtk_builder_get_object(builder, "content"));
    content_area = GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));
    gtk_box_pack_start(content_area, content, true, true, 0);

    // Add handler for close response
    g_signal_connect_swapped(
        G_OBJECT(dialog),
        "response",
        G_CALLBACK(ddcplugin_settings_dialog_response),
        ddcplugin);

    // Go!
    gtk_widget_show_all(dialog);
}
