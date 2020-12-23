#include "process.h"

bool check_overpass(fnRun_t *fnRun);
void run_overpass(fnClean_t *fnClean);
void clean_overpass();
void steering(short pos);

void init_overpass(fnCheck_t *fnCheck) {
    // settings for slope detection
    recog->is_on_slope.enabled = true;

    // Initialize to drive with PSD
    ctrld_write(CMD_POSITION_CONTROL_ON_OFF, 0);
    ctrld_write(CMD_SPEED_CONTROL_ON_OFF, 1);
    ctrld_write(CMD_SPEED_PID_PROPORTIONAL, 20);
    ctrld_write(CMD_SPEED_PID_INTEGRAL, 20);
    ctrld_write(CMD_SPEED_PID_DIFFERENTAL, 20);

    MSG("UPCOMING MISSION => overpass & slope");
    *fnCheck = check_overpass;
}

void clean_overpass() {
    recog->is_on_slope.enabled = false;
    init_drive(); // Init drive mission
}

bool check_overpass(fnRun_t *fnRun) {
    static psd_data_t pL, pR;

    bool ret = false;

    if (recog->psd.valid) {
        pL = recog->psd.value[PSD_LEFT_1];
        pR = recog->psd.value[PSD_RIGHT_1];

        if (pL < 20.0f && pR < 20.0f) {
            MSG("START MISSION => overpass & slope");
            *fnRun = run_overpass;
            return true;
        }
    }

    return false;
}

// Constants for PSD driving
#define GAIN_P       48.0f
#define GAIN_P_SLOPE 96.0f
#define VELO         100
#define VELO_SLOPE   40

// MAX_PSD_VALUE_SLOPE = (lane width / 2) - (car width) = (40 / 2) - (18 / 2)
#define MAX_PSD_VALUE_SLOPE 11.0f
#define PSD_EXIT_THRESHOLD  PSD_DISTANCE_MAX - 1.f
#define TARGET_STEERING     1500

void run_overpass(fnClean_t *fnClean) {
    psd_data_t fL, fR; // front Left and Right

    ctrld_write(CMD_DESIRE_SPEED, VELO);

    for (;;) {
        if (recog->is_on_slope.value) { break; }

        fL = recog->psd.value[PSD_LEFT_1];
        fR = recog->psd.value[PSD_RIGHT_1];
        steering(GAIN_P * (fL - fR) + TARGET_STEERING);
    }

    ctrld_write(CMD_DESIRE_SPEED, VELO_SLOPE);

    for (;;) {
        fL = recog->psd.value[PSD_LEFT_1];
        fR = recog->psd.value[PSD_RIGHT_1];

        if (fL > PSD_EXIT_THRESHOLD && fR > PSD_EXIT_THRESHOLD) { break; }

        // constrain psd value in straight section
        if (fL > MAX_PSD_VALUE_SLOPE) { fL = MAX_PSD_VALUE_SLOPE; }
        if (fR > MAX_PSD_VALUE_SLOPE) { fR = MAX_PSD_VALUE_SLOPE; }
        steering(GAIN_P * (fL - fR) + TARGET_STEERING);
    }

    *fnClean = clean_overpass;
}

#define SERVO_US_DELAY 10 * 1000

void steering(short pos) {
    if (pos > 2000) {
        pos = 2000;
    } else if (pos < 1000) {
        pos = 1000;
    }

    ctrld_write(CMD_STEERING_SERVO_CONTROL, pos);
    usleep(SERVO_US_DELAY);
}