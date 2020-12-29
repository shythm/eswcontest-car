#include "car-header.h"
#include "process.h"

#define TL_SPEED       90
#define TL_SLEEP       100000 // 0.1s
#define GAIN_POSITION  35
#define LAST_TURN_TICK (PI * 80.0F / 180.0F * RADIUS * TICK_PER_CM) // 80-degree

bool check_trafficLight(fnRun_t *);
void do_trafficLight(fnClean_t *);
void clean_trafficLight();
void dive_into_end_point();

void init_trafficLight(fnCheck_t *fnCheck) {
    recog->is_on_end_zone.enabled = false;
    recog->traffic_light.enabled  = false;
    recog->tl_lane.enable         = false;

    MSG("UPCOMING MISSION => trafficLight & end-zone");
    *fnCheck = check_trafficLight;
}

bool check_trafficLight(fnRun_t *fnRun) {
    if (get_is_on_stop_line()) {
        set_desire_speed(0); // stop on the stop line
        beep(50);
        MSG("START MISSION => trafficLight & end-zone");
        *fnRun = do_trafficLight;
        return true;
    }
    return false;
}

void do_trafficLight(fnClean_t *fnClean) {
    recog->lane.enabled          = false;
    recog->traffic_light.enabled = true;
    // stop on the stop line
    set_desire_speed(0);

    // tilt up camera to see trafficLight
    set_camera_Yservo(1500);
    sleep(1);

    // wait for signal
    volatile recog_traffic_light_t tl           = recog->traffic_light.value;
    int                            tl_direction = 0;
    for (int i = 0; i < 10; tl = recog->traffic_light.value) {
        if (tl == TL_LEFT) {
            tl_direction--;
            i++;
        } else if (tl == TL_GREEN) {
            tl_direction++;
            i++;
        } else
            continue;
        usleep(10000);
    }
    recog->traffic_light.enabled = false;
    // tilt down camera
    set_camera_Yservo(1700);

    // progress
    set_steering(1500);
    set_desire_speed(TL_SPEED);
    while (recog->psd.value[PSD_FRONT] > 26.f) {
        // printf("PSD FRONT: %3.1f\n", recog->psd.value[PSD_FRONT]);
    }
    set_desire_speed(0);
    usleep(TL_SLEEP);

    // regress
    if (tl_direction < 0) { // left
        move(-TL_SPEED, -10.f * TICK_PER_CM);
        set_steering(2000);
    } else { // right
        move(-TL_SPEED, -5.f * TICK_PER_CM);
        set_steering(1000);
    }
    recog->tl_lane.enable         = true;
    recog->is_on_end_zone.enabled = true;
    usleep(TL_SLEEP);

    // turn xx-degree
    move(TL_SPEED, LAST_TURN_TICK);

    // progress toward end point
    set_desire_speed(TL_SPEED);

    while (!recog->is_on_end_zone.value) dive_into_end_point();
    while (recog->is_on_end_zone.value) dive_into_end_point();
    set_desire_speed(0);

    // end trafficLight
    beep(50);
    *fnClean = clean_trafficLight;
}

void clean_trafficLight() {
    recog->is_on_end_zone.enabled = false;
    recog->traffic_light.enabled  = false;
    recog->tl_lane.enable         = false;

    sleep(2);
    // while (1) sleep(1); /* THE END */
}

void dive_into_end_point() {
    short steering = (-recog->tl_lane.value * GAIN_POSITION) + 1500.f;
    // constrain 1200~1800
    steering = CONSTRAIN(steering, 1200, 1800);
    set_steering(steering);
}