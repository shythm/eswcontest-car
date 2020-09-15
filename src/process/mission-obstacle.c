#include "ctrlboard-lib.h"
#include "process.h"

void do_obstacle(State *state);

void init_obstacle(State *state) {
    state->input->stop_obstacle.enabled     = true;
    state->input->stop_obstacle.value.pos_x = -1;
    state->missions.obstacle.function       = do_obstacle;
}

void check_obstacle(State *state) {
    static int is_checked = false;

    if (!is_checked) {
        if (state->input->stop_obstacle.value.pos_x != -1) {
            state->missions.obstacle.priority = 10;
            is_checked                        = true;
        }
    } else {
        state->input->stop_obstacle.enabled = false;
    }
}

void do_obstacle(State *state) {
    ctrlboard_byte_container container;

    container.c_int16 = 0;
    send_ctrlboard(state->ctrl, CMD_DESIRE_SPEED, 2, &container);

    // block until the obstacle is disappear.
    while (state->input->stop_obstacle.value.pos_x != -1) {}
}