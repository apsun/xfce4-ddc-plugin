#include "ddcplugin_settings_dialog.h"
#include <config.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include "ddcplugin_settings.h"
#include "ddcplugin_settings_dialog_ui.h"

struct _DdcPluginSettingsDialogClass {
    GObject __parent__;
};

struct _DdcPluginSettingsDialog {
    GObject __parent__;

    DdcPluginSettings *settings;
    GtkWidget *dialog;
};

G_DEFINE_TYPE(DdcPluginSettingsDialog, ddcplugin_settings_dialog, G_TYPE_OBJECT);

static void
ddcplugin_settings_dialog_dispose(GObject *object)
{
    DdcPluginSettingsDialog *dialog = DDCPLUGIN_SETTINGS_DIALOG(object);

    ddcplugin_settings_dialog_hide(dialog);

    if (dialog->settings != NULL) {
        g_object_unref(dialog->settings);
        dialog->settings = NULL;
    }

    G_OBJECT_CLASS(ddcplugin_settings_dialog_parent_class)->dispose(object);
}

static void
ddcplugin_settings_dialog_class_init(DdcPluginSettingsDialogClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = ddcplugin_settings_dialog_dispose;
}

static void
ddcplugin_settings_dialog_init(DdcPluginSettingsDialog *dialog)
{
    dialog->settings = NULL;
    dialog->dialog = NULL;
}

void
ddcplugin_settings_dialog_hide(DdcPluginSettingsDialog *dialog)
{
    if (dialog->dialog != NULL) {
        gtk_widget_destroy(dialog->dialog);
        dialog->dialog = NULL;
    }
}

void
ddcplugin_settings_dialog_show(DdcPluginSettingsDialog *dialog)
{
    GtkBuilder *builder;

    if (dialog->dialog != NULL) {
        g_info("dialog already exists");
        return;
    }

    // Inflate dialog from .glade file
    builder = gtk_builder_new();
    gtk_builder_set_translation_domain(builder, GETTEXT_PACKAGE);
    gtk_builder_add_from_string(
        builder,
        ddcplugin_settings_dialog_ui,
        ddcplugin_settings_dialog_ui_length,
        NULL);
    dialog->dialog = GTK_WIDGET(gtk_builder_get_object(builder, "dialog"));

    // Close dialog when response is emitted
    g_signal_connect_swapped(
        G_OBJECT(dialog->dialog),
        "response",
        G_CALLBACK(ddcplugin_settings_dialog_hide),
        dialog);

    // Bind controls to settings object properties
    g_object_bind_property(
        dialog->settings,
        ENABLE_KEYBIND_BRIGHTNESS,
        gtk_builder_get_object(builder, "switch-enable-hotkeys-brightness"),
        "active",
        G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

    g_object_bind_property(
        dialog->settings,
        ENABLE_KEYBIND_VOLUME,
        gtk_builder_get_object(builder, "switch-enable-hotkeys-volume"),
        "active",
        G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

    g_object_bind_property(
        dialog->settings,
        STEP_SIZE_BRIGHTNESS,
        gtk_builder_get_object(builder, "adjustment-step-size-brightness"),
        "value",
        G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

    g_object_bind_property(
        dialog->settings,
        STEP_SIZE_VOLUME,
        gtk_builder_get_object(builder, "adjustment-step-size-volume"),
        "value",
        G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

    // Destroy builder once we've referenced all of its contents
    g_object_unref(builder);

    // Go!
    gtk_widget_show_all(dialog->dialog);
}

DdcPluginSettingsDialog *
ddcplugin_settings_dialog_new(DdcPluginSettings *settings)
{
    DdcPluginSettingsDialog *dialog;

    dialog = g_object_new(ddcplugin_settings_dialog_get_type(), NULL);
    dialog->settings = g_object_ref(settings);
    return dialog;
}
