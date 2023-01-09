#include <config.h>
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/libxfce4panel.h>
#include <keybinder.h>
#include <ddcutil_c_api.h>

#define ENABLE_KEYBIND_BRIGHTNESS 0
#define ENABLE_KEYBIND_VOLUME 1

#define STEP_BRIGHTNESS 5
#define STEP_VOLUME 5

#define VCP_FEATURE_CODE_BRIGHTNESS 0x10
#define VCP_FEATURE_CODE_VOLUME 0x62
#define VCP_FEATURE_CODE_MUTED 0x8D

typedef struct DdcValue {
    uint16_t current;
    uint16_t max;
} DdcValue;

typedef struct DdcState {
    DdcValue brightness;
    DdcValue volume;
    DdcValue muted;
} DdcState;

typedef struct DdcDisplay {
    struct DdcDisplay *next;

    DDCA_Display_Info info;
    DDCA_Display_Handle handle;

    pthread_t update_thread;
    pthread_cond_t update_cond;
    pthread_mutex_t state_mutex;
    DdcState desired_state;
    bool exit;
} DdcDisplay;

typedef struct DdcPlugin {
    XfcePanelPlugin *plugin;
    GtkWidget *widget;
    DdcDisplay *displays;
} DdcPlugin;

static void __attribute__((format(printf, 1, 2)))
eprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

static DdcDisplay *
ddcplugin_pick_display(DdcPlugin *ddcplugin)
{
    DdcDisplay *display = ddcplugin->displays;
    if (display == NULL) {
        eprintf("no displays detected\n");
        return NULL;
    }

    if (display->next == NULL) {
        return display;
    }

    // TODO: handle multiple monitors
    eprintf("more than one display detected\n");
    return display;
}

static DDCA_Status
ddcplugin_raw_read_value(
    DDCA_Display_Handle handle,
    DDCA_Vcp_Feature_Code feature,
    DdcValue *out_value)
{
    DDCA_Status rc;
    DDCA_Non_Table_Vcp_Value valrec;

    rc = ddca_get_non_table_vcp_value(handle, feature, &valrec);
    if (rc == 0) {
        out_value->current = valrec.sh << 8 | valrec.sl;
        out_value->max = valrec.mh << 8 | valrec.ml;
    } else {
        out_value->current = -1;
        out_value->max = -1;
    }
    return rc;
}

static DDCA_Status
ddcplugin_raw_write_value(
    DDCA_Display_Handle handle,
    DDCA_Vcp_Feature_Code feature,
    uint16_t value)
{
    return ddca_set_non_table_vcp_value(
        handle,
        feature,
        value >> 8,
        value & 0xff);
}

static void
ddcplugin_value_modify(DdcValue *value, int delta)
{
    int new_value = value->current + delta;
    if (new_value < 0) {
        new_value = 0;
    } else if (new_value > value->max) {
        new_value = value->max;
    }
    value->current = new_value;
}

static bool
ddcplugin_state_eq(DdcState *a, DdcState *b)
{
    return
        a->brightness.current == b->brightness.current &&
        a->volume.current == b->volume.current &&
        a->muted.current == b->muted.current;
}

static DDCA_Status
ddcplugin_display_read_values(DdcDisplay *display)
{
    DDCA_Status rc;

    // Brightness
    rc = ddcplugin_raw_read_value(
        display->handle,
        VCP_FEATURE_CODE_BRIGHTNESS,
        &display->desired_state.brightness);
    if (rc != 0) {
        eprintf(
            "failed to read brightness of display %s: %s\n",
            display->info.sn,
            ddca_rc_desc(rc));
        return rc;
    }
    eprintf(
        "current brightness of display %s is %d/%d\n",
        display->info.sn,
        display->desired_state.brightness.current,
        display->desired_state.brightness.max);

    // Volume
    rc = ddcplugin_raw_read_value(
        display->handle,
        VCP_FEATURE_CODE_VOLUME,
        &display->desired_state.volume);
    if (rc != 0) {
        eprintf(
            "failed to read volume of display %s: %s\n",
            display->info.sn,
            ddca_rc_desc(rc));
        return rc;
    }
    eprintf(
        "current volume of display %s is %d/%d\n",
        display->info.sn,
        display->desired_state.volume.current,
        display->desired_state.volume.max);

    // Muted
    rc = ddcplugin_raw_read_value(
        display->handle,
        VCP_FEATURE_CODE_MUTED,
        &display->desired_state.muted);
    if (rc != 0) {
        eprintf(
            "failed to read mute status of display %s: %s\n",
            display->info.sn,
            ddca_rc_desc(rc));
        return rc;
    }
    eprintf(
        "current mute status of display %s is %d/%d\n",
        display->info.sn,
        display->desired_state.muted.current,
        display->desired_state.muted.max);

    return 0;
}

static DDCA_Status
ddcplugin_display_write_values(
    DdcDisplay *display,
    const DdcState *desired_state,
    DdcState *current_state)
{
    DDCA_Status rc;

    // Brightness
    if (desired_state->brightness.current != current_state->brightness.current) {
        rc = ddcplugin_raw_write_value(
            display->handle,
            VCP_FEATURE_CODE_BRIGHTNESS,
            desired_state->brightness.current);
        if (rc != 0) {
            eprintf(
                "failed to write brightness of display %s: %s\n",
                display->info.sn,
                ddca_rc_desc(rc));
            return rc;
        }
        eprintf(
            "set brightness of display %s to %d/%d\n",
            display->info.sn,
            desired_state->brightness.current,
            desired_state->brightness.max);
    }

    // Volume
    if (desired_state->volume.current != current_state->volume.current) {
        rc = ddcplugin_raw_write_value(
            display->handle,
            VCP_FEATURE_CODE_VOLUME,
            desired_state->volume.current);
        if (rc != 0) {
            eprintf(
                "failed to write volume of display %s: %s\n",
                display->info.sn,
                ddca_rc_desc(rc));
            return rc;
        }
        eprintf(
            "set volume of display %s to %d/%d\n",
            display->info.sn,
            desired_state->volume.current,
            desired_state->volume.max);
    }

    // Muted
    if (desired_state->muted.current != current_state->muted.current) {
        rc = ddcplugin_raw_write_value(
            display->handle,
            VCP_FEATURE_CODE_MUTED,
            desired_state->muted.current);
        if (rc != 0) {
            eprintf(
                "failed to write mute status of display %s: %s\n",
                display->info.sn,
                ddca_rc_desc(rc));
            return rc;
        }
        eprintf(
            "set mute status of display %s to %d/%d\n",
            display->info.sn,
            desired_state->muted.current,
            desired_state->muted.max);
    }

    *current_state = *desired_state;
    return 0;
}

static void *
ddcplugin_update_thread(void *arg)
{
    DDCA_Status rc;
    DdcDisplay *display = arg;
    DdcState desired_state = display->desired_state;
    DdcState current_state = desired_state;

    pthread_mutex_lock(&display->state_mutex);
    while (!display->exit) {
        while (ddcplugin_state_eq(&display->desired_state, &current_state)) {
            pthread_cond_wait(&display->update_cond, &display->state_mutex);
        }
        desired_state = display->desired_state;
        pthread_mutex_unlock(&display->state_mutex);

        rc = ddcplugin_display_write_values(display, &desired_state, &current_state);
        if (rc != 0) {
            eprintf(
                "failed to write desired state of display %s: %s\n",
                display->info.sn,
                ddca_rc_desc(rc));
        }

        pthread_mutex_lock(&display->state_mutex);
    }
    pthread_mutex_unlock(&display->state_mutex);
    return NULL;
}

static void __attribute__((unused))
ddcplugin_keybind_brightness_up(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    DdcDisplay *display = ddcplugin_pick_display(ddcplugin);
    if (display == NULL) {
        return;
    }

    pthread_mutex_lock(&display->state_mutex);
    ddcplugin_value_modify(&display->desired_state.brightness, STEP_BRIGHTNESS);
    pthread_mutex_unlock(&display->state_mutex);
    pthread_cond_signal(&display->update_cond);
}

static void __attribute__((unused))
ddcplugin_keybind_brightness_down(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    DdcDisplay *display = ddcplugin_pick_display(ddcplugin);
    if (display == NULL) {
        return;
    }

    pthread_mutex_lock(&display->state_mutex);
    ddcplugin_value_modify(&display->desired_state.brightness, -STEP_BRIGHTNESS);
    pthread_mutex_unlock(&display->state_mutex);
    pthread_cond_signal(&display->update_cond);
}

static void __attribute__((unused))
ddcplugin_keybind_volume_up(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    DdcDisplay *display = ddcplugin_pick_display(ddcplugin);
    if (display == NULL) {
        return;
    }

    pthread_mutex_lock(&display->state_mutex);
    ddcplugin_value_modify(&display->desired_state.volume, STEP_VOLUME);
    pthread_mutex_unlock(&display->state_mutex);
    pthread_cond_signal(&display->update_cond);
}

static void __attribute__((unused))
ddcplugin_keybind_volume_down(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    DdcDisplay *display = ddcplugin_pick_display(ddcplugin);
    if (display == NULL) {
        return;
    }

    pthread_mutex_lock(&display->state_mutex);
    ddcplugin_value_modify(&display->desired_state.volume, -STEP_VOLUME);
    pthread_mutex_unlock(&display->state_mutex);
    pthread_cond_signal(&display->update_cond);
}

static void __attribute__((unused))
ddcplugin_keybind_mute_toggle(const char *keystring, void *plugin)
{
    DdcPlugin *ddcplugin = plugin;
    DdcDisplay *display = ddcplugin_pick_display(ddcplugin);
    if (display == NULL) {
        return;
    }

    // TODO: why is muted == 1, unmuted == 2?
    pthread_mutex_lock(&display->state_mutex);
    display->desired_state.muted.current = display->desired_state.muted.current == 1 ? 2 : 1;
    pthread_mutex_unlock(&display->state_mutex);
    pthread_cond_signal(&display->update_cond);
}

static void
ddcplugin_release_displays(DdcPlugin *ddcplugin)
{
    DDCA_Status rc;
    DdcDisplay *display;

    while (ddcplugin->displays != NULL) {
        display = ddcplugin->displays;

        // Stop update thread
        if (display->update_thread != 0) {
            pthread_mutex_lock(&display->state_mutex);
            display->exit = true;
            pthread_mutex_unlock(&display->state_mutex);
            pthread_cond_signal(&display->update_cond);
            pthread_join(display->update_thread, NULL);
        }

        // Release display handle
        if (display->handle != NULL) {
            rc = ddca_close_display(display->handle);
            if (rc != 0) {
                eprintf(
                    "failed to close display %s: %s\n",
                    display->info.sn,
                    ddca_rc_desc(rc));
            }
        }

        ddcplugin->displays = display->next;
        free(display);
    }
}

static void
ddcplugin_acquire_displays(DdcPlugin *ddcplugin)
{
    DDCA_Status rc;
    DDCA_Display_Info_List *info_list;

    // Get list of available displays
    rc = ddca_get_display_info_list2(false, &info_list);
    if (rc != 0) {
        eprintf("failed to get display list: %s\n", ddca_rc_desc(rc));
        goto error;
    }

    for (int i = 0; i < info_list->ct; ++i) {
        int err;
        DdcDisplay *display;
        DDCA_Display_Info *info = &info_list->info[i];
        eprintf("detected display %s (model: %s)\n", info->sn, info->model_name);

        // Create the display object
        display = malloc(sizeof(*display));
        if (display == NULL) {
            eprintf("failed to allocate memory for display %s\n", info->sn);
            goto error;
        }
        display->next = ddcplugin->displays;
        ddcplugin->displays = display;
        display->info = *info;
        display->handle = NULL;
        display->update_thread = 0;
        pthread_cond_init(&display->update_cond, NULL);
        pthread_mutex_init(&display->state_mutex, NULL);
        display->desired_state.brightness.current = -1;
        display->desired_state.brightness.max = -1;
        display->desired_state.volume.current = -1;
        display->desired_state.volume.max = -1;
        display->desired_state.muted.current = -1;
        display->desired_state.muted.max = -1;
        display->exit = false;

        // Acquire display handle
        rc = ddca_open_display2(display->info.dref, true, &display->handle);
        if (rc != 0) {
            eprintf("failed to open display %s: %s\n", info->sn, ddca_rc_desc(rc));
            goto error;
        }

        // Read current display state
        rc = ddcplugin_display_read_values(display);
        if (rc != 0) {
            eprintf(
                "failed to load current values from display %s: %s\n",
                info->sn,
                ddca_rc_desc(rc));
            goto error;
        }

        // Create update thread
        err = pthread_create(
            &display->update_thread,
            NULL,
            ddcplugin_update_thread,
            display);
        if (err != 0) {
            eprintf("failed to create update thread: %s\n", strerror(err));
            goto error;
        }
    }

    return;

error:
    ddcplugin_release_displays(ddcplugin);
    return;
}

static void
ddcplugin_keybind_unregister(void)
{
#if ENABLE_KEYBIND_BRIGHTNESS
    keybinder_unbind("XF86MonBrightnessUp", ddcplugin_keybind_brightness_up);
    keybinder_unbind("XF86MonBrightnessDown", ddcplugin_keybind_brightness_down);
#endif
#if ENABLE_KEYBIND_VOLUME
    keybinder_unbind("XF86AudioRaiseVolume", ddcplugin_keybind_volume_up);
    keybinder_unbind("XF86AudioLowerVolume", ddcplugin_keybind_volume_down);
    keybinder_unbind("XF86AudioMute", ddcplugin_keybind_mute_toggle);
#endif
}

static void
ddcplugin_keybind_register(DdcPlugin *ddcplugin)
{
    keybinder_init();
    if (
#if ENABLE_KEYBIND_BRIGHTNESS
        !keybinder_bind("XF86MonBrightnessUp", ddcplugin_keybind_brightness_up, ddcplugin) ||
        !keybinder_bind("XF86MonBrightnessDown", ddcplugin_keybind_brightness_down, ddcplugin) ||
#endif
#if ENABLE_KEYBIND_VOLUME
        !keybinder_bind("XF86AudioRaiseVolume", ddcplugin_keybind_volume_up, ddcplugin) ||
        !keybinder_bind("XF86AudioLowerVolume", ddcplugin_keybind_volume_down, ddcplugin) ||
        !keybinder_bind("XF86AudioMute", ddcplugin_keybind_mute_toggle, ddcplugin) ||
#endif
        false)
    {
        eprintf("failed to bind keys - already in use?\n");
        ddcplugin_keybind_unregister();
        abort();
    }
}

static void
ddcplugin_free(XfcePanelPlugin *plugin, DdcPlugin *ddcplugin)
{
    // Unregister keybinds
    ddcplugin_keybind_unregister();

    // Release display resources
    ddcplugin_release_displays(ddcplugin);

    // Destroy panel icon
    gtk_widget_destroy(ddcplugin->widget);

    // Free the plugin object
    free(ddcplugin);

    eprintf("xfce4-ddc-plugin finalized\n");
}

static void
ddcplugin_new(XfcePanelPlugin *plugin)
{
    DdcPlugin *ddcplugin;

    // Initialize locale
    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    // Create the plugin object
    ddcplugin = malloc(sizeof(*ddcplugin));
    ddcplugin->plugin = plugin;
    ddcplugin->widget = NULL;
    ddcplugin->displays = NULL;
    g_signal_connect(G_OBJECT(plugin), "free-data", G_CALLBACK(ddcplugin_free), ddcplugin);

    // Create panel icon
    ddcplugin->widget = gtk_image_new_from_icon_name("video-display", GTK_ICON_SIZE_BUTTON);
    gtk_container_add(GTK_CONTAINER(plugin), ddcplugin->widget);
    gtk_widget_show_all(GTK_WIDGET(plugin));
    xfce_panel_plugin_add_action_widget(plugin, ddcplugin->widget);

    // Acquire display resources
    ddcplugin_acquire_displays(ddcplugin);

    // Register keybinds
    ddcplugin_keybind_register(ddcplugin);

    eprintf("xfce4-ddc-plugin initialized\n");
}

XFCE_PANEL_PLUGIN_REGISTER(ddcplugin_new);
