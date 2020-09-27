#include "ctrlboard-lib.h"
#include "process.h"
typedef ctrlboard_byte_container container;
void                             do_overpass(State *state);

void init_overpass(State *state) {
    ctrlboard_byte_container container;
#if 1
    if (command(CMD_POSITION_CONTROL_ON_OFF, 0))
        printf("Overpass Init Fail 1 \n");

    if (command(CMD_SPEED_CONTROL_ON_OFF, 1)) printf("Overpass Init Fail 2 \n");

    if (command(CMD_SPEED_PID_PROPORTIONAL, 20))
        printf("Overpass Init Fail 3 \n");

    if (command(CMD_SPEED_PID_INTEGRAL, 20)) printf("Overpass Init Fail 4 \n");

    if (command(CMD_SPEED_PID_DIFFERENTAL, 20))
        printf("Overpass Init Fail 4 \n");
#endif
}

void check_overpass(State *state) {
    state->missions.overpass.priority = 2;
    state->missions.overpass.function = do_overpass;
}

#define GAIN_P          80.0f
#define TARGET_STEERING 1500
void do_overpass(State *state) {
    container  data;
    psd_data_t fL, fR; // front Left and Right
    short      position;

    data.c_int16 = 100;
    send_ctrlboard(state->ctrl, CMD_DESIRE_SPEED, 2, &data);

    for (;;) {
        fL = state->input->psd.value[PSD_LEFT_1];
        fR = state->input->psd.value[PSD_RIGHT_1];

        position     = GAIN_P * (fL - fR);
        data.c_int16 = TARGET_STEERING + (short)position;
        if (data.c_int16 > 2000) {
            data.c_int16 = 2000;
        } else if (data.c_int16 < 1000) {
            data.c_int16 = 1000;
        }
        send_ctrlboard(state->ctrl, CMD_STEERING_SERVO_CONTROL, 2, &data);

        usleep(10 * 1000);
        // printf("%f %f \n", fL, fR);
    }
}