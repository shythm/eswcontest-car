#include "car-header.h"
#include "process.h"

#define THRESHOLD_SO_AREA 1000.0f
#define ONE_SHOT_COUNT    250
#define ONE_SHOT_DELAY_US 1000
#define WAIT_DELAY        1

bool check_obstacle(fnRun_t *fnRun);
void run_obstacle(fnClean_t *fnClear);
void clean_obstacle();

void init_obstacle(fnCheck_t *fnCheck) {
    recog->stop_obstacle.value.area = 0.0f;
    recog->stop_obstacle.enabled    = true;

    set_camera_Yservo(1650);

    MSG("UPCOMING MISSION => obstacle");
    *fnCheck = check_obstacle;
}

bool check_obstacle(fnRun_t *fnRun) {
    if (recog->stop_obstacle.value.area > THRESHOLD_SO_AREA) {
        MSG("START MISSION => obstacle");
        *fnRun = run_obstacle;
        return true;
    }

    return false;
}

void run_obstacle(fnClean_t *fnClean) {
    set_desire_speed(0);
    beep(50);

    // block until the obstacle is disappear.
    for (int i = 0; i < ONE_SHOT_COUNT; i++) {
        if (recog->stop_obstacle.value.area > THRESHOLD_SO_AREA) i = 0;
        usleep(ONE_SHOT_DELAY_US);
    }

    // wait while the priority stop obstacle is dissapeared.
    sleep(WAIT_DELAY);

    *fnClean = clean_obstacle;
}

void clean_obstacle() {
    // turn off stop_obstacle recognition.
    recog->stop_obstacle.enabled        = false;
    recog->ext_data.call_init_lane_info = true;
}