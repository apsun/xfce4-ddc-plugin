#include "ddcplugin_keybind.h"
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <keybinder.h>
#include <libxfce4ui/libxfce4ui.h>
#include "ddcplugin_display.h"
#include "ddcplugin_settings.h"

struct _DdcPluginKeybindClass {
    GObjectClass __parent__;
};

struct _DdcPluginKeybind {
    GObject __parent__;

    DdcPluginDisplay *display_list;
    DdcPluginSettings *settings;

    gulong signal_handler_brightness;
    gulong signal_handler_volume;
};

G_DEFINE_TYPE(DdcPluginKeybind, ddcplugin_keybind, G_TYPE_OBJECT);

static DdcPluginDisplay *
ddcplugin_keybind_pick_display(DdcPluginKeybind *keybind)
{
    DdcPluginDisplay *display = keybind->display_list;
    if (display == NULL) {
        g_warning("no displays detected");
        return NULL;
    }

    if (ddcplugin_display_get_next(display) == NULL) {
        return display;
    }

    // TODO: handle multiple monitors
    g_warning("more than one display detected");
    return display;
}

static void
ddcplugin_keybind_brightness_up(const char *keystring, void *data)
{
    DdcPluginKeybind *keybind = data;
    DdcPluginDisplay *display;
    gint step_size_brightness;

    display = ddcplugin_keybind_pick_display(keybind);
    if (display == NULL) {
        return;
    }

    g_object_get(keybind->settings, STEP_SIZE_BRIGHTNESS, &step_size_brightness, NULL);
    ddcplugin_display_modify_brightness(display, step_size_brightness);
}

static void
ddcplugin_keybind_brightness_down(const char *keystring, void *data)
{
    DdcPluginKeybind *keybind = data;
    DdcPluginDisplay *display;
    gint step_size_brightness;

    display = ddcplugin_keybind_pick_display(keybind);
    if (display == NULL) {
        return;
    }

    g_object_get(keybind->settings, STEP_SIZE_BRIGHTNESS, &step_size_brightness, NULL);
    ddcplugin_display_modify_brightness(display, -step_size_brightness);
}

static void
ddcplugin_keybind_volume_up(const char *keystring, void *data)
{
    DdcPluginKeybind *keybind = data;
    DdcPluginDisplay *display;
    gint step_size_volume;

    display = ddcplugin_keybind_pick_display(keybind);
    if (display == NULL) {
        return;
    }

    g_object_get(keybind->settings, STEP_SIZE_VOLUME, &step_size_volume, NULL);
    ddcplugin_display_modify_volume(display, step_size_volume);
}

static void
ddcplugin_keybind_volume_down(const char *keystring, void *data)
{
    DdcPluginKeybind *keybind = data;
    DdcPluginDisplay *display;
    gint step_size_volume;

    display = ddcplugin_keybind_pick_display(keybind);
    if (display == NULL) {
        return;
    }

    g_object_get(keybind->settings, STEP_SIZE_VOLUME, &step_size_volume, NULL);
    ddcplugin_display_modify_volume(display, -step_size_volume);
}

static void
ddcplugin_keybind_mute_toggle(const char *keystring, void *data)
{
    DdcPluginKeybind *keybind = data;
    DdcPluginDisplay *display;

    display = ddcplugin_keybind_pick_display(keybind);
    if (display == NULL) {
        return;
    }

    ddcplugin_display_toggle_muted(display);
}

static void
ddcplugin_keybind_unregister_brightness(void)
{
    keybinder_unbind("XF86MonBrightnessUp", ddcplugin_keybind_brightness_up);
    keybinder_unbind("XF86MonBrightnessDown", ddcplugin_keybind_brightness_down);
    g_info("unregistered brightness keybinds");
}

static void
ddcplugin_keybind_unregister_volume(void)
{
    keybinder_unbind("XF86AudioRaiseVolume", ddcplugin_keybind_volume_up);
    keybinder_unbind("XF86AudioLowerVolume", ddcplugin_keybind_volume_down);
    keybinder_unbind("XF86AudioMute", ddcplugin_keybind_mute_toggle);
    g_info("unregistered volume keybinds");
}

static int
ddcplugin_keybind_register_brightness(DdcPluginKeybind *keybind)
{
    int rc = 0;

    keybinder_init();
    if (!keybinder_bind("XF86MonBrightnessUp", ddcplugin_keybind_brightness_up, keybind) ||
        !keybinder_bind("XF86MonBrightnessDown", ddcplugin_keybind_brightness_down, keybind))
    {
        g_warning("failed to register brightness keybinds");
        rc = -EBUSY;
        goto error;
    }

    g_info("registered brightness keybinds");

exit:
    return rc;

error:
    ddcplugin_keybind_unregister_brightness();
    goto exit;
}

static int
ddcplugin_keybind_register_volume(DdcPluginKeybind *keybind)
{
    int rc = 0;

    keybinder_init();
    if (!keybinder_bind("XF86AudioRaiseVolume", ddcplugin_keybind_volume_up, keybind) ||
        !keybinder_bind("XF86AudioLowerVolume", ddcplugin_keybind_volume_down, keybind) ||
        !keybinder_bind("XF86AudioMute", ddcplugin_keybind_mute_toggle, keybind))
    {
        g_warning("failed to register volume keybinds");
        rc = -EBUSY;
        goto error;
    }

    g_info("registered volume keybinds");

exit:
    return rc;

error:
    ddcplugin_keybind_unregister_volume();
    goto exit;
}

static void
ddcplugin_keybind_update_brightness(DdcPluginKeybind *keybind)
{
    gboolean enable;
    g_object_get(keybind->settings, ENABLE_KEYBIND_BRIGHTNESS, &enable, NULL);

    ddcplugin_keybind_unregister_brightness();
    if (enable && ddcplugin_keybind_register_brightness(keybind) < 0) {
        xfce_dialog_show_warning(
            NULL,
            _("Failed to grab XF86MonBrightnessUp, XF86MonBrightnessDown keys.\n"
              "Is another program (e.g. xfce4-power-manager) currently using them?"),
            _("xfce4-ddc-plugin could not register brightness keys"));
    }
}

static void
ddcplugin_keybind_update_volume(DdcPluginKeybind *keybind)
{
    gboolean enable;
    g_object_get(keybind->settings, ENABLE_KEYBIND_VOLUME, &enable, NULL);

    ddcplugin_keybind_unregister_volume();
    if (enable && ddcplugin_keybind_register_volume(keybind) < 0) {
        xfce_dialog_show_warning(
            NULL,
            _("Failed to grab XF86AudioRaiseVolume, XF86AudioLowerVolume, "
              "XF86AudioMute keys.\nIs another program (e.g. xfce4-volumed, "
              "xfce4-pulseaudio-plugin) currently using them?"),
            _("xfce4-ddc-plugin could not register volume keys"));
    }
}

static void
ddcplugin_keybind_init(DdcPluginKeybind *keybind)
{
    keybind->display_list = NULL;
    keybind->settings = NULL;
    keybind->signal_handler_brightness = 0;
    keybind->signal_handler_volume = 0;
}

DdcPluginKeybind *
ddcplugin_keybind_new(
    DdcPluginDisplay *display_list,
    DdcPluginSettings *settings)
{
    DdcPluginKeybind *keybind;

    keybind = g_object_new(ddcplugin_keybind_get_type(), NULL);
    keybind->display_list = g_object_ref(display_list);
    keybind->settings = g_object_ref(settings);

    keybind->signal_handler_brightness = g_signal_connect_swapped(
        G_OBJECT(settings),
        "notify::" ENABLE_KEYBIND_BRIGHTNESS,
        G_CALLBACK(ddcplugin_keybind_update_brightness),
        keybind);

    keybind->signal_handler_volume = g_signal_connect_swapped(
        G_OBJECT(settings),
        "notify::" ENABLE_KEYBIND_VOLUME,
        G_CALLBACK(ddcplugin_keybind_update_volume),
        keybind);

    ddcplugin_keybind_update_brightness(keybind);
    ddcplugin_keybind_update_volume(keybind);

    return keybind;
}

static void
ddcplugin_keybind_dispose(GObject *object)
{
    DdcPluginKeybind *keybind = DDCPLUGIN_KEYBIND(object);

    ddcplugin_keybind_unregister_volume();
    ddcplugin_keybind_unregister_brightness();

    if (keybind->signal_handler_volume > 0) {
        g_signal_handler_disconnect(keybind->settings, keybind->signal_handler_volume);
        keybind->signal_handler_volume = 0;
    }

    if (keybind->signal_handler_brightness > 0) {
        g_signal_handler_disconnect(keybind->settings, keybind->signal_handler_brightness);
        keybind->signal_handler_brightness = 0;
    }

    if (keybind->settings != NULL) {
        g_object_unref(keybind->settings);
        keybind->settings = NULL;
    }

    if (keybind->display_list != NULL) {
        g_object_unref(keybind->display_list);
        keybind->display_list = NULL;
    }

    G_OBJECT_CLASS(ddcplugin_keybind_parent_class)->dispose(object);
}

static void
ddcplugin_keybind_class_init(DdcPluginKeybindClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = ddcplugin_keybind_dispose;
}
