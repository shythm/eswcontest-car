#include "car-header.h"
#include "ctrlboard-lib.h"
#include "process.h"

#define TL_SPEED 75
#define TL_SLEEP 500000

void init_overtaking(State *state);
void check_overtaking(State *state);
void do_overtaking(State *state);

mqid_ctrl mqid;

void init_overtaking(State *state) {
    mqid = state->ctrl;

    state->missions.overtaking.function = do_overtaking;
}

void check_overtaking(State *state) {
    if (state->input->psd.value[PSD_FRONT] < 10.f) {
        state->missions.overtaking.priority = 10;
    }
}

void do_overtaking(State *state) {
    // adsf

    // asdf
}
