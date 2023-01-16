#ifndef DDCDISPLAY_H
#define DDCDISPLAY_H

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <ddcutil_c_api.h>

typedef struct DdcDisplayValue {
    uint16_t current;
    uint16_t max;
} DdcDisplayValue;

typedef struct DdcDisplayState {
    DdcDisplayValue brightness;
    DdcDisplayValue volume;
    DdcDisplayValue muted;
} DdcDisplayState;

typedef struct DdcDisplay {
    struct DdcDisplay *next;

    DDCA_Display_Info info;
    DDCA_Display_Handle handle;

    pthread_t update_thread;
    pthread_cond_t update_cond;
    pthread_mutex_t state_mutex;
    DdcDisplayState desired_state;
    bool exit;
} DdcDisplay;

void ddcdisplay_list_destroy(DdcDisplay *display_list);
int ddcdisplay_list_create(DdcDisplay **out_display_list);

const char *ddcdisplay_get_model(DdcDisplay *display);
const char *ddcdisplay_get_serial(DdcDisplay *display);

int ddcdisplay_get_brightness(DdcDisplay *display);
int ddcdisplay_get_volume(DdcDisplay *display);
bool ddcdisplay_is_muted(DdcDisplay *display);

void ddcdisplay_set_brightness(DdcDisplay *display, int value);
void ddcdisplay_set_volume(DdcDisplay *display, int value);
void ddcdisplay_set_muted(DdcDisplay *display, bool value);

#endif // DDCDISPLAY_H
