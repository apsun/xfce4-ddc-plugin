#include "ddcplugin_settings_dialog.h"
#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/libxfce4panel.h>
#include "ddcplugin.h"

static void
ddcplugin_settings_dialog_response(DdcPlugin *ddcplugin, int response)
{
    if (response == GTK_RESPONSE_OK) {
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
    if (ddcplugin->settings_dialog != NULL) {
        g_info("dialog already exists");
        return;
    }

    ddcplugin->settings_dialog = xfce_titled_dialog_new_with_mixed_buttons(
        _("DDC Plugin"),
        GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(ddcplugin->plugin))),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        "window-close", _("_Close"), GTK_RESPONSE_OK,
        NULL);
    gtk_window_set_position(GTK_WINDOW(ddcplugin->settings_dialog), GTK_WIN_POS_CENTER);
    gtk_window_set_icon_name(GTK_WINDOW(ddcplugin->settings_dialog), "video-display");
    gtk_widget_show_all(ddcplugin->settings_dialog);

    g_signal_connect_swapped(
        G_OBJECT(ddcplugin->settings_dialog),
        "response",
        G_CALLBACK(ddcplugin_settings_dialog_response),
        ddcplugin);
}
