#include "car-header.h"
#include "ctrlboard-lib.h"
#include "process.h"
#include <stdbool.h>

#define PARK_SPEED 50     // 50~100
#define PARK_SLEEP 500000 // 500ms = 0.5s

void rr_save_and_recover(char);
void do_parking_vertical(State *);
void do_parking_parallel(State *);

typedef ctrlboard_byte_container container;

mqid_ctrl mqid;

recog_result *rr;
int           parking_complete = 0;

void init_parking(State *state) {
    container data;
    int       ret = get_shm_recog_result(&rr, 0);
}

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
    return;
    // printf("psd: %3.1f\n", rr->psd.value[PSD_RIGHT_1]);

    static enum { NONE, READY, DECISION } parking_state = 0;

    static int encoder_count1 = 0;

    switch (parking_state) {
    case NONE: { // no obstacle sensed
        state->missions.parking.priority = 0;
        if (parking_complete >= 2) break;
        if (rr->psd.value[PSD_RIGHT_1] < 26.f) parking_state = READY;
        break;
    }
    case READY: { // first obstacle sensed
        if (rr->psd.value[PSD_RIGHT_1] > 29.f) {
            encoder_count1 = read_encoder_counter(mqid); // parking area start
            parking_state  = DECISION;
        }
        break;
    }
    case DECISION: {                             // parking area sensed
        if (rr->psd.value[PSD_RIGHT_1] < 26.f) { // parking area end
            int distance =
                (read_encoder_counter(mqid) - encoder_count1) / TICK_PER_CM;
            printf("@@@@distance : %d [cm]\n", distance);
            if (25 < distance && distance < 45) { // parking vertical
                set_desire_speed(mqid, 0);
                state->missions.parking.priority = 10;
                state->missions.parking.function = do_parking_vertical;

            } else if (45 < distance && distance < 65) { // parking parallel
                set_desire_speed(mqid, 0);
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
    rr_save_and_recover(0);
    int   desire_encoder    = 0; // [tick]
    short previous_steering = read_steering(mqid);

    // set steering at center
    set_steering(mqid, 1500);
    usleep(PARK_SLEEP);

    // regress : until psd_right_1 is near by end of the 2nd obstacle
    set_desire_speed(mqid, -PARK_SPEED);
    while (rr->psd.value[PSD_RIGHT_1] > 29.f) usleep(1000);
    set_desire_speed(mqid, 0);
    usleep(PARK_SLEEP);

    // progress : move to proper position to park
    move(mqid, PARK_SPEED, 25.f * TICK_PER_CM);

    // steering to 1000
    set_steering(mqid, 1000);
    usleep(PARK_SLEEP);

    // turn 90-degree backward
    move(mqid, -PARK_SPEED, -(RADIUS * PI / 2 * TICK_PER_CM));

    // steering to 1500
    set_steering(mqid, 1500);
    usleep(PARK_SLEEP);

    // regress until the distance from the wall is 18cm
    set_encoder_counter(mqid, 0);
    set_desire_speed(mqid, -PARK_SPEED);
    while (rr->psd.value[PSD_BACK] > 18.f) usleep(1000);
    set_desire_speed(mqid, 0);
    desire_encoder = read_encoder_counter(mqid);

    // beep
    beep(mqid, 50);
    sleep(1);

    // go straight as the car regressed
    move(mqid, PARK_SPEED, -desire_encoder);

    // steering to 1000
    set_steering(mqid, 1000);
    usleep(PARK_SLEEP);

    // turn 90-degree forward
    move(mqid, PARK_SPEED, (RADIUS * PI / 2 * TICK_PER_CM));

    // set steering as previous steering before parking
    set_steering(mqid, previous_steering);
    usleep(PARK_SLEEP);

    parking_complete++;
    rr_save_and_recover(1);
    return;
}

void do_parking_parallel(State *state) {
    rr_save_and_recover(0);
    int   desire_encoder    = 0; // [tick]
    short previous_steering = read_steering(mqid);
    float turn_radian       = PI / 3.f;
    float straight_cm       = 15.f;

    // set steering at center
    set_steering(mqid, 1500);
    usleep(PARK_SLEEP);

    // regress: until psd_right_1 is near by end of the 2nd obstacle
    set_desire_speed(mqid, -PARK_SPEED);
    while (rr->psd.value[PSD_RIGHT_1] < 26.f) usleep(1000);
    set_desire_speed(mqid, 0);
    usleep(PARK_SLEEP);

    // progress: move to proper position to park
    move(mqid, PARK_SPEED, 40.f * TICK_PER_CM);

    // steering to 1000
    set_steering(mqid, 1000);
    usleep(PARK_SLEEP);

    // turn 60-degree backward
    move(mqid, -PARK_SPEED, -(RADIUS * turn_radian * TICK_PER_CM));

    // steering to 1500
    set_steering(mqid, 1500);
    usleep(PARK_SLEEP);

    // regress: move to proper position
    move(mqid, -PARK_SPEED, -straight_cm * TICK_PER_CM);

    // steering to 2000
    set_steering(mqid, 2000);
    usleep(PARK_SLEEP);

    // turn until the distance from the wall is 5cm backward
    set_encoder_counter(mqid, 0);
    set_desire_speed(mqid, -PARK_SPEED);
    while (rr->psd.value[PSD_BACK] > 5.f) usleep(1000);
    set_desire_speed(mqid, 0);
    desire_encoder = read_encoder_counter(mqid);
    usleep(PARK_SLEEP);

    // beep
    beep(mqid, 50);
    sleep(1);

    // turn forward as the car regressed
    move(mqid, PARK_SPEED, -desire_encoder);

    // steering to 1500
    set_steering(mqid, previous_steering);
    usleep(PARK_SLEEP);

    // progress: move to proper position
    move(mqid, PARK_SPEED, straight_cm * TICK_PER_CM);

    // steering to 1000
    set_steering(mqid, 1000);
    usleep(PARK_SLEEP);

    // turn 45-degree forward
    move(mqid, PARK_SPEED, (RADIUS * turn_radian * TICK_PER_CM));

    // set steering as previous steering before parking
    set_steering(mqid, previous_steering);
    sleep(10);

    parking_complete++;
    rr_save_and_recover(1);
    return;
}

void rr_save_and_recover(char mode) {
    static bool so_enable, tl_enable;
    if (mode == 0) { // save rr enable data, and set enable false
        so_enable                 = rr->stop_obstacle.enabled;
        rr->stop_obstacle.enabled = false;

        tl_enable                 = rr->traffic_light.enabled;
        rr->traffic_light.enabled = false;
    } else if (mode == 1) { // recover enable data
        rr->stop_obstacle.enabled = so_enable;
        rr->traffic_light.enabled = tl_enable;
    }
}