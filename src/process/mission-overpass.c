#include "process.h"
#include "psd-driving.h"

bool check_overpass(fnRun_t *fnRun);
void run_overpass(fnClean_t *fnClean);
void clean_overpass();
int  callback();

void init_overpass(fnCheck_t *fnCheck) {
    // settings for slope detection
    recog->is_on_slope.enabled = true;
    command(CMD_CAMERA_X_SERVO_CONTROL, 1500);
    command(CMD_CAMERA_Y_SERVO_CONTROL, 1725);

    MSG("UPCOMING MISSION => overpass & slope");
    *fnCheck = check_overpass;
}

void clean_overpass() { recog->is_on_slope.enabled = false; }

bool check_overpass(fnRun_t *fnRun) {
    static float pL, pR;
    bool         ret = false;

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

#define GAIN_P             40.0f
#define VELO               100
#define VELO_SLOPE         40
#define PSD_EXIT_THRESHOLD PSD_DISTANCE_MAX - 1.f

void run_overpass(fnClean_t *fnClean) {
    // drive with psd sensors and block until callback function returns true
    psd_driving(ctrl, recog, VELO, GAIN_P, callback);

    *fnClean = clean_overpass;
}

int callback() {
    static bool  init_sd = true;
    static bool  slope   = false;
    static float pL;
    static float pR;

    if (!slope) {
        // When you detect the slope at least once.
        slope = recog->is_on_slope.value;
    }

    if (slope) {
        if (init_sd) {
            command(CMD_DESIRE_SPEED, VELO_SLOPE);
            init_sd = false;
        }

        pL = recog->psd.value[PSD_LEFT_1];
        pR = recog->psd.value[PSD_RIGHT_1];
        if (pL > PSD_EXIT_THRESHOLD && pR > PSD_EXIT_THRESHOLD) {
            // init variables
            init_sd = true;
            slope   = false;

            return 1;
        }
    }

    return 0;
}
