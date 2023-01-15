#ifndef DDCPLUGIN_SETTINGS_DIALOG_H
#define DDCPLUGIN_SETTINGS_DIALOG_H

#include <libxfce4panel/libxfce4panel.h>
#include "ddcplugin.h"

void ddcplugin_settings_dialog_destroy(DdcPlugin *ddcplugin);

void ddcplugin_settings_dialog_show(XfcePanelPlugin *plugin, DdcPlugin *ddcplugin);

#endif // DDCPLUGIN_SETTINGS_DIALOG_H
