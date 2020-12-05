#include "ctrlboard-lib.h"
#include "process.h"
#include "psd-driving.h"

typedef ctrlboard_byte_container container;

void   do_overpass(State *state);
int    callback();
State *st; // for callback function

void init_overpass(State *state) {
    // state->input->is_on_overpass.enabled = true;
    state->missions.overpass.function = do_overpass;

    st = state; // for callback function
}

void check_overpass(State *state) {
    return;

    static float pL, pR;
    bool         ret = false;

    if (state->input->psd.valid) {
        pL = state->input->psd.value[PSD_LEFT_1];
        pR = state->input->psd.value[PSD_RIGHT_1];

        if (pL < 20.0f && pR < 20.0f) { ret = true; }
    }

    if (ret) {
        state->missions.overpass.priority = 0; // 일단 0으로
    }
}

#define GAIN_P     40.0f
#define VELO       120
#define VELO_SLOPE 40

void do_overpass(State *state) {
    // drive with psd sensors and block until callback function returns true
    psd_driving(state->ctrl, state->input, VELO, GAIN_P, callback);
}

#define PSD_EXIT_THRESHOLD PSD_DISTANCE_MAX - 1.f
int callback() {
    static bool  init    = true;
    static bool  init_sd = true;
    static bool  slope   = false;
    static float pL;
    static float pR;
    int          ret = 0;

    if (init) {
        st->input->is_on_slope.enabled = true;
        command(CMD_CAMERA_X_SERVO_CONTROL, 1500);
        command(CMD_CAMERA_Y_SERVO_CONTROL, 1725);
        init = false;
    }
    if (!slope) {
        // When you detect the slope at least once.
        slope = st->input->is_on_slope.value;
    }

    if (slope) {
        if (init_sd) {
            command(CMD_DESIRE_SPEED, VELO_SLOPE);
            init_sd = false;
        }

        pL = st->input->psd.value[PSD_LEFT_1];
        pR = st->input->psd.value[PSD_RIGHT_1];
        if (pL > PSD_EXIT_THRESHOLD && pR > PSD_EXIT_THRESHOLD) {
            st->input->is_on_slope.enabled = false;

            // init variables
            init    = true;
            init_sd = true;
            slope   = false;
            // When both psds are not blocked
            ret = 1;
        }
    }

    return ret;
}
