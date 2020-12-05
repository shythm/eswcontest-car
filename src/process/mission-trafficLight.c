#include "car-header.h"
#include "ctrlboard-lib.h"
#include "process.h"
#include "recognize-lib.h"
#include <limits.h>

#define TL_SPEED       70
#define TL_SLEEP       100000
#define BAR_LEN        40
#define GAIN_POSITION  50
#define LAST_TURN_TICK PI * 8.0 / 18.0 * RADIUS *TICK_PER_CM // 80-degree

void init_trafficLight(State *state);
void check_trafficLight(State *state);
void do_trafficLight(State *state);
void dive_into_end_point(State *state);

int discover_stop_line = 1; // 1:practice, 0: real

void init_trafficLight(State *state) {
    state->input->is_on_end_zone.enabled  = false;
    state->input->is_on_stop_line.enabled = true;
    state->input->traffic_light.enabled   = false;
    state->input->tl_lane.enable          = false;
    state->missions.trafficLight.function = do_trafficLight;

    mqid = state->ctrl;
}

void check_trafficLight(State *state) {
    // state->missions.trafficLight.priority = 10;
    return;
    state->input->is_on_stop_line.enabled = true;
    // printf("%d\n", state->input->is_on_stop_line.value);
    if (discover_stop_line >= 2) return;
    if (state->input->is_on_stop_line.value == true) {
        if (discover_stop_line == 0) {
            discover_stop_line++;
        } else if (discover_stop_line == 1) {
            discover_stop_line++;
            state->missions.trafficLight.priority = 10;
            set_desire_speed(0); // stop on the stop line
            beep(50);
            state->input->is_on_stop_line.enabled = false;
            // state->input->tl_lane.enable = true;
        }
    }
}

void do_trafficLight(State *state) {
    state->input->lane.enabled            = false;
    state->input->is_on_stop_line.enabled = false;
    // sleep(1);
    // set_desire_speed(  35);
    // while (1) {
    //     set_steering(  -(state->input->tl_lane.value.position) * 50 +
    //     1500);
    // }

    // stop on the stop line
    set_desire_speed(0);

    // tilt up camera to see trafficLight
    set_camer_Yservo(1500);
    state->input->traffic_light.enabled   = true;
    state->input->is_on_stop_line.enabled = false;
    usleep(TL_SLEEP);

    // wait for signal
    recog_traffic_light_t tl = state->input->traffic_light.value;
    while (tl != TL_LEFT && tl != TL_GREEN)
        tl = state->input->traffic_light.value;

    // progress
    set_steering(1500);
    set_desire_speed(TL_SPEED);
    while (state->input->psd.value[PSD_FRONT] > 26.f) {
        // printf("PSD FRONT: %3.1f\n", state->input->psd.value[PSD_FRONT]);
    }
    set_desire_speed(0);

    // regress
    set_desire_speed(0);
    usleep(TL_SLEEP);
    if (tl == TL_LEFT) // left: regress more
        move(-TL_SPEED, -10.f * TICK_PER_CM);
    else // right: regress less
        move(-TL_SPEED, -5.f * TICK_PER_CM);
    /*
   // get traffic light signals
    int sig_left = 0, sig_right = 0;
    for (int i = 0; i < 100;) {
        recog_traffic_light_t tl = state->input->traffic_light.value;
        if (tl == TL_LEFT) sig_left++, i++;
        else if (tl == TL_GREEN)
            sig_right++, i++;
        usleep(10);
    }
    */

    state->input->traffic_light.enabled   = false;
    state->input->tl_lane.enable          = true;
    state->input->is_on_end_point.enabled = true;

    // tilt down camera
    set_camer_Yservo(1700);

    if (tl == TL_LEFT) set_steering(2000);
    else if (tl == TL_GREEN)
        set_steering(1000);
    usleep(TL_SLEEP);

    // turn xx-degree
    move(TL_SPEED, LAST_TURN_TICK);

    // progress toward end point
    set_desire_speed(TL_SPEED);
#if 0
    while (state->input->is_on_end_point.value) {
        set_steering(  state->input->tl_lane.value.position * -50 + 1500);
    }
#else
    while (!state->input->is_on_end_point.value) dive_into_end_point(state);
    while (state->input->is_on_end_point.value) dive_into_end_point(state);

#endif
    set_desire_speed(0);

    /*
        // turn left or right 90-degrees
        set_steering(  (sig_left > sig_right) ? 2000 : 1000);
        move(  TL_SPEED, RADIUS * PI / 2.0f * TICK_PER_CM);

        // progress into end-point
        state->input->is_on_end_point.enabled = true;
        set_steering(  1500);
        set_desire_speed(  TL_SPEED);
        set_encoder_counter(  0);
        usleep(TL_SLEEP);
        // progress until I can see end point
        while (!state->input->is_on_end_point.value) usleep(1000);
        // progress until I can't see end point
        while (state->input->is_on_end_point.value) usleep(1000);
        set_desire_speed(  0);
    */

    // end trafficLight
    beep(50);
    while (1) sleep(1);
}
void dive_into_end_point(State *state) {
    int steering = state->input->tl_lane.value * -GAIN_POSITION + 1500;
    if (steering > 2000) steering = 2000;
    else if (steering < 1000)
        steering = 1000;
    set_steering((short)steering);
}