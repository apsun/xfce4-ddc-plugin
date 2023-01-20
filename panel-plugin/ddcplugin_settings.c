#include "ddcplugin_settings.h"
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <libxfce4util/libxfce4util.h>
#include <xfconf/xfconf.h>

#define DEFAULT_ENABLE_KEYBIND_BRIGHTNESS TRUE
#define DEFAULT_ENABLE_KEYBIND_VOLUME TRUE
#define DEFAULT_STEP_SIZE_BRIGHTNESS 5
#define DEFAULT_STEP_SIZE_VOLUME 5

struct _DdcPluginSettings {
    GObject __parent__;

    gboolean enable_keybind_brightness;
    gboolean enable_keybind_volume;
    gint step_size_brightness;
    gint step_size_volume;

    XfconfChannel *channel;
};

G_DEFINE_TYPE(DdcPluginSettings, ddcplugin_settings, G_TYPE_OBJECT);

enum {
    PROP_ENABLE_KEYBIND_BRIGHTNESS = 1,
    PROP_ENABLE_KEYBIND_VOLUME,
    PROP_STEP_SIZE_BRIGHTNESS,
    PROP_STEP_SIZE_VOLUME,
    N_PROPERTIES
};

static GParamSpec *ddcplugin_settings_properties[N_PROPERTIES] = { NULL, };

static void
ddcplugin_settings_set_property(
    GObject *object,
    guint prop_id,
    const GValue *value,
    GParamSpec *pspec)
{
    DdcPluginSettings *settings = DDCPLUGIN_SETTINGS(object);
    switch (prop_id) {
    case PROP_ENABLE_KEYBIND_BRIGHTNESS:
        settings->enable_keybind_brightness = g_value_get_boolean(value);
        g_info("set %s -> %d", pspec->name, settings->enable_keybind_brightness);
        break;
    case PROP_ENABLE_KEYBIND_VOLUME:
        settings->enable_keybind_volume = g_value_get_boolean(value);
        g_info("set %s -> %d", pspec->name, settings->enable_keybind_volume);
        break;
    case PROP_STEP_SIZE_BRIGHTNESS:
        settings->step_size_brightness = g_value_get_int(value);
        g_info("set %s -> %d", pspec->name, settings->step_size_brightness);
        break;
    case PROP_STEP_SIZE_VOLUME:
        settings->step_size_volume = g_value_get_int(value);
        g_info("set %s -> %d", pspec->name, settings->step_size_volume);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
ddcplugin_settings_get_property(
    GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
    DdcPluginSettings *settings = DDCPLUGIN_SETTINGS(object);
    switch (prop_id) {
    case PROP_ENABLE_KEYBIND_BRIGHTNESS:
        g_value_set_boolean(value, settings->enable_keybind_brightness);
        break;
    case PROP_ENABLE_KEYBIND_VOLUME:
        g_value_set_boolean(value, settings->enable_keybind_volume);
        break;
    case PROP_STEP_SIZE_BRIGHTNESS:
        g_value_set_int(value, settings->step_size_brightness);
        break;
    case PROP_STEP_SIZE_VOLUME:
        g_value_set_int(value, settings->step_size_volume);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static gulong
ddcplugin_settings_property_bind(
    const gchar *property_base,
    XfconfChannel *channel,
    const gchar *property_name,
    GType property_type,
    DdcPluginSettings *settings)
{
    gchar *xfconf_property;
    gulong handler;

    xfconf_property = g_strconcat(property_base, "/", property_name, NULL);

    handler = xfconf_g_property_bind(
        channel, xfconf_property, property_type, settings, property_name);

    g_free(xfconf_property);

    return handler;
}

static void
ddcplugin_settings_dispose(GObject *object)
{
    DdcPluginSettings *settings = DDCPLUGIN_SETTINGS(object);

    if (settings->channel != NULL) {
        xfconf_g_property_unbind_all(settings->channel);
        settings->channel = NULL;
    }

    G_OBJECT_CLASS(ddcplugin_settings_parent_class)->dispose(object);
}

static void
ddcplugin_settings_init(DdcPluginSettings *settings)
{
    settings->enable_keybind_brightness = DEFAULT_ENABLE_KEYBIND_BRIGHTNESS;
    settings->enable_keybind_volume = DEFAULT_ENABLE_KEYBIND_VOLUME;
    settings->step_size_brightness = DEFAULT_STEP_SIZE_BRIGHTNESS;
    settings->step_size_volume = DEFAULT_STEP_SIZE_VOLUME;
    settings->channel = NULL;
}

DdcPluginSettings *
ddcplugin_settings_new(const gchar *property_base)
{
    DdcPluginSettings *settings;

    settings = g_object_new(DDCPLUGIN_TYPE_SETTINGS, NULL);

    if (!xfconf_init(NULL)) {
        g_warning("could not initialize xfconf, using default settings");
        goto exit;
    }

    settings->channel = xfconf_channel_get("xfce4-ddc-plugin");

    ddcplugin_settings_property_bind(
        property_base,
        settings->channel,
        ENABLE_KEYBIND_BRIGHTNESS,
        G_TYPE_BOOLEAN,
        settings);

    ddcplugin_settings_property_bind(
        property_base,
        settings->channel,
        ENABLE_KEYBIND_VOLUME,
        G_TYPE_BOOLEAN,
        settings);

    ddcplugin_settings_property_bind(
        property_base,
        settings->channel,
        STEP_SIZE_BRIGHTNESS,
        G_TYPE_INT,
        settings);

    ddcplugin_settings_property_bind(
        property_base,
        settings->channel,
        STEP_SIZE_VOLUME,
        G_TYPE_INT,
        settings);

exit:
    return settings;
}

static void
ddcplugin_settings_class_init(DdcPluginSettingsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = ddcplugin_settings_dispose;
    object_class->set_property = ddcplugin_settings_set_property;
    object_class->get_property = ddcplugin_settings_get_property;

    ddcplugin_settings_properties[PROP_ENABLE_KEYBIND_BRIGHTNESS] = g_param_spec_boolean(
        ENABLE_KEYBIND_BRIGHTNESS,
        NULL,
        NULL,
        DEFAULT_ENABLE_KEYBIND_BRIGHTNESS,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    ddcplugin_settings_properties[PROP_ENABLE_KEYBIND_VOLUME] = g_param_spec_boolean(
        ENABLE_KEYBIND_VOLUME,
        NULL,
        NULL,
        DEFAULT_ENABLE_KEYBIND_BRIGHTNESS,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    ddcplugin_settings_properties[PROP_STEP_SIZE_BRIGHTNESS] = g_param_spec_int(
        STEP_SIZE_BRIGHTNESS,
        NULL,
        NULL,
        1,
        100,
        DEFAULT_STEP_SIZE_BRIGHTNESS,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    ddcplugin_settings_properties[PROP_STEP_SIZE_VOLUME] = g_param_spec_int(
        STEP_SIZE_VOLUME,
        NULL,
        NULL,
        1,
        100,
        DEFAULT_STEP_SIZE_VOLUME,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        ddcplugin_settings_properties);
}
