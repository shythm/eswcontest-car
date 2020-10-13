#include "car-header.h"
#include "process.h"
#include <stdio.h>

#define TERM       500
#define TERM_SERVO 250
#define TERM_TURN  900

void wait() {
    for (int i = 0; i < TERM; i++) { usleep(1000); }
}

void waitT(int ms) {
    for (int i = 0; i < ms; i++) { usleep(1000); }
}

void do_overtaking(State *state) {
    command(CMD_DESIRE_SPEED, 0);              // Stop
    command(CMD_STEERING_SERVO_CONTROL, 1500); // Align steer
    waitT(TERM_SERVO);                         // Wait...
    command(CMD_DESIRE_SPEED, -50);            // Go back
    while (state->input->psd.value[PSD_FRONT] < 29) { waitT(10); }

    command(CMD_DESIRE_SPEED, 0); // Stop
    waitT(TERM_SERVO);
    if (state->input->is_there_car.data.left) {
        command(CMD_STEERING_SERVO_CONTROL, 2000);
        waitT(TERM_SERVO);
        command(CMD_DESIRE_SPEED, 100);
        waitT(TERM_TURN);
        command(CMD_STEERING_SERVO_CONTROL, 1200);
        waitT(TERM_SERVO);
        command(CMD_DESIRE_SPEED, 100);
        waitT(TERM_TURN * 3.3);
        command(CMD_STEERING_SERVO_CONTROL, 2000);
        waitT(TERM_SERVO * 3);
    } else if (state->input->is_there_car.data.right) {
        command(CMD_STEERING_SERVO_CONTROL, 1000);
        waitT(TERM_SERVO);
        command(CMD_DESIRE_SPEED, 100);
        waitT(TERM_TURN);
        command(CMD_STEERING_SERVO_CONTROL, 1800);
        waitT(TERM_SERVO);
        command(CMD_DESIRE_SPEED, 100);
        waitT(TERM_TURN * 3.6);
        command(CMD_STEERING_SERVO_CONTROL, 1000);
        waitT(TERM_SERVO * 4.3);
    } else {
        command(CMD_DESIRE_SPEED, 100);
        waitT(TERM_TURN);
        waitT(TERM_TURN);
    }
    command(CMD_DESIRE_SPEED, 0);

#if 1

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

#endif

    state->input->lane.initialize = true;
    while (state->input->lane.initialize) waitT(100);
}

void init_overtaking(State *state) {
    state->input->is_there_car.enabled  = true;
    state->missions.overtaking.function = do_overtaking;
}

void check_overtaking(State *state) {
    static int has_detected = 0;
    if (!has_detected) {
        if (state->input->psd.value[PSD_FRONT] < 20.0f) {
            // has_detected                        = 1;
            state->missions.overtaking.priority = 99;
        }
    }
}
