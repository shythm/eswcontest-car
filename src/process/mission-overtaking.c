#include "car-header.h"
#include "process.h"

#define SPEED_OVERTAKING 60
#define SLEEP_OVERTAKING 400000

void init_overtaking(fnCheck_t *fnCheck);
bool check_overtaking(fnRun_t *fnRun);
void do_overtaking(fnClean_t *fnClean);
void clean_overtaking(void);

void init_overtaking(fnCheck_t *fnCheck) {
    // set_desire_speed(0);
    // set_camera_Yservo(1600);
    // recog->other_cars.enable = 1;
    // while (1) {}
    set_steering(1500);
    // wait until psd is valid
    // sleep(2);
    while (!recog->psd.valid) {}
    MSG("UPCOMING MISSION => overtaking");
    *fnCheck = check_overtaking;
}

bool check_overtaking(fnRun_t *fnRun) {
    // set_camera_Yservo(1600);
    // recog->other_cars.enable = true;
    // while (1) {}

    if (recog->psd.value[PSD_FRONT] < 28.f) {
        set_desire_speed(0);
        *fnRun = do_overtaking;
        MSG("START MISSION => overtaking");
        MSG("1: psd %3.1f", recog->psd.value[PSD_FRONT]);
        return true;
    }
    return false;
}

void do_overtaking(fnClean_t *fnClean) {
    int         overtaking_direction;
    const float turn_rad = PI / 3.0f;
    // stop
    set_desire_speed(0);
    set_steering(1500);
    usleep(SLEEP_OVERTAKING * 3);

    set_desire_speed(-SPEED_OVERTAKING / 2);

    MSG("2: psd %3.1f", recog->psd.value[PSD_FRONT]);
    while (recog->psd.value[PSD_FRONT] < 20.0f) {}
    set_desire_speed(0);
    usleep(SLEEP_OVERTAKING);
    MSG("1: psd %3.1f", recog->psd.value[PSD_FRONT]);

    move(-SPEED_OVERTAKING, -25.f * TICK_PER_CM);

    // search empty road
    recog->other_cars.enable = true;
    set_camera_Yservo(1600);
    usleep(SLEEP_OVERTAKING * 2);
    sleep(3);
    overtaking_direction = 0;
    for (int i = 0; i < 100; i++) {

        overtaking_direction += recog->other_cars.value;
        usleep(2500);
    }

    int steer_direc = 500;
    if (overtaking_direction < 0) { // left
        steer_direc = -500;
        MSG("LEFT!!!\n");
    } else if (overtaking_direction >= 0) { // right
        steer_direc = 500;
        MSG("RIGHT!!!\n");
    }

    set_camera_Yservo(1700);
    recog->other_cars.enable = false;

    // RIGHT | LEFT
    // turn right | left
    set_steering(1500 - steer_direc);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, turn_rad * RADIUS * TICK_PER_CM);
    usleep(SLEEP_OVERTAKING);

    // turn left | right
    set_steering(1500 + steer_direc);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, turn_rad * RADIUS * TICK_PER_CM);
    usleep(SLEEP_OVERTAKING);

    // go straight
    set_steering(1500);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, 40.0f * TICK_PER_CM);
    usleep(SLEEP_OVERTAKING);

    // turn left | right
    set_steering(1500 + steer_direc);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, turn_rad * RADIUS * TICK_PER_CM);
    usleep(SLEEP_OVERTAKING);

    // turn right | left
    set_steering(1500 - steer_direc);
    usleep(SLEEP_OVERTAKING * 2);
    move(SPEED_OVERTAKING, turn_rad * RADIUS * TICK_PER_CM);

    set_steering(1500);
}

void clean_overtaking() {}
