#include "ctrlboard-lib.h"
#include "process.h"

void do_overtaking(State *state);

void init_overtaking(State *state) {
    state->missions.overtaking.function = do_overtaking;
}

void check_overtaking(State *state) {}

void do_overtaking(State *state) {}
