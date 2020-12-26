#include "process.h"

#define MAX_VELO 200 // Maximum velocity

void init_drive() {
    ctrld_write(CMD_CAMERA_Y_SERVO_CONTROL, 1700);
    ctrld_write(CMD_POSITION_CONTROL_ON_OFF, 0);
    ctrld_write(CMD_SPEED_CONTROL_ON_OFF, 1);
    ctrld_write(CMD_SPEED_PID_PROPORTIONAL, 20);
    ctrld_write(CMD_SPEED_PID_INTEGRAL, 20);
    ctrld_write(CMD_SPEED_PID_DIFFERENTAL, 20);

    recog->lane.enabled = true;
    // recog->ext_data.call_init_lane_info = true;
    target_velo = MAX_VELO;

    MSG("Initialize drive mission!");

#if 0
#define TERM_DRIVE 1200
#define TERM_STEER 500

    ctrld_write(CMD_STEERING_SERVO_CONTROL, 1000);
    for (int i = 0; i < TERM_STEER; i++) usleep(1000);

    ctrld_write(CMD_DESIRE_SPEED, -200);
    for (int i = 0; i < TERM_DRIVE; i++) usleep(1000);

    ctrld_write(CMD_DESIRE_SPEED, 0);
    for (int i = 0; i < TERM_STEER; i++) usleep(1000);

    ctrld_write(CMD_STEERING_SERVO_CONTROL, 2000);
    for (int i = 0; i < TERM_STEER; i++) usleep(1000);

    ctrld_write(CMD_DESIRE_SPEED, 200);
    for (int i = 0; i < TERM_DRIVE; i++) { usleep(1000); }

    ctrld_write(CMD_DESIRE_SPEED, 0);
    for (int i = 0; i < TERM_STEER; i++) usleep(1000);

    ctrld_write(CMD_STEERING_SERVO_CONTROL, 1500);
    for (int i = 0; i < TERM_STEER; i++) usleep(1000);

    ctrld_write(CMD_DESIRE_SPEED, -200);
    for (int i = 0; i < TERM_DRIVE; i++) { usleep(1000); }

    ctrld_write(CMD_DESIRE_SPEED, 0);

    for (;;) { usleep(100000); }
#endif
}

#define GAIN_P      15.0f // P gain of PID control
#define GAIN_I      0.00f // I gain of PID control
#define ANTI_WINDUP 500   // Anti windup of I error
#define CURVE_DECEL 150   // The smaller this value, the more it slows down.

void do_drive() {
    int steering_val = 1500;

    float        pos    = recog->lane.value.pos_yl;
    static float errSum = 0;
    errSum += pos * GAIN_I;

    // Anti-windup
    if (errSum > ANTI_WINDUP) errSum = ANTI_WINDUP;
    if (errSum < -ANTI_WINDUP) errSum = -ANTI_WINDUP;

    // PI-control with pos value and convert control value to steer value
    steering_val = 1500 + (short)(pos * GAIN_P + errSum);

    short velocity =
        (short)(target_velo * CURVE_DECEL / (CURVE_DECEL + abs(pos)));

    // Limit steering range
    if (steering_val > 2000) steering_val = 2000;
    if (steering_val < 1000) steering_val = 1000;

    // Send steering value to hardware
    ctrld_write(CMD_STEERING_SERVO_CONTROL, steering_val);

    // Send velocity to hardware
    ctrld_write(CMD_DESIRE_SPEED, velocity);
}
