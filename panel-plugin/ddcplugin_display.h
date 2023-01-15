#ifndef DDCPLUGIN_DISPLAY_H
#define DDCPLUGIN_DISPLAY_H

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <ddcutil_c_api.h>

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

void ddcplugin_display_modify_volume(DdcDisplay *display, int delta);

void ddcplugin_display_modify_brightness(DdcDisplay *display, int delta);

void ddcplugin_display_toggle_mute(DdcDisplay *display);

void ddcplugin_display_list_destroy(DdcDisplay *display_list);

int ddcplugin_display_list_create(DdcDisplay **out_display_list);

#endif // DDCPLUGIN_DISPLAY_H
