#include "ctrlboard-lib.h"
#include "process.h"

void init_drive(State *state) {
    ctrlboard_byte_container data;

    if (ctrl_msgq(CMD_CAMERA_Y_SERVO_CONTROL, 2, &data) != MSG_STATE_SUCCESS) {
        printf("fail 1");
    }

    data.c_uint8 = 0;
    if (ctrl_msgq(CMD_POSITION_CONTROL_ON_OFF, 1, &data) != MSG_STATE_SUCCESS) {
        printf("fail 2");
    }

    data.c_uint8 = 1;
    if (ctrl_msgq(CMD_SPEED_CONTROL_ON_OFF, 1, &data) != MSG_STATE_SUCCESS) {
        printf("fail 3");
    }

    data.c_uint8 = 20;
    if (ctrl_msgq(CMD_SPEED_PID_PROPORTIONAL, 1, &data) != MSG_STATE_SUCCESS) {
        printf("fail 4");
    }

    data.c_uint8 = 20;
    if (ctrl_msgq(CMD_SPEED_PID_INTEGRAL, 1, &data) != MSG_STATE_SUCCESS) {
        printf("fail 5");
    }

    data.c_uint8 = 20;
    if (ctrl_msgq(CMD_SPEED_PID_DIFFERENTAL, 1, &data) != MSG_STATE_SUCCESS) {
        printf("fail 6");
    }

    data.c_int16 = 0;
    if (ctrl_msgq(CMD_DESIRE_SPEED, 2, &data) != MSG_STATE_SUCCESS) {
        printf("fail 7");
    }

    printf("Initialize finished.\n");
}