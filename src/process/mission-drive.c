#include "process.h"

void init_drive(State *state) {
    ctrlboard_byte_container container;

    if (message_ctrlboard(msgq_id, MSGQ_ID, CMD_CAMERA_Y_SERVO_CONTROL,
                          CMD_TYPE_WRITE, 2, &container) != MSG_STATE_SUCCESS) {
        printf("fail 1");
    }

    container.c_uint8 = 0;
    if (message_ctrlboard(msgq_id, MSGQ_ID, CMD_POSITION_CONTROL_ON_OFF,
                          CMD_TYPE_WRITE, 1, &container) != MSG_STATE_SUCCESS) {
        printf("fail 2");
    }

    container.c_uint8 = 1;
    if (message_ctrlboard(msgq_id, MSGQ_ID, CMD_SPEED_CONTROL_ON_OFF,
                          CMD_TYPE_WRITE, 1, &container) != MSG_STATE_SUCCESS) {
        printf("fail 3");
    }

    container.c_uint8 = 20;
    if (message_ctrlboard(msgq_id, MSGQ_ID, CMD_SPEED_PID_PROPORTIONAL,
                          CMD_TYPE_WRITE, 1, &container) != MSG_STATE_SUCCESS) {
        printf("fail 4");
    }

    container.c_uint8 = 20;
    if (message_ctrlboard(msgq_id, MSGQ_ID, CMD_SPEED_PID_INTEGRAL,
                          CMD_TYPE_WRITE, 1, &container) != MSG_STATE_SUCCESS) {
        printf("fail 5");
    }

    container.c_uint8 = 20;
    if (message_ctrlboard(msgq_id, MSGQ_ID, CMD_SPEED_PID_DIFFERENTAL,
                          CMD_TYPE_WRITE, 1, &container) != MSG_STATE_SUCCESS) {
        printf("fail 6");
    }

    container.c_int16 = 150;
    if (message_ctrlboard(msgq_id, MSGQ_ID, CMD_DESIRE_SPEED, CMD_TYPE_WRITE, 2,
                          &container) != MSG_STATE_SUCCESS) {
        printf("fail 7");
    }
}