#include "ctrlboard-lib.h"
#include "process.h"

void do_trafficLight(State *state);

void init_trafficLight(State *state) {
    // state->input->traffic_light.enabled   = true;
    state->input->is_on_end_point.enabled = true;

    state->missions.trafficLight.function = do_trafficLight;
}

void check_trafficLight(State *state) {
    if (state->input->is_on_end_point.value) {
        state->missions.trafficLight.priority = 0;
    }
}

void do_trafficLight(State *state) {
    ctrlboard_byte_container cont;

    // 속도를 줄임
    cont.c_int16 = 50;
    send_ctrlboard(state->ctrl, CMD_DESIRE_SPEED, 2, &cont);

    // 종료 지점이 화면에서 안 들어올 때까지 계속 앞으로 감
    while (state->input->is_on_end_point.value) {}

    // 종료 지점이 끝나면 주행 종료
    cont.c_int16 = 0;
    send_ctrlboard(state->ctrl, CMD_DESIRE_SPEED, 2, &cont);
    for (;;) {}
}