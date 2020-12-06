#include "car-header.h"
#include "process.h"

#define TUNN_SPEED          140 // 100~140 in hard map (over 140 in easy map)
#define TUNN_GAIN           40
#define BACK_PSD_SAFE_DIST  6.f
#define BACK_PSD_SAFE_STEER 40

void do_tunnel(State *state);

static recog_psd_data *psd;

void init_tunnel(State *state) {
    psd                             = &(state->input->psd);
    state->missions.tunnel.function = do_tunnel;
}

void check_tunnel(State *state) {
    static int is_checked = false;

    if (is_checked == true) return;
    else {
        if (psd->value[PSD_LEFT_1] < 20.f && psd->value[PSD_RIGHT_1] < 20.f) {
            is_checked                      = true;
            state->missions.tunnel.priority = 10;
            beep(50);
        }
    }
}

void do_tunnel(State *state) {
    psd_data_t fL, fR;
    short      position = 0;
    set_desire_speed(TUNN_SPEED);
    while (1) {
        if (psd->value[PSD_LEFT_1] > 29.f && psd->value[PSD_RIGHT_1] > 29.f)
            break; // out of tunnel

        // Basic Alogrithm: linear control about postion
        fL       = psd->value[PSD_LEFT_1];
        fR       = psd->value[PSD_RIGHT_1];
        position = TUNN_GAIN * (fL - fR) + 1500;

        // constrain 1000~2000
        if (position > 2000) position = 2000;
        else if (position < 1000)
            position = 1000;

        // Additionary Algorithm: prevent invading safe distance
        // 로봇 뒤쪽이 터널 벽에 닿는 것을 예방
        if (psd->value[PSD_LEFT_2] < BACK_PSD_SAFE_DIST)
            position -= BACK_PSD_SAFE_STEER;
        if (psd->value[PSD_RIGHT_2] < BACK_PSD_SAFE_DIST)
            position += BACK_PSD_SAFE_STEER;

        // constrain 1000~2000
        if (position > 2000) position = 2000;
        else if (position < 1000)
            position = 1000;

        set_steering(position);
    }

    // stop after tunnel (for debugging)
    set_desire_speed(0);
    while (1) sleep(1);
}