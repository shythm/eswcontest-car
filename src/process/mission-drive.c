#include "ctrlboard-lib.h"
#include "process.h"

typedef ctrlboard_byte_container container;

void do_drive(State *state);

#define GAIN_IRR 0.5f

void init_drive(State *state) {
    container data;

    data.c_int16 = 0;
    if (ctrl_msgq(CMD_CAMERA_Y_SERVO_CONTROL, 2, &data) != MSG_STATE_SUCCESS)
        printf("fail 1");

    data.c_uint8 = 0;
    if (ctrl_msgq(CMD_POSITION_CONTROL_ON_OFF, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail 2");

    data.c_uint8 = 1;
    if (ctrl_msgq(CMD_SPEED_CONTROL_ON_OFF, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail 3");

    data.c_uint8 = 20;
    if (ctrl_msgq(CMD_SPEED_PID_PROPORTIONAL, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail 4");

    data.c_uint8 = 20;
    if (ctrl_msgq(CMD_SPEED_PID_INTEGRAL, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail 5");

    data.c_uint8 = 20;
    if (ctrl_msgq(CMD_SPEED_PID_DIFFERENTAL, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail 6");

    data.c_int16 = 0;
    if (ctrl_msgq(CMD_DESIRE_SPEED, 2, &data) != MSG_STATE_SUCCESS)
        printf("fail 7");

    printf("Initialize finished.\n");
}

void check_drive(State *state) {
    state->missions.drive.priority = 1;
    state->missions.drive.function = do_drive;
}

void do_drive(State *state) {
    int          steering_val = 1500;
    static float pos          = 0;
    container    data;

    // Update position value with
    pos = pos * (GAIN_IRR) + state->input->lane.value.position * (1 - GAIN_IRR);

    // P-control with pos value
    steering_val = 1500 + (short)(pos);

    // Limit steering range
    if (steering_val > 2000) steering_val = 2000;
    if (steering_val < 1000) steering_val = 1000;

    // Send steering value to hardware
    data.c_int16 = steering_val;
    ctrl_msgq(CMD_STEERING_SERVO_CONTROL, 2, &data);

    // Sleep for 20ms
    usleep(20 * 1000);
}
