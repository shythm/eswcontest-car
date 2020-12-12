#include "car-header.h"
#include "process.h"

// 100~140 in hard map || 160~170 in easy map
#define TUNN_SPEED 170
// 35 ~ 50 : 속력이 높을 수록 높은 게인을 주어야 한다.
#define TUNN_GAIN 45
// 안전거리가 짧을수록 둔감하지만 운전에 영향을 덜 준다.
#define BACK_PSD_SAFE_DIST 6.f
// 20 ~ 40 : 속력이 높을 수록 작은 값을 주어야 한다.(원심력때문에)
#define BACK_PSD_SAFE_STEER 20

bool check_tunnel(fnRun_t *fnRun);
void do_tunnel(fnClean_t *fnClean);

static recog_psd_data *psd;

void init_tunnel(fnCheck_t *fnCheck) {
    psd = &recog->psd;

    MSG("UPCOMING MISSION => tunnel");
    *fnCheck = check_tunnel;
}

bool check_tunnel(fnRun_t *fnRun) {
    if (psd->value[PSD_LEFT_1] < 20.f && psd->value[PSD_RIGHT_1] < 20.f) {
        // 전조등 켜는 함수 넣으면 좋을 듯
        beep(50);
        MSG("START MISSION => tunnel");
        *fnRun = do_tunnel;
        return true;
    }

    return false;
}

void do_tunnel(fnClean_t *fnClean) {
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
        // maintain safe distance
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
}