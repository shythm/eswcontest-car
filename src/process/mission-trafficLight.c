#include "car-header.h"
#include "ctrlboard-lib.h"
#include "process.h"
#include "recognize-lib.h"

#define TL_SPEED 75
#define TL_SLEEP 500000

void init_trafficLight(State *state);
void check_trafficLight(State *state);
void do_trafficLight(State *state);

int       discover_stop_line = 1; // 1:practice, 0: real
mqid_ctrl mqid;

void init_trafficLight(State *state) {
    state->input->is_on_end_point.enabled = false;
    state->input->is_on_stop_line.enabled = true;
    state->input->traffic_light.enabled   = false;
    state->missions.trafficLight.function = do_trafficLight;

    mqid = state->ctrl;
}

void check_trafficLight(State *state) {
    if (discover_stop_line >= 2) return;
    if (state->input->is_on_stop_line.value == true) {
        if (discover_stop_line == 0) {
            discover_stop_line++;
        } else if (discover_stop_line == 1) {
            discover_stop_line++;
            state->missions.trafficLight.priority = 10;
            set_desire_speed(mqid, 0); // stop on the stop line
            beep(mqid, 50);
        }
    }
}

void do_trafficLight(State *state) {

    // stop on the stop line
    set_desire_speed(mqid, 0);

    // tilt up camera to see trafficLight
    set_camer_Yservo(mqid, 1500);
    state->input->traffic_light.enabled   = true;
    state->input->is_on_stop_line.enabled = false;
    sleep(1);

    // wait for signal
    for (recog_traffic_light_t tl = state->input->traffic_light.value;
         tl != TL_LEFT && tl != TL_GREEN;
         tl = state->input->traffic_light.value) {
        usleep(1000);
    }

    // progress
    set_steering(mqid, 1500);
    set_desire_speed(mqid, TL_SPEED);
    while (state->input->psd.value[PSD_FRONT] > 26.f) {
        // printf("PSD FRONT: %3.1f\n", state->input->psd.value[PSD_FRONT]);
        usleep(1000);
    }
    set_desire_speed(mqid, 0);
    usleep(TL_SLEEP);

    // regress
    move(mqid, -50, -13.f * TICK_PER_CM);
    usleep(TL_SLEEP);

    // get traffic light signals
    int sig_left = 0, sig_right = 0;
    for (int i = 0; i < 100;) {
        recog_traffic_light_t tl = state->input->traffic_light.value;
        if (tl == TL_LEFT) sig_left++, i++;
        else if (tl == TL_GREEN)
            sig_right++, i++;
        usleep(10000);
    }

    // tilt down camera
    set_camer_Yservo(mqid, 1700);

    // turn left or right 90-degree
    set_steering(mqid, (sig_left > sig_right) ? 2000 : 1000);
    move(mqid, TL_SPEED, RADIUS * PI / 2.0f * TICK_PER_CM);

    // progress into end-point
    state->input->is_on_end_point.enabled = true;
    set_steering(mqid, 1500);
    set_desire_speed(mqid, TL_SPEED);
    set_encoder_counter(mqid, 0);
    usleep(TL_SLEEP);
    // progress until I can see end point
    while (!state->input->is_on_end_point.value) usleep(1000);
    // progress until I can't see end point
    while (state->input->is_on_end_point.value) usleep(1000);
    set_desire_speed(mqid, 0);

    beep(mqid, 50);
    sleep(2);

    // come back
    move(mqid, -TL_SPEED, -read_encoder_counter(mqid));
    set_steering(mqid, (sig_left > sig_right) ? 2000 : 1000);
    usleep(TL_SLEEP);
    move(mqid, -TL_SPEED, -RADIUS * PI / 2.0f * TICK_PER_CM);
    set_steering(mqid, 1500);
    move(mqid, -TL_SPEED, -50.f * TICK_PER_CM);

    while (1) sleep(1);
}
