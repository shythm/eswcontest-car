#include "ctrlboard-lib.h"
#include "process.h"

void do_obstacle(State *state);

void init_obstacle(State *state) {
    state->input->stop_obstacle.enabled = true;
    state->missions.obstacle.function   = do_obstacle;
}

void check_obstacle(State *state) {
    static int is_checked = 0;

    if (!is_checked) {
        if (state->input->stop_obstacle.value.pos_x != -1) {
            printf("!");
            state->missions.obstacle.priority = 10;
        }

        is_checked = 1;
    } else {
        state->input->stop_obstacle.enabled = false;
    }
}

void do_obstacle(State *state) {
    int                      msgqid = state->msgq_id;
    ctrlboard_byte_container container;

    container.c_int16 = 0;
    message_ctrlboard(msgqid, MSGQ_ID_PROCESS, CMD_DESIRE_SPEED, CMD_TYPE_WRITE,
                      2, &container);

    // block until the obstacle is disappear.
    while (state->input->stop_obstacle.value.pos_x != -1) {}
}