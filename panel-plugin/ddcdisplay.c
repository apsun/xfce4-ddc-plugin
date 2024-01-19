#include "ddcdisplay.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <glib.h>
#include <ddcutil_c_api.h>

#define VCP_FEATURE_CODE_BRIGHTNESS 0x10
#define VCP_FEATURE_CODE_VOLUME 0x62
#define VCP_FEATURE_CODE_MUTED 0x8D

static int
ddcdisplay_raw_read_value(
    DDCA_Display_Handle handle,
    DDCA_Vcp_Feature_Code feature,
    DdcDisplayValue *out_value)
{
    int rc = 0;
    DDCA_Non_Table_Vcp_Value valrec;

    rc = ddca_get_non_table_vcp_value(handle, feature, &valrec);
    if (rc >= 0) {
        out_value->current = valrec.sh << 8 | valrec.sl;
        out_value->max = valrec.mh << 8 | valrec.ml;
    } else {
        out_value->current = -1;
        out_value->max = -1;
    }
    return rc;
}

static int
ddcdisplay_raw_write_value(
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
ddcdisplay_value_modify(DdcDisplayValue *value, int new_value)
{
    if (new_value < 0) {
        new_value = 0;
    } else if (new_value > value->max) {
        new_value = value->max;
    }
    value->current = new_value;
}

static bool
ddcdisplay_state_eq(DdcDisplayState *a, DdcDisplayState *b)
{
    return
        a->brightness.current == b->brightness.current &&
        a->volume.current == b->volume.current &&
        a->muted.current == b->muted.current;
}

static int
ddcdisplay_read_values(DdcDisplay *display)
{
    int rc = 0;

    // Brightness
    rc = ddcdisplay_raw_read_value(
        display->handle,
        VCP_FEATURE_CODE_BRIGHTNESS,
        &display->desired_state.brightness);
    if (rc < 0) {
        g_warning(
            "failed to read brightness of display %s: %s",
            display->info.sn,
            ddca_rc_desc(rc));
        goto exit;
    }
    g_info(
        "current brightness of display %s is %d/%d",
        display->info.sn,
        display->desired_state.brightness.current,
        display->desired_state.brightness.max);

    // Volume
    rc = ddcdisplay_raw_read_value(
        display->handle,
        VCP_FEATURE_CODE_VOLUME,
        &display->desired_state.volume);
    if (rc < 0) {
        g_warning(
            "failed to read volume of display %s: %s",
            display->info.sn,
            ddca_rc_desc(rc));
        goto exit;
    }
    g_info(
        "current volume of display %s is %d/%d",
        display->info.sn,
        display->desired_state.volume.current,
        display->desired_state.volume.max);

    // Muted
    rc = ddcdisplay_raw_read_value(
        display->handle,
        VCP_FEATURE_CODE_MUTED,
        &display->desired_state.muted);
    if (rc < 0) {
        g_warning(
            "failed to read mute status of display %s: %s",
            display->info.sn,
            ddca_rc_desc(rc));
        goto exit;
    }
    g_info(
        "current mute status of display %s is %d/%d",
        display->info.sn,
        display->desired_state.muted.current,
        display->desired_state.muted.max);

exit:
    return rc;
}

static int
ddcdisplay_write_values(
    DdcDisplay *display,
    const DdcDisplayState *desired_state,
    DdcDisplayState *current_state)
{
    int rc = 0;

    // Brightness
    if (desired_state->brightness.current != current_state->brightness.current) {
        rc = ddcdisplay_raw_write_value(
            display->handle,
            VCP_FEATURE_CODE_BRIGHTNESS,
            desired_state->brightness.current);
        if (rc < 0) {
            g_warning(
                "failed to write brightness of display %s: %s",
                display->info.sn,
                ddca_rc_desc(rc));
            goto exit;
        }
        g_info(
            "set brightness of display %s to %d/%d",
            display->info.sn,
            desired_state->brightness.current,
            desired_state->brightness.max);
    }

    // Volume
    if (desired_state->volume.current != current_state->volume.current) {
        rc = ddcdisplay_raw_write_value(
            display->handle,
            VCP_FEATURE_CODE_VOLUME,
            desired_state->volume.current);
        if (rc < 0) {
            g_warning(
                "failed to write volume of display %s: %s",
                display->info.sn,
                ddca_rc_desc(rc));
            goto exit;
        }
        g_info(
            "set volume of display %s to %d/%d",
            display->info.sn,
            desired_state->volume.current,
            desired_state->volume.max);
    }

    // Muted
    if (desired_state->muted.current != current_state->muted.current) {
        rc = ddcdisplay_raw_write_value(
            display->handle,
            VCP_FEATURE_CODE_MUTED,
            desired_state->muted.current);
        if (rc < 0) {
            g_warning(
                "failed to write mute status of display %s: %s",
                display->info.sn,
                ddca_rc_desc(rc));
            goto exit;
        }
        g_info(
            "set mute status of display %s to %d/%d",
            display->info.sn,
            desired_state->muted.current,
            desired_state->muted.max);
    }

    *current_state = *desired_state;

exit:
    return rc;
}

static void *
ddcdisplay_update_thread(void *arg)
{
    int rc = 0;
    DdcDisplay *display = arg;
    DdcDisplayState desired_state = display->desired_state;
    DdcDisplayState current_state = desired_state;

    pthread_mutex_lock(&display->state_mutex);
    while (1) {
        // Wait for exit signal or state update
        while (!display->exit && ddcdisplay_state_eq(&display->desired_state, &current_state)) {
            pthread_cond_wait(&display->update_cond, &display->state_mutex);
        }

        if (display->exit) {
            break;
        }

        desired_state = display->desired_state;
        pthread_mutex_unlock(&display->state_mutex);

        // Write desired values to display
        rc = ddcdisplay_write_values(display, &desired_state, &current_state);
        if (rc < 0) {
            g_warning(
                "failed to write desired state of display %s: %s",
                display->info.sn,
                ddca_rc_desc(rc));
        }

        pthread_mutex_lock(&display->state_mutex);
    }
    pthread_mutex_unlock(&display->state_mutex);
    return NULL;
}

void
ddcdisplay_list_destroy(DdcDisplay *display_list)
{
    int rc = 0;
    DdcDisplay *display;

    while (display_list != NULL) {
        display = display_list;

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
            if (rc < 0) {
                g_warning(
                    "failed to close display %s: %s",
                    display->info.sn,
                    ddca_rc_desc(rc));
            }
        }

        display_list = display->next;
        g_free(display);
    }
}

int
ddcdisplay_list_create(DdcDisplay **out_display_list)
{
    int rc = 0;
    DdcDisplay *display;
    DDCA_Display_Info_List *info_list;
    DDCA_Display_Info *info;

    *out_display_list = NULL;

    // Initialize libddcutil
    rc = ddca_init2(NULL, DDCA_SYSLOG_DEBUG, DDCA_INIT_OPTIONS_NONE, NULL);
    if (rc < 0) {
        g_warning("failed to initialize libddcutil");
        goto error;
    }

    // Get list of available displays
    rc = ddca_get_display_info_list2(false, &info_list);
    if (rc < 0) {
        g_warning("failed to get display list: %s", ddca_rc_desc(rc));
        goto error;
    }

    if (info_list->ct == 0) {
        rc = -ENODEV;
        g_warning("no displays detected");
        goto error;
    }

    for (int i = 0; i < info_list->ct; ++i) {
        info = &info_list->info[i];
        g_info("detected display %s (model: %s)", info->sn, info->model_name);

        // Create the display object
        display = g_malloc(sizeof(*display));
        display->next = NULL;
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
        if (rc < 0) {
            g_warning("failed to open display %s: %s", info->sn, ddca_rc_desc(rc));
            ddcdisplay_list_destroy(display);
            continue;
        }

        // Read current display state
        rc = ddcdisplay_read_values(display);
        if (rc < 0) {
            g_warning(
                "failed to load current values from display %s: %s",
                info->sn,
                ddca_rc_desc(rc));
            ddcdisplay_list_destroy(display);
            continue;
        }

        // Display looks valid, insert it into the list
        display->next = *out_display_list;
        *out_display_list = display;

        // Create update thread
        rc = pthread_create(
            &display->update_thread,
            NULL,
            ddcdisplay_update_thread,
            display);
        if (rc != 0) {
            g_warning("failed to create update thread: %s", strerror(rc));
            rc = -rc;
            goto error;
        }
    }

exit:
    return rc;

error:
    ddcdisplay_list_destroy(*out_display_list);
    *out_display_list = NULL;
    goto exit;
}

const char *
ddcdisplay_get_model(DdcDisplay *display)
{
    return display->info.model_name;
}

const char *
ddcdisplay_get_serial(DdcDisplay *display)
{
    return display->info.sn;
}

int
ddcdisplay_get_brightness(DdcDisplay *display)
{
    int value;

    pthread_mutex_lock(&display->state_mutex);
    value = display->desired_state.brightness.current;
    pthread_mutex_unlock(&display->state_mutex);
    return value;
}

int
ddcdisplay_get_volume(DdcDisplay *display)
{
    int value;

    pthread_mutex_lock(&display->state_mutex);
    value = display->desired_state.volume.current;
    pthread_mutex_unlock(&display->state_mutex);
    return value;
}

bool
ddcdisplay_is_muted(DdcDisplay *display)
{
    bool value;

    // TODO: why is muted == 1, unmuted == 2?
    pthread_mutex_lock(&display->state_mutex);
    value = display->desired_state.muted.current == 1;
    pthread_mutex_unlock(&display->state_mutex);
    return value;
}

void
ddcdisplay_set_brightness(DdcDisplay *display, int value)
{
    pthread_mutex_lock(&display->state_mutex);
    ddcdisplay_value_modify(&display->desired_state.brightness, value);
    pthread_mutex_unlock(&display->state_mutex);
    pthread_cond_signal(&display->update_cond);
}

void
ddcdisplay_set_volume(DdcDisplay *display, int value)
{
    pthread_mutex_lock(&display->state_mutex);
    ddcdisplay_value_modify(&display->desired_state.volume, value);
    pthread_mutex_unlock(&display->state_mutex);
    pthread_cond_signal(&display->update_cond);
}

void
ddcdisplay_set_muted(DdcDisplay *display, bool value)
{
    // TODO: why is muted == 1, unmuted == 2?
    pthread_mutex_lock(&display->state_mutex);
    display->desired_state.muted.current = value ? 1 : 2;
    pthread_mutex_unlock(&display->state_mutex);
    pthread_cond_signal(&display->update_cond);
}
