#include "ctrlboard-lib.h"
#include "process.h"

typedef ctrlboard_byte_container container;

void do_drive(State *state);

void init_drive(State *state) {
    container data;

    data.c_int16 = 1700;
    if (ctrl_msgq(CMD_CAMERA_Y_SERVO_CONTROL, 2, &data) != MSG_STATE_SUCCESS)
        printf("fail 1\n");

    data.c_uint8 = 0;
    if (ctrl_msgq(CMD_POSITION_CONTROL_ON_OFF, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail 2\n");

    data.c_uint8 = 1;
    if (ctrl_msgq(CMD_SPEED_CONTROL_ON_OFF, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail 3\n");

    data.c_uint8 = 20;
    if (ctrl_msgq(CMD_SPEED_PID_PROPORTIONAL, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail 4\n");

    data.c_uint8 = 20;
    if (ctrl_msgq(CMD_SPEED_PID_INTEGRAL, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail 5\n");

    data.c_uint8 = 20;
    if (ctrl_msgq(CMD_SPEED_PID_DIFFERENTAL, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail 6\n");

    state->input->lane.enabled = true;

    printf("Initialize finished.\n");
}

void check_drive(State *state) {
    state->missions.drive.priority = 1;
    state->missions.drive.function = do_drive;
}

#define GAIN_IRR 0.5f
void do_drive(State *state) {
    int       steering_val = 1500;
    container data;

#define GAIN_P      15    // P gain of PID control
#define GAIN_I      0.00f // I gain of PID control
#define ANTI_WINDUP 500   // Anti windup of I error
#define MAX_VELO    200   // Maximum velocity
#define CURVE_DECEL 150   // The smaller this value, the more it slows down.

    int          pos    = state->input->lane.value.position;
    static float errSum = 0;
    errSum += pos * GAIN_I;

    // Anti-windup
    if (errSum > ANTI_WINDUP) errSum = ANTI_WINDUP;
    if (errSum < -ANTI_WINDUP) errSum = -ANTI_WINDUP;

    // PI-control with pos value and convert control value to steer value
    steering_val = 1500 + (short)(pos * GAIN_P + errSum);

    short velocity = (short)(MAX_VELO * CURVE_DECEL / (CURVE_DECEL + abs(pos)));

    // Limit steering range
    if (steering_val > 2000) steering_val = 2000;
    if (steering_val < 1000) steering_val = 1000;

    // Send steering value to hardware
    data.c_int16 = steering_val;
    ctrl_msgq(CMD_STEERING_SERVO_CONTROL, 2, &data);

    // Send velocity to hardware
    data.c_int16 = velocity;
    ctrl_msgq(CMD_DESIRE_SPEED, 2, &data);
}
