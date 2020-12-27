#include "car-header.h"
#include "process.h"

#define SPEED_VERTICAL 85     // 50~100
#define SPEED_PARALLEL 50     // 50
#define SLEEP_VERTICAL 300000 // 0.3s
#define SLEEP_PARALLEL 300000 // 0.3s

#define BRAKING_TIME 110000
#define TIME_STEP    1000

bool check_parking_vertical(fnRun_t *);
bool check_parking_parallel(fnRun_t *);
void do_parking_vertical(fnClean_t *);
void do_parking_parallel(fnClean_t *);
void stop_slowly();

void init_parking_vertical(fnCheck_t *fnCheck) {
    // wait until psd is valid
    while (!recog->psd.valid) {}

    MSG("UPCOMING MISSION => parking_veritcal");
    *fnCheck = check_parking_vertical;
}
void init_parking_parallel(fnCheck_t *fnCheck) {
    // wait until psd is valid
    while (!recog->psd.valid) {}

    MSG("UPCOMING MISSION => parking_parallel");
    *fnCheck = check_parking_parallel;
}

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

bool check_parking_vertical(fnRun_t *fnRun) {
    // printf("psd: %3.1f\n", recog->psd.value[PSD_RIGHT_1]);
    static enum { NONE, READY, DECISION } parking_state = 0;
    static int encoder_count1                           = 0;
    switch (parking_state) {
    case NONE: { // no obstacle sensed
        if (recog->psd.value[PSD_RIGHT_1] < 28.f) parking_state = READY;
        break;
    }
    case READY: { // first obstacle sensed
        if (recog->psd.value[PSD_RIGHT_1] > 29.f) {
            encoder_count1 = read_encoder_counter(); // parking area start
            parking_state  = DECISION;
        }
        break;
    }
    case DECISION: {                                // parking area sensed
        if (recog->psd.value[PSD_RIGHT_1] < 28.f) { // parking area end
            int distance =
                (float)(read_encoder_counter() - encoder_count1) / TICK_PER_CM;
            printf("@@@@distance : %d [cm]\n", distance);
            if (25 < distance && distance < 40) {
                MSG("START MISSION => parking vertical");
                *fnRun        = do_parking_vertical;
                parking_state = NONE;
                return true;
            } else {
                parking_state = NONE;
            }
        }
        break;
    }
    default:
        parking_state = NONE;
        break;
    }
    return false;
}

bool check_parking_parallel(fnRun_t *fnRun) {
    // printf("psd: %3.1f\n", recog->psd.value[PSD_RIGHT_1]);
    static enum { NONE, READY, DECISION } parking_state = 0;
    static int encoder_count1                           = 0;

    switch (parking_state) {
    case NONE: { // no obstacle sensed
        if (recog->psd.value[PSD_RIGHT_1] < 28.f) parking_state = READY;
        break;
    }
    case READY: { // first obstacle sensed
        if (recog->psd.value[PSD_RIGHT_1] > 29.f) {
            encoder_count1 = read_encoder_counter(); // parking area start
            parking_state  = DECISION;
        }
        break;
    }
    case DECISION: {                                // parking area sensed
        if (recog->psd.value[PSD_RIGHT_1] < 28.f) { // parking area end
            int distance =
                (float)(read_encoder_counter() - encoder_count1) / TICK_PER_CM;
            printf("@@@@distance : %d [cm]\n", distance);
            if (45 < distance && distance < 60) {
                MSG("START MISSION => parking parallel");
                *fnRun        = do_parking_parallel;
                parking_state = NONE;
                return true;
            } else {
                parking_state = NONE;
            }
        }
        break;
    }
    default:
        parking_state = NONE;
        break;
    }
    return false;
}

void do_parking_vertical(fnClean_t *fnClean) {
    // stop slowly
    stop_slowly();

    // remember previous steering
    short previous_steering = read_steering();

    // set steering at center
    set_steering(1500);
    usleep(SLEEP_VERTICAL);

    if (recog->psd.value[PSD_RIGHT_1] < 29.0f) {
        // progress: until psd_right_1 is near by progress point
        set_desire_speed(SPEED_VERTICAL / 2);
        while (recog->psd.value[PSD_RIGHT_1] < 29.f) {}
    } else {
        // regress: until psd_right_1 is near by progress point
        set_desire_speed(-SPEED_VERTICAL / 2);
        while (recog->psd.value[PSD_RIGHT_1] > 29.f) {}
    }

    set_desire_speed(0);
    usleep(SLEEP_VERTICAL);

    // progress : move to proper position to park
    move(SPEED_VERTICAL, 16.f * TICK_PER_CM);

    // steering to 1000
    set_steering(1000);
    usleep(SLEEP_VERTICAL);

    // turn 90-degree backward
    move(-SPEED_VERTICAL, -(RADIUS * PI / 2 * TICK_PER_CM));

    // steering to 1500
    set_steering(1500);
    usleep(SLEEP_VERTICAL);

    // regress until the distance from the wall is 15cm
    // set_encoder_counter(0);

    int regressed_ticks = read_encoder_counter(); // [tick]
    set_desire_speed(-SPEED_VERTICAL);
    while (recog->psd.value[PSD_BACK] > 12.f) {}
    set_desire_speed(0);
    regressed_ticks -= read_encoder_counter(); // [tick]

    // beep
    beep(50);
    usleep(SLEEP_VERTICAL);

    // go straight as the car regressed
    move(SPEED_VERTICAL, regressed_ticks);

    // steering to 1000
    set_steering(1000);
    usleep(SLEEP_VERTICAL);

    // turn 90-degree forward
    move(SPEED_VERTICAL, (RADIUS * PI / 2 * TICK_PER_CM));

    // recover steering as previous steering before parking
    set_steering(previous_steering);
    usleep(SLEEP_VERTICAL);

    recog->ext_data.call_init_lane_info = true;
    return;
}

void do_parking_parallel(fnClean_t *fnClean) {
    // stop slowly
    stop_slowly();
    usleep(SLEEP_PARALLEL);

    // remember previous steering
    short       previous_steering = read_steering();
    const float turn_radian       = PI / 3.0f; // 60-degree
    const float straight_cm       = 25.f;

    // set steering at center
    set_steering(1500);
    usleep(SLEEP_PARALLEL);

    if (recog->psd.value[PSD_RIGHT_1] < 29.0f) {
        // progress: until psd_right_1 is near by progress point
        set_desire_speed(SPEED_PARALLEL);
        while (recog->psd.value[PSD_RIGHT_1] < 29.f) {}
        set_desire_speed(0);
        usleep(SLEEP_PARALLEL);
    } else {
        // regress: until psd_right_1 is near by progress point
        set_desire_speed(-SPEED_PARALLEL);
        while (recog->psd.value[PSD_RIGHT_1] > 29.f) {}
        set_desire_speed(0);
        usleep(SLEEP_PARALLEL);
    }

    // progress: move to proper position to park
    move(SPEED_PARALLEL, 5.f * TICK_PER_CM);

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
    // set_encoder_counter(0);

    int regressed_ticks = read_encoder_counter(); // [tick]
    set_desire_speed(-SPEED_PARALLEL);
    while (recog->psd.value[PSD_BACK] > 7.3f &&
           recog->psd.value[PSD_RIGHT_2] > 6.f) {}
    set_desire_speed(0);
    regressed_ticks -= read_encoder_counter(); // [tick]

    // beep
    beep(50);
    usleep(SLEEP_PARALLEL);

    // turn forward as the car regressed
    set_steering(2000);
    move(SPEED_PARALLEL, regressed_ticks);

    // steering to 1500
    set_steering(1500);
    usleep(SLEEP_PARALLEL);

    // progress: move to proper position
    move(SPEED_PARALLEL, (straight_cm + 4.0f) * TICK_PER_CM);

    // steering to 1000
    set_steering(1000);
    usleep(SLEEP_PARALLEL);

    // turn ??-degree forward
    move(SPEED_PARALLEL, (RADIUS * turn_radian * TICK_PER_CM));

    // set steering as previous steering before parking
    set_steering(previous_steering);
    usleep(SLEEP_PARALLEL);

    recog->ext_data.call_init_lane_info = true;
    return;
}

void stop_slowly() {
    short initial_speed = read_desire_speed();
    float tangent       = -(float)(initial_speed) / (float)(BRAKING_TIME);
    if (initial_speed < 10) {
        set_desire_speed(0);
        return;
    }
    for (int time = TIME_STEP; time < BRAKING_TIME; time += TIME_STEP) {
        set_desire_speed((tangent * time + initial_speed));
    }
    set_desire_speed(0);
}