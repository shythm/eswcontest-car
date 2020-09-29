#include "ctrlboard-lib.h"
#include "recognize-lib.h"
#include "process.h"

#define STRAIGHT_DISTANCE 100
#define ROTATE_DISTANCE 100
#define ROTATE_STEERING 100

typedef ctrlboard_byte_container container;

void init_roundabout(State *state);
void check_roundabout(State *state);
void do_roundabout(State *state);
void set_speed(mqid_ctrl mqid, short speed);

void init_roundabout(State *state) {
    state->input->is_on_stop_line.enabled = true;
}

void check_roundabout(State *state) {
    if (state->input->is_on_stop_line.value) {
        set_speed(state->ctrl, 0);
        state->missions.roundabout.priority = 2;
        state->missions.roundabout.function = do_roundabout;
    } else {
        state->missions.roundabout.priority = 0;
        state->missions.roundabout.function = NULL;
    }
}

void do_roundabout(State *state) {
    // while (true) {
    //     printf("PSD: %f\n", state->input->psd.value[PSD_FRONT]);
    //     printf("Left Lane: %f\n", state->input->lane.value.left_pos);
    //     printf("Right Lane: %f\n", state->input->lane.value.right_pos);
    //     printf("Lane Pos: %f\n", state->input->lane.value.position);
    //     printf("\n");
    //     fflush(stdout);
    //     usleep(1000*1000);
    // }


}

