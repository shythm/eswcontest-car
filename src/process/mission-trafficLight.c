#include "car-header.h"
#include "process.h"

#define SPEED_TL       110
#define VELO_STOP_LINE 50 // velocity at stop line

#define REGRESS_ENABLE 0

bool check_trafficLight(fnRun_t *);
void do_trafficLight(fnClean_t *);
void clean_trafficLight();
void dive_into_end_zone();

float pre_target_velo;

void init_trafficLight(fnCheck_t *fnCheck) {
    recog->is_on_end_zone.enabled = false;
    recog->traffic_light.enabled  = false;
    recog->tl_lane.enable         = false;
    recog->stop_line_pos.enable   = true;

    MSG("UPCOMING MISSION => trafficLight & end-zone");
    pre_target_velo = (float)target_velo;
    *fnCheck        = check_trafficLight;
}

bool check_trafficLight(fnRun_t *fnRun) {
    static float stop_line_pos      = -1.0f;
    static bool  is_there_stop_line = false;
    if (get_is_on_stop_line()) {
        MSG("START MISSION => trafficLight & end-zone");
        recog->ext_data.call_init_lane_info = true; // 차선 인식 정보 초기화
        recog->stop_line_pos.enable         = false;
        set_desire_speed(0);
        beep(50);
        *fnRun = do_trafficLight;
        return true;
    }
    // 카메라에 정지선 감지되면 감속(감지되지 않으면 음수)
    stop_line_pos = recog->stop_line_pos.value;
    if (stop_line_pos > 0.0f) { // 정지선에 가까워질수록 속도가 느려짐
        float temp_velo = (VELO_STOP_LINE - pre_target_velo) * stop_line_pos +
                          pre_target_velo;
        // VELO_STOP_LINE < 다음 target_velo < 현재 target_velo
        target_velo = CONSTRAIN(temp_velo, VELO_STOP_LINE, target_velo);
    }
    return false;
}

void do_trafficLight(fnClean_t *fnClean) {
    const int straight_tick = (int)(33.f * TICK_PER_CM);
    const int turn_tick     = (int)(PI * 90.0F / 180.0F * RADIUS * TICK_PER_CM);
    recog->lane.enabled     = false;
    recog->traffic_light.enabled = true;

    // stop
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

    recog->tl_lane.enable         = true;
    recog->is_on_end_zone.enabled = true;

#if REGRESS_ENABLE
    // progress
    set_steering(1500);
    set_desire_speed(SPEED_TL);
    while (recog->psd.value[PSD_FRONT] > 26.f) {
        // printf("PSD FRONT: %3.1f\n", recog->psd.value[PSD_FRONT]);
    }
    set_desire_speed(0);
    usleep(TL_SLEEP);

    // regress
    if (tl_direction < 0) { // left
        move(-SPEED_TL, -10.f * TICK_PER_CM);
        set_steering(2000);
    } else { // right
        move(-SPEED_TL, -10.f * TICK_PER_CM);
        set_steering(1000);
    }
#else
    // progress
    move(SPEED_TL, straight_tick);
    usleep(SLEEP_STOP);
    // turn
    set_steering((tl_direction < 0) ? 2000 : 1000); // left : right
    move(SPEED_TL, turn_tick);
#endif

    // progress toward end point
    set_desire_speed(SPEED_TL);
    while (!recog->is_on_end_zone.value) dive_into_end_zone();
    while (recog->is_on_end_zone.value) dive_into_end_zone();
    set_desire_speed(0);

    // end trafficLight
    beep(50);
    *fnClean = clean_trafficLight;
}

void clean_trafficLight() {
    recog->is_on_end_zone.enabled       = false;
    recog->traffic_light.enabled        = false;
    recog->tl_lane.enable               = false;
    recog->stop_line_pos.enable         = false;
    recog->ext_data.call_init_lane_info = true;

    sleep(2);
    // while (1) sleep(1); /* THE END */
}

void dive_into_end_zone() {
    static const float gain     = 35.f;
    short              steering = (-recog->tl_lane.value * gain) + 1500.f;
    // constrain 1200~1800
    steering = CONSTRAIN(steering, 1200, 1800);
    set_steering(steering);
}