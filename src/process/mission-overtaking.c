#include "process.h"

void do_overtaking(State *state) {}

void init_overtaking(State *state) {
    state->input->is_on_stop_line.enabled = true;
    state->missions.overtaking.function   = do_overtaking;
}

void check_overtaking(State *state) {
    if (state->input->is_on_stop_line.value == true)
        state->missions.overtaking.priority = 2;
}