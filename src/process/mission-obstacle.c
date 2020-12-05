#include "car-header.h"
#include "ctrlboard-lib.h"
#include "process.h"
void do_obstacle(State *state);

#define THRESHOLD_SO_AREA 1000.0f
#define ONE_SHOT_COUNT    250
#define ONE_SHOT_DELAY_US 1000
#define WAIT_DELAY        2

static recog_stop_obstacle_data *so_data;

void init_obstacle(State *state) {
    so_data             = &(state->input->stop_obstacle);
    so_data->value.area = 0.0f;
    so_data->enabled    = true;

    state->missions.obstacle.function = do_obstacle;
}

void check_obstacle(State *state) {
    static int is_checked = false;

    if (!is_checked) {
        if (so_data->value.area > THRESHOLD_SO_AREA) {
            state->missions.obstacle.priority = 3;

            is_checked = true;
        }
    }
}

void do_obstacle(State *state) {
    ctrlboard_byte_container container;

    container.c_int16 = 0;
    send_ctrlboard(state->ctrl, CMD_DESIRE_SPEED, 2, &container);

    beep(50);

    // block until the obstacle is disappear.
    for (int i = 0; i < ONE_SHOT_COUNT; i++) {
        if (so_data->value.area > THRESHOLD_SO_AREA) i = 0;
        usleep(ONE_SHOT_DELAY_US);
    }

    // wait while the priority stop obstacle is dissapeared.
    sleep(WAIT_DELAY);

    // restore and clear
    init_drive(state);
    so_data->enabled = false;
}