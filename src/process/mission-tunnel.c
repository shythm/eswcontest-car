#include "car-header.h"
#include "process.h"

// 100~140 in hard map || 160~170 in easy map
#define TUNN_SPEED 180
// 35 ~ 50 : 속력이 높을 수록 높은 게인을 주어야 한다.
#define TUNN_GAIN 45
// 안전거리가 짧을수록 둔감하지만 운전에 영향을 덜 준다.
#define BACK_PSD_SAFE_DIST 6.f
// 20 ~ 40 : 속력이 높을 수록 작은 값을 주어야 한다.(원심력때문에)
#define BACK_PSD_SAFE_STEER 15

bool check_tunnel(fnRun_t *fnRun);
void do_tunnel(fnClean_t *fnClean);

void init_tunnel(fnCheck_t *fnCheck) {
    MSG("UPCOMING MISSION => tunnel");
    *fnCheck = check_tunnel;
}

bool check_tunnel(fnRun_t *fnRun) {
    static psd_data_t fL, fR;

    fL = recog->psd.value[PSD_LEFT_1];
    fR = recog->psd.value[PSD_RIGHT_1];

    if (fL < 20.f && fR < 20.f) {
        MSG("START MISSION => tunnel");
        *fnRun = do_tunnel;
        return true;
    }

    return false;
}

void do_tunnel(fnClean_t *fnClean) {
    psd_data_t fL, fR, bL, bR;
    short      position       = 0;
    int        lane_init_flag = 0;
    int        num            = 0;

    set_desire_speed(TUNN_SPEED);
    ctrld_write(CMD_FRONT_A_REAL_LIGHT_CONTROL, 1);
    while (1) {
        fL = recog->psd.value[PSD_LEFT_1];
        fR = recog->psd.value[PSD_RIGHT_1];
        bL = recog->psd.value[PSD_LEFT_2];
        bR = recog->psd.value[PSD_RIGHT_2];

        if (fL > 29.f && fR > 29.f) {
            // out of tunnel
            break;
        }

        // Basic Alogrithm: linear control about postion
        position = TUNN_GAIN * (fL - fR) + 1500;

        // constrain 1000~2000
        position = CONSTRAIN(position, 1000, 2000);

        // Additionary Algorithm: prevent invading safe distance
        // 로봇 뒤쪽이 터널 벽에 닿는 것을 예방
        if (bL < BACK_PSD_SAFE_DIST) position -= BACK_PSD_SAFE_STEER;
        if (bR < BACK_PSD_SAFE_DIST) position += BACK_PSD_SAFE_STEER;

        set_steering(position);

        if (1450 < position && position < 1550) {
            if (lane_init_flag == 1) {
                recog->ext_data.call_init_lane_info = true;
                num++;
            }
            lane_init_flag++;
        } else {
            lane_init_flag = 0;
        }
    }

    printf("lane init num: %d\n", num);

    ctrld_write(CMD_FRONT_A_REAL_LIGHT_CONTROL, 0);
}