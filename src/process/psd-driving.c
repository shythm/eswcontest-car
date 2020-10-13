#include "psd-driving.h"
#define TARGET_STEERING 1500
typedef ctrlboard_byte_container container;

void psd_driving(mqid_ctrl ctrl, recog_result *rr, short velo, float gain_p,
                 int (*callback)()) {
    container  data;
    psd_data_t fL, fR; // front Left and Right
    short      position;

    // Initialize
    if (command(CMD_POSITION_CONTROL_ON_OFF, 0)) printf("Init Fail 1 \n");
    if (command(CMD_SPEED_CONTROL_ON_OFF, 1)) printf("Init Fail 2 \n");
    if (command(CMD_SPEED_PID_PROPORTIONAL, 20)) printf("Init Fail 3 \n");
    if (command(CMD_SPEED_PID_INTEGRAL, 20)) printf("Init Fail 4 \n");
    if (command(CMD_SPEED_PID_DIFFERENTAL, 20)) printf("Init Fail 5 \n");

    data.c_int16 = velo;
    send_ctrlboard(ctrl, CMD_DESIRE_SPEED, 2, &data);

    for (;;) {
        fL = rr->psd.value[PSD_LEFT_1];
        fR = rr->psd.value[PSD_RIGHT_1];

        position     = gain_p * (fL - fR);
        data.c_int16 = TARGET_STEERING + (short)position;
        if (data.c_int16 > 2000) {
            data.c_int16 = 2000;
        } else if (data.c_int16 < 1000) {
            data.c_int16 = 1000;
        }
        send_ctrlboard(ctrl, CMD_STEERING_SERVO_CONTROL, 2, &data);

        usleep(10 * 1000);

        if (callback()) break;
    }
}