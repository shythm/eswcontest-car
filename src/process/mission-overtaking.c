#include "ctrlboard-lib.h"
#include "process.h"

typedef ctrlboard_byte_container container;

void do_overtaking(State *state);

void init_overtaking(State *state) {
    state->input->is_there_car.enabled  = true;
    state->missions.overtaking.function = do_overtaking;
}

void check_overtaking(State *state) {
    // if (state->input->is_on_stop_line.value == true)
    state->missions.overtaking.priority = 10;
    printf("check-ov\n");
    container data;
    data.c_int16 = 1700;
    if (send_ctrlboard(state->ctrl, CMD_CAMERA_Y_SERVO_CONTROL, 2, &data) !=
        MSG_STATE_SUCCESS)
        printf("fail to set steering : mission-parking ");
}

void do_overtaking(State *state) {
    sleep(1);
    // container data = {50};
    // if (send_ctrlboard(state->ctrl, CMD_SOUND, 1, &data) !=
    // MSG_STATE_SUCCESS)
    //     printf("fail to sound beep: mission-parking\n");
}