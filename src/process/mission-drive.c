#include "ctrlboard-lib.h"
#include "process.h"

typedef ctrlboard_byte_container container;

void do_drive(State *state);

void init_drive(State *state) {
    if (command(CMD_CAMERA_Y_SERVO_CONTROL, 1700) != MSG_STATE_SUCCESS)
        printf("fail 1\n");

    if (command(CMD_POSITION_CONTROL_ON_OFF, 0) != MSG_STATE_SUCCESS)
        printf("fail 2\n");

    if (command(CMD_SPEED_CONTROL_ON_OFF, 1) != MSG_STATE_SUCCESS)
        printf("fail 3\n");

    if (command(CMD_SPEED_PID_PROPORTIONAL, 20) != MSG_STATE_SUCCESS)
        printf("fail 4\n");

    if (command(CMD_SPEED_PID_INTEGRAL, 20) != MSG_STATE_SUCCESS)
        printf("fail 5\n");

    if (command(CMD_SPEED_PID_DIFFERENTAL, 20) != MSG_STATE_SUCCESS)
        printf("fail 6\n");

    state->input->lane.enabled = true;

    printf("Initialize for mission-drive has been finished.\n");

#if 0
#define TERM_DRIVE 1200
#define TERM_STEER 500

    command(CMD_STEERING_SERVO_CONTROL, 1000);
    for (int i = 0; i < TERM_STEER; i++) usleep(1000);

    command(CMD_DESIRE_SPEED, -200);
    for (int i = 0; i < TERM_DRIVE; i++) usleep(1000);

    command(CMD_DESIRE_SPEED, 0);
    for (int i = 0; i < TERM_STEER; i++) usleep(1000);

    command(CMD_STEERING_SERVO_CONTROL, 2000);
    for (int i = 0; i < TERM_STEER; i++) usleep(1000);

    command(CMD_DESIRE_SPEED, 200);
    for (int i = 0; i < TERM_DRIVE; i++) { usleep(1000); }

    command(CMD_DESIRE_SPEED, 0);
    for (int i = 0; i < TERM_STEER; i++) usleep(1000);

    command(CMD_STEERING_SERVO_CONTROL, 1500);
    for (int i = 0; i < TERM_STEER; i++) usleep(1000);

    command(CMD_DESIRE_SPEED, -200);
    for (int i = 0; i < TERM_DRIVE; i++) { usleep(1000); }

    command(CMD_DESIRE_SPEED, 0);

    for (;;) { usleep(100000); }
#endif
}

void check_drive(State *state) {
    state->missions.drive.priority = 1;
    state->missions.drive.function = do_drive;
}

#define GAIN_IRR 0.5f
void do_drive(State *state) {
    int steering_val = 1500;

#define GAIN_P      14.4  // P gain of PID control
#define GAIN_I      0.00f // I gain of PID control
#define ANTI_WINDUP 500   // Anti windup of I error
#define MAX_VELO    40    // Maximum velocity
#define CURVE_DECEL 150   // The smaller this value, the more it slows down.

    int          pos    = state->input->lane.value.pos_yawl;
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
    command(CMD_STEERING_SERVO_CONTROL, steering_val);

    // Send velocity to hardware
    command(CMD_DESIRE_SPEED, velocity);
}
