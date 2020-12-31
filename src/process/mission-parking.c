#include "car-header.h"
#include "process.h"

#define SPEED_VERTICAL 85 // 50~100
#define SPEED_PARALLEL 80 // 50
#define SUPP_SPEED     10
#define BRAKING_TIME   130
#define TIME_STEP      1

#define OBSTACLE_DISTANCE 29.5F

#define LEFT  0
#define RIGHT 1
// Location of parking lot to road
#define LOC_VERT LEFT // parking vertical
#define LOC_PARA LEFT // parking parallel

#if LOC_VERT == RIGHT
const int psd_XX_1_vert    = PSD_RIGHT_1;
const int steer_direc_vert = 500;
#elif LOC_VERT == LEFT
const int psd_XX_1_vert    = PSD_LEFT_2;
int       steer_direc_vert = -500;
#endif

#if LOC_PARA == RIGHT
const int psd_XX_1_para    = PSD_RIGHT_1;
const int psd_XX_2_para    = PSD_RIGHT_2;
const int steer_direc_para = 500;
#elif LOC_PARA == LEFT
const int psd_XX_1_para    = PSD_LEFT_2;
const int psd_XX_2_para    = PSD_LEFT_2;
const int steer_direc_para = -500;
#endif

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
    static enum { NONE, READY, DECISION } parking_state = 0;
    static int encoder_count1                           = 0;

    switch (parking_state) {
    case NONE: { // no obstacle sensed
        if (recog->psd.value[psd_XX_1_vert] < OBSTACLE_DISTANCE)
            parking_state = READY;
        break;
    }
    case READY: { // first obstacle sensed
        if (recog->psd.value[psd_XX_1_vert] > OBSTACLE_DISTANCE) {
            encoder_count1 = read_encoder_counter(); // parking area start
            parking_state  = DECISION;
        }
        break;
    }
    case DECISION: { // parking area sensed
        if (recog->psd.value[psd_XX_1_vert] <
            OBSTACLE_DISTANCE) { // parking area end
            int distance =
                (float)(read_encoder_counter() - encoder_count1) / TICK_PER_CM;
            printf("@@@@distance : %d [cm]\n", distance);
            if (25 < distance && distance < 45) {
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
        if (recog->psd.value[psd_XX_1_para] < OBSTACLE_DISTANCE)
            parking_state = READY;
        break;
    }
    case READY: { // first obstacle sensed
        if (recog->psd.value[psd_XX_1_para] > OBSTACLE_DISTANCE) {
            encoder_count1 = read_encoder_counter(); // parking area start
            parking_state  = DECISION;
        }
        break;
    }
    case DECISION: { // parking area sensed
        if (recog->psd.value[psd_XX_1_para] <
            OBSTACLE_DISTANCE) { // parking area end
            int distance =
                (float)(read_encoder_counter() - encoder_count1) / TICK_PER_CM;
            printf("@@@@distance : %d [cm]\n", distance);
            if (45 < distance && distance < 65) {
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
    usleep(SLEEP_STOP);

    if (recog->psd.value[psd_XX_1_vert] < OBSTACLE_DISTANCE) {
        // progress: until psd is near by progress point
        set_desire_speed(SPEED_VERTICAL / 2);
        while (recog->psd.value[psd_XX_1_vert] < OBSTACLE_DISTANCE) {}
    } else {
        // regress: until psd is near by progress point
        set_desire_speed(-SPEED_VERTICAL / 2);
        while (recog->psd.value[psd_XX_1_vert] > OBSTACLE_DISTANCE) {}
    }

    set_desire_speed(0);
    usleep(SLEEP_STEER);

#if LOC_VERT == RIGHT // move to proper position to park
    move(SPEED_VERTICAL, 25.f * TICK_PER_CM);
#else
    move(-SPEED_VERTICAL, -2.5f * TICK_PER_CM);
#endif

    // Location of parking lot: right | left
    // turn right | left
    set_steering(1500 - steer_direc_vert);
    move(-SPEED_VERTICAL, -(RADIUS * PI / 2.f * TICK_PER_CM));

    // regress into vertical parking lot
    set_steering(1500);
    int regressed_ticks = read_encoder_counter(); // [tick]
    set_desire_speed(-SPEED_VERTICAL);
    while (recog->psd.value[PSD_BACK] > 10.f) {}
    set_desire_speed(0);
    regressed_ticks -= read_encoder_counter(); // [tick]

    // beep
    beep(50);
    sleep(1);

#if LOC_VERT == LEFT
    regressed_ticks -= 4.F * TICK_PER_CM;
#endif
    // go straight as the car regressed
    move(SPEED_VERTICAL, regressed_ticks - 4.f * TICK_PER_CM);

    // turn right | left
    set_steering(1500 - steer_direc_vert);
    move(SPEED_VERTICAL, (RADIUS * PI / 2 * TICK_PER_CM) + 4.f * TICK_PER_CM);

    // recover steering as previous steering before parking
    recog->ext_data.call_init_lane_info = true;
    set_steering(previous_steering);
    usleep(SLEEP_STEER);
    return;
}

void do_parking_parallel(fnClean_t *fnClean) {
    // stop slowly
    stop_slowly();
    usleep(SLEEP_STOP);

    // remember previous steering
    short       previous_steering = read_steering();
    const float turn_radian       = PI * 55.f / 180.f; // 60-degre
    const float straight_tick     = 33.f * TICK_PER_CM;

    // set steering at center
    set_steering(1500);
    usleep(SLEEP_STEER);

    if (recog->psd.value[psd_XX_1_para] < OBSTACLE_DISTANCE) {
        // progress: until psd_right_1 is near by progress point
        set_desire_speed(SPEED_PARALLEL);
        while (recog->psd.value[psd_XX_1_para] < OBSTACLE_DISTANCE) {}
    } else {
        // regress: until psd_right_1 is near by progress point
        set_desire_speed(-SPEED_PARALLEL);
        while (recog->psd.value[psd_XX_1_para] > OBSTACLE_DISTANCE) {}
    }
    set_desire_speed(0);
    usleep(SLEEP_STOP);

#if LOC_PARA == RIGHT
    // progress: move to proper position to park
    move(SPEED_PARALLEL, 13.f * TICK_PER_CM);
#else
    move(-SPEED_PARALLEL, -18.f * TICK_PER_CM);
#endif
    usleep(SLEEP_STOP);

    // Location of parking lot: right | left
    // turn right | left
    set_steering(1500 - steer_direc_para);
    move(-(SPEED_PARALLEL + SUPP_SPEED), -(RADIUS * turn_radian * TICK_PER_CM));

    // REGRESS straight
    set_steering(1500);
#if LOC_PARA == RIGHT

    move(-SPEED_PARALLEL, -straight_tick);
#else
    move(-SPEED_PARALLEL, -(straight_tick - 12.F * TICK_PER_CM));
#endif

    // turn left | right until the distance from the wall is x-cm backward
    set_steering(1500 + steer_direc_para);
    int regressed_ticks = read_encoder_counter(); // [tick]
    set_desire_speed(-SPEED_PARALLEL);
    while (recog->psd.value[PSD_BACK] > 7.3f &&
           recog->psd.value[psd_XX_2_para] > 6.f) {}
    set_desire_speed(0);
    regressed_ticks -= read_encoder_counter(); // [tick]

    // beep
    beep(50);
    sleep(1);

    // turn forward as the car regressed
    set_steering(1500 + steer_direc_para); // steering to left |right
    move((SPEED_PARALLEL + SUPP_SPEED), regressed_ticks);

    // progress: move to proper position
    set_steering(1500);
#if LOC_PARA == RIGHT
    move(SPEED_PARALLEL, straight_tick - 12.f * TICK_PER_CM);
#else
    move(SPEED_PARALLEL, straight_tick - 24.f * TICK_PER_CM);
#endif

    // turn left |right
    set_steering(1500 - steer_direc_para);
    move((SPEED_PARALLEL + SUPP_SPEED),
         (RADIUS * turn_radian * TICK_PER_CM) + 9.f * TICK_PER_CM);

    // set steering as previous steering before parking
    recog->ext_data.call_init_lane_info = true;
    set_steering(previous_steering);
    usleep(SLEEP_STEER);
    return;
}

void stop_slowly() {
    short initial_speed = read_desire_speed();
    float tangent       = -(float)(initial_speed) / (float)(BRAKING_TIME);
    for (int time = TIME_STEP; time < BRAKING_TIME; time += TIME_STEP) {
        set_desire_speed((tangent * time + initial_speed));
    }
    set_desire_speed(0);
}