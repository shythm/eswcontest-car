#include "car-header.h"
#include "ctrlboard-lib.h"
#include "process.h"
#include <stdbool.h>

#define SPEED_VERTICAL 100    // 50~100
#define SPEED_PARALLEL 50     // 50~100
#define SLEEP_VERTICAL 100000 //
#define SLEEP_PARALLEL 300000 //

// void rr_save_and_recover(char);
void do_parking_vertical(State *);
void do_parking_parallel(State *);

typedef ctrlboard_byte_container container;

int parking_complete = 0;

void init_parking(State *state) { container data; }

/*
           ________________
          |               |
__________|    parking    |__________
|         |     area      |         |
|_________|               |_________|
          ^               ^
          |               |
          |               encoder_count1
          encoder_count2

<------- progress direction of car
*/

void check_parking(State *state) {
    // printf("psd: %3.1f\n", rr->psd.value[PSD_RIGHT_1]);
    static enum { NONE, READY, DECISION } parking_state = 0;

    static int encoder_count1 = 0;

    switch (parking_state) {
    case NONE: { // no obstacle sensed
        state->missions.parking.priority = 0;
        if (parking_complete >= 2) break;
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
            if (25 < distance && distance < 45) { // parking vertical
                set_desire_speed(0);
                state->missions.parking.priority = 10;
                state->missions.parking.function = do_parking_vertical;

            } else if (45 < distance && distance < 65) { // parking parallel
                // set_desire_speed( 0);
                state->missions.parking.priority = 10;
                state->missions.parking.function = do_parking_parallel;
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
    // rr_save_and_recover(0);
    short previous_steering = read_steering();

    // set steering at center
    set_steering(1500);
    usleep(SLEEP_VERTICAL);

    // regress : until psd_right_1 is near by end of the 2nd obstacle
    set_desire_speed(-SPEED_VERTICAL);
    while (state->input->psd.value[PSD_RIGHT_1] > 29.f) usleep(1000);
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

    // regress until the distance from the wall is 18cm
    set_encoder_counter(0);
    set_desire_speed(-SPEED_VERTICAL);
    while (state->input->psd.value[PSD_BACK] > 15.f) usleep(1000);
    set_desire_speed(0);
    int regressed_ticks = read_encoder_counter(mqid); // [tick]

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

    parking_complete++;
    // rr_save_and_recover(1);
    return;
}

void do_parking_parallel(State *state) {
    //_save_and_recover(0);
    // stop slowly
    const int braking_time  = 40000; // 40ms
    const int time_step     = 1000;  // 1ms
    short     initial_speed = read_desire_speed(mqid);
    float     tangent       = -(float)(initial_speed) / (float)(braking_time);

    for (int time = time_step; time <= braking_time; time += time_step) {
        set_desire_speed((tangent * time + initial_speed));
        usleep(time_step - 10);
    }
    set_desire_speed(0);

    short       previous_steering = read_steering(mqid);
    const float turn_radian       = PI / 3.f;
    const float straight_cm       = 15.f;

    usleep(SLEEP_PARALLEL);
    // set steering at center
    set_steering(1500);
    usleep(SLEEP_PARALLEL);

    // regress: until psd_right_1 is near by end of the 2nd obstacle
    set_desire_speed(-SPEED_PARALLEL);
    while (state->input->psd.value[PSD_RIGHT_1] < 26.f) usleep(1000);
    set_desire_speed(0);
    usleep(SLEEP_PARALLEL);

    // progress: move to proper position to park
    move(SPEED_PARALLEL, 35.f * TICK_PER_CM);

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
    set_encoder_counter(0);
    set_desire_speed(-SPEED_PARALLEL);
    while (state->input->psd.value[PSD_BACK] > 6.f) usleep(1000);
    set_desire_speed(0);
    int regressed_ticks = read_encoder_counter(mqid); // [tick]

    // beep
    beep(50);
    sleep(1);

    // turn forward as the car regressed
    move(SPEED_PARALLEL, -regressed_ticks);

    // steering to 1500
    set_steering(1500);
    usleep(SLEEP_PARALLEL);

    // progress: move to proper position
    move(SPEED_PARALLEL, straight_cm * TICK_PER_CM);

    // steering to 1000
    set_steering(1000);
    usleep(SLEEP_PARALLEL);

    // turn ??-degree forward
    move(SPEED_PARALLEL, (RADIUS * turn_radian * TICK_PER_CM));

    // set steering as previous steering before parking
    set_steering(previous_steering);
    usleep(SLEEP_PARALLEL);

    parking_complete++;
    // rr_save_and_recover(1);
    return;
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