#ifndef DDCPLUGIN_SETTINGS_DIALOG_H
#define DDCPLUGIN_SETTINGS_DIALOG_H

#include <glib.h>
#include <glib-object.h>
#include "ddcplugin_settings.h"

G_DECLARE_FINAL_TYPE(
    DdcPluginSettingsDialog,
    ddcplugin_settings_dialog,
    DDCPLUGIN,
    SETTINGS_DIALOG,
    GObject);

DdcPluginSettingsDialog *ddcplugin_settings_dialog_new(DdcPluginSettings *settings);

void ddcplugin_settings_dialog_hide(DdcPluginSettingsDialog *dialog);
void ddcplugin_settings_dialog_show(DdcPluginSettingsDialog *dialog);

#endif // DDCPLUGIN_SETTINGS_DIALOG_H
