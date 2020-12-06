#include "car-header.h"
#include "ctrlboard-lib.h"
#include "process.h"
#include <stdbool.h>

#define SPEED_VERTICAL 85     // 50~100
#define SPEED_PARALLEL 50     // 50~100
#define SLEEP_VERTICAL 300000 //
#define SLEEP_PARALLEL 300000 //
#define BRAKING_TIME   100000
#define TIME_STEP      1000

// void rr_save_and_recover(char);
void do_parking_vertical(State *);
void do_parking_parallel(State *);
void stop_slowly();

void init_parking(State *state) {}

/*
           ________________
           |               |
 __________|    parking    |__________
 |         |     area      |         |
 |_________|               |_________|
^          ^               ^
|          |               |
|          |               encoder_count1
|          encoder_count2
(progress point)

<------- progress direction of car
*/

void check_parking(State *state) {
    // printf("psd: %3.1f\n", state->input->psd.value[PSD_RIGHT_1]);
    static enum { NONE, READY, DECISION } parking_state = 0;
    static int encoder_count1                           = 0;
    static int parking_complete                         = 0;

    if (parking_complete >= 2) return;
    switch (parking_state) {
    case NONE: { // no obstacle sensed
        state->missions.parking.priority = 0;
        if (state->input->psd.value[PSD_RIGHT_1] < 26.f) parking_state = READY;
        break;
    }
    case READY: { // first obstacle sensed
        if (state->input->psd.value[PSD_RIGHT_1] > 29.f) {
            encoder_count1 = read_encoder_counter(); // parking area start
            parking_state  = DECISION;
        }
        break;
    }
    case DECISION: { // parking area sensed
        if (state->input->psd.value[PSD_RIGHT_1] < 26.f) { // parking area end
            int distance =
                (float)(read_encoder_counter() - encoder_count1) / TICK_PER_CM;
            printf("@@@@distance : %d [cm]\n", distance);

            if (25 < distance && distance < 40) { // parking vertical
                state->missions.parking.priority = 10;
                state->missions.parking.function = do_parking_vertical;
                parking_complete++;

            } else if (45 < distance && distance < 60) { // parking parallel
                state->missions.parking.priority = 10;
                state->missions.parking.function = do_parking_parallel;
                parking_complete++;
            }
            parking_state = NONE;
        }
        break;
    }
    default:
        state->missions.parking.priority = 0;
        parking_state                    = NONE;
        break;
    }
}

void do_parking_vertical(State *state) {
    short previous_steering = read_steering();
    stop_slowly();
    // set_desire_speed(0);

    // set steering at center
    set_steering(1500);
    usleep(SLEEP_VERTICAL);

    if (state->input->psd.value[PSD_RIGHT_1] < 26.0f) {
        // progress: until psd_right_1 is near by progress point
        set_desire_speed(SPEED_VERTICAL / 2);
        while (state->input->psd.value[PSD_RIGHT_1] < 29.f) {}
    } else {
        // regress: until psd_right_1 is near by progress point
        set_desire_speed(-SPEED_VERTICAL / 2);
        while (state->input->psd.value[PSD_RIGHT_1] > 29.f) {}
    }

    set_desire_speed(0);
    usleep(SLEEP_VERTICAL);

    // progress : move to proper position to park
    move(SPEED_VERTICAL, 20.f * TICK_PER_CM);

    // steering to 1000
    set_steering(1000);
    usleep(SLEEP_VERTICAL);

    // turn 90-degree backward
    move(-SPEED_VERTICAL, -(RADIUS * PI / 2 * TICK_PER_CM));

    // steering to 1500
    set_steering(1500);
    usleep(SLEEP_VERTICAL);

    // regress until the distance from the wall is 15cm
    set_encoder_counter(0);
    set_desire_speed(-SPEED_VERTICAL);
    while (state->input->psd.value[PSD_BACK] > 15.f)
        ;
    set_desire_speed(0);
    int regressed_ticks = read_encoder_counter(); // [tick]

    // beep
    beep(50);
    sleep(1);

    // go straight as the car regressed
    move(SPEED_VERTICAL, -regressed_ticks);

    // steering to 1000
    set_steering(1000);
    usleep(SLEEP_VERTICAL);

    // turn 90-degree forward
    move(SPEED_VERTICAL, (RADIUS * PI / 2 * TICK_PER_CM));

    // recover steering as previous steering before parking
    set_steering(previous_steering);
    usleep(SLEEP_VERTICAL);

    while (1) sleep(1);
    // rr_save_and_recover(1);
    return;
}

void do_parking_parallel(State *state) {
    //_save_and_recover(0);
    // stop slowly
    stop_slowly();
    short       previous_steering = read_steering();
    const float turn_radian       = PI / 3.0f; // 60-degree
    const float straight_cm       = 25.f;

    usleep(SLEEP_PARALLEL);
    // set steering at center
    set_steering(1500);
    usleep(SLEEP_PARALLEL);

    if (state->input->psd.value[PSD_RIGHT_1] < 26.0f) {
        // progress: until psd_right_1 is near by progress point
        set_desire_speed(SPEED_PARALLEL);
        while (state->input->psd.value[PSD_RIGHT_1] < 29.f) {}
        set_desire_speed(0);
        usleep(SLEEP_PARALLEL);
        // progress: move to proper position to park
        move(SPEED_PARALLEL, 8.f * TICK_PER_CM);
    } else {
        // regress: until psd_right_1 is near by progress point
        set_desire_speed(-SPEED_PARALLEL);
        while (state->input->psd.value[PSD_RIGHT_1] > 29.f) {}
        set_desire_speed(0);
        usleep(SLEEP_PARALLEL);
        // progress: move to proper position to park
        move(SPEED_PARALLEL, 8.f * TICK_PER_CM);
    }

    // steering to 1000
    set_steering(1000);
    usleep(SLEEP_PARALLEL);

    // turn ??-degree backward
    move(-SPEED_PARALLEL, -(RADIUS * turn_radian * TICK_PER_CM));

    // steering to 1500
    set_steering(1500);
    usleep(SLEEP_PARALLEL);

    // regress: move to proper position
    move(-SPEED_PARALLEL, -straight_cm * TICK_PER_CM);

    // steering to 2000
    set_steering(2000);
    usleep(SLEEP_PARALLEL);

    // turn until the distance from the wall is 5cm backward
    set_steering(2000);
    set_encoder_counter(0);
    set_desire_speed(-SPEED_PARALLEL);
    while (state->input->psd.value[PSD_BACK] > 7.3f &&
           state->input->psd.value[PSD_RIGHT_2] > 6.f)
        ;
    set_desire_speed(0);
    int regressed_ticks = read_encoder_counter(); // [tick]

    // beep
    beep(50);
    sleep(1);

    // turn forward as the car regressed
    set_steering(2000);
    move(SPEED_PARALLEL, -regressed_ticks);

    // steering to 1500
    set_steering(1500);
    usleep(SLEEP_PARALLEL);

    // progress: move to proper position
    move(SPEED_PARALLEL, (straight_cm + 5.f) * TICK_PER_CM);

    // steering to 1000
    set_steering(1000);
    usleep(SLEEP_PARALLEL);

    // turn ??-degree forward
    move(SPEED_PARALLEL, (RADIUS * turn_radian * TICK_PER_CM));

    // set steering as previous steering before parking
    set_steering(previous_steering);
    usleep(SLEEP_PARALLEL);

    // rr_save_and_recover(1);
    while (1) sleep(1);
    return;
}
void stop_slowly() {
    short initial_speed = read_desire_speed();
    float tangent       = -(float)(initial_speed) / (float)(BRAKING_TIME);
    if (initial_speed < 10) {
        set_desire_speed(0);
        return;
    }
    for (int time = TIME_STEP; time <= BRAKING_TIME; time += TIME_STEP) {
        set_desire_speed((tangent * time + initial_speed));
    }
    set_desire_speed(0);
}

// void rr_save_and_recover(char mode) {
//     static bool so_enable, tl_enable;
//     if (mode == 0) { // save rr enable data, and set enable false
//         so_enable                 = state->input->stop_obstacle.enabled;
//         state->input->stop_obstacle.enabled = false;

//         tl_enable                 = state->input->traffic_light.enabled;
//         state->input->traffic_light.enabled = false;
//     } else if (mode == 1) { // recover enable data
//         state->input->stop_obstacle.enabled = so_enable;
//         state->input->traffic_light.enabled = tl_enable;
//     }
// }