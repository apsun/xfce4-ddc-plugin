#include "ddcplugin_display.h"
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include "ddcdisplay.h"

struct _DdcPluginDisplayClass {
    GObjectClass __parent__;
};

struct _DdcPluginDisplay {
    GObject __parent__;

    DdcDisplay *display;
    DdcPluginDisplay *next;
};

G_DEFINE_TYPE(DdcPluginDisplay, ddcplugin_display, G_TYPE_OBJECT);

enum {
    PROP_MODEL = 1,
    PROP_SERIAL,
    PROP_BRIGHTNESS,
    PROP_VOLUME,
    PROP_MUTED,
    N_PROPERTIES
};

static GParamSpec *ddcplugin_display_properties[N_PROPERTIES] = { NULL, };

static void
ddcplugin_display_dispose(GObject *object)
{
    DdcPluginDisplay *display = DDCPLUGIN_DISPLAY(object);

    if (display->next != NULL) {
        g_object_unref(display->next);
        display->next = NULL;
    }

    G_OBJECT_CLASS(ddcplugin_display_parent_class)->dispose(object);
}

static void
ddcplugin_display_set_property(
    GObject *object,
    guint prop_id,
    const GValue *value,
    GParamSpec *pspec)
{
    DdcPluginDisplay *display = DDCPLUGIN_DISPLAY(object);
    switch (prop_id) {
    case PROP_BRIGHTNESS:
        ddcdisplay_set_brightness(display->display, g_value_get_int(value));
        break;
    case PROP_VOLUME:
        ddcdisplay_set_volume(display->display, g_value_get_int(value));
        break;
    case PROP_MUTED:
        ddcdisplay_set_muted(display->display, g_value_get_boolean(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
ddcplugin_display_get_property(
    GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
    DdcPluginDisplay *display = DDCPLUGIN_DISPLAY(object);
    switch (prop_id) {
    case PROP_MODEL:
        g_value_set_string(value, ddcdisplay_get_model(display->display));
        break;
    case PROP_SERIAL:
        g_value_set_string(value, ddcdisplay_get_serial(display->display));
        break;
    case PROP_BRIGHTNESS:
        g_value_set_int(value, ddcdisplay_get_brightness(display->display));
        break;
    case PROP_VOLUME:
        g_value_set_int(value, ddcdisplay_get_volume(display->display));
        break;
    case PROP_MUTED:
        g_value_set_boolean(value, ddcdisplay_is_muted(display->display));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
ddcplugin_display_class_init(DdcPluginDisplayClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = ddcplugin_display_dispose;
    object_class->set_property = ddcplugin_display_set_property;
    object_class->get_property = ddcplugin_display_get_property;

    ddcplugin_display_properties[PROP_MODEL] = g_param_spec_string(
        MODEL,
        NULL,
        NULL,
        NULL,
        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    ddcplugin_display_properties[PROP_SERIAL] = g_param_spec_string(
        SERIAL,
        NULL,
        NULL,
        NULL,
        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    ddcplugin_display_properties[PROP_BRIGHTNESS] = g_param_spec_int(
        BRIGHTNESS,
        NULL,
        NULL,
        0,
        100,
        100,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    ddcplugin_display_properties[PROP_VOLUME] = g_param_spec_int(
        VOLUME,
        NULL,
        NULL,
        0,
        100,
        100,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    ddcplugin_display_properties[PROP_MUTED] = g_param_spec_boolean(
        MUTED,
        NULL,
        NULL,
        false,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        ddcplugin_display_properties);
}

static void
ddcplugin_display_init(DdcPluginDisplay *display)
{
    display->display = NULL;
    display->next = NULL;
}

DdcPluginDisplay *
ddcplugin_display_new(DdcDisplay *raw_display_list)
{
    DdcPluginDisplay *head = NULL;
    DdcPluginDisplay **phead = &head;
    DdcDisplay *raw_display = raw_display_list;

    while (raw_display != NULL) {
        *phead = g_object_new(ddcplugin_display_get_type(), NULL);
        (*phead)->display = raw_display;
        phead = &(*phead)->next;
        raw_display = raw_display->next;
    }

    return head;
}

DdcPluginDisplay *
ddcplugin_display_get_next(DdcPluginDisplay *display)
{
    return display->next;
}

static void
ddcplugin_display_modify_int_clamped(
    DdcPluginDisplay *display,
    const gchar *prop_name,
    gint delta)
{
    gint value;
    g_object_get(display, prop_name, &value, NULL);
    value += delta;
    if (value < 0) {
        value = 0;
    } else if (value > 100) {
        value = 100;
    }
    g_object_set(display, prop_name, value, NULL);
}

void
ddcplugin_display_modify_brightness(DdcPluginDisplay *display, gint delta)
{
    ddcplugin_display_modify_int_clamped(display, BRIGHTNESS, delta);
}

void
ddcplugin_display_modify_volume(DdcPluginDisplay *display, gint delta)
{
    ddcplugin_display_modify_int_clamped(display, VOLUME, delta);
}

void
ddcplugin_display_toggle_muted(DdcPluginDisplay *display)
{
    gboolean value;
    g_object_get(display, MUTED, &value, NULL);
    value = !value;
    g_object_set(display, MUTED, value, NULL);
}
