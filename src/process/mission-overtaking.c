#include "car-header.h"
#include "process.h"

#define SPEED_OVERTAKING 60
#define SLEEP_OVERTAKING 500000

void init_overtaking(fnCheck_t *fnCheck);
bool check_overtaking(fnRun_t *fnRun);
void do_overtaking(fnClean_t *fnClean);
void clean_overtaking(void);

void init_overtaking(fnCheck_t *fnCheck) {
    // wait until psd is valid
    sleep(2);
    while (!recog->psd.valid) {}
    MSG("UPCOMING MISSION => overtaking");
    *fnCheck = check_overtaking;
}

bool check_overtaking(fnRun_t *fnRun) {
    fflush(stdout);
    if (recog->psd.value[PSD_FRONT] < 20.f) {
        *fnRun = do_overtaking;
        MSG("START MISSION => overtaking");
        return true;
    }
    return false;
}

void do_overtaking(fnClean_t *fnClean) {
    MSG("enter do");
    enum { NONE, LEFT, RIGHT } overtaking_direction = NONE;
    const float turn_rad1                           = PI / 3.0f;
    // stop
    set_desire_speed(0);
    usleep(SLEEP_OVERTAKING);
    MSG("1");

    if (recog->psd.value[PSD_FRONT] < 25.f) {
        set_desire_speed(-SPEED_OVERTAKING);
        while (recog->psd.value[PSD_FRONT] < 20.0f) {}
        set_desire_speed(0);
        usleep(SLEEP_OVERTAKING);
    }

    // search empty road
    overtaking_direction = RIGHT;
    MSG("2");
    // turn right
    set_steering(1000);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, turn_rad1 * RADIUS * TICK_PER_CM);
    usleep(SLEEP_OVERTAKING);

    // turn left
    set_steering(2000);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, turn_rad1 * RADIUS * TICK_PER_CM);
    usleep(SLEEP_OVERTAKING);

    // go straight
    set_steering(1500);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, 10.0f * TICK_PER_CM);
    usleep(SLEEP_OVERTAKING);

    // turn left
    set_steering(2000);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, turn_rad1 * RADIUS * TICK_PER_CM);
    usleep(SLEEP_OVERTAKING);

    // turn right
    set_steering(1000);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, turn_rad1 * RADIUS * TICK_PER_CM);
}

void clean_overtaking() {}
