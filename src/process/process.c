#include "process.h"

recog_result *recog;
mqid_ctrl     ctrl;

extern fnInit_t mission_list[]; // mission list for contest  (in missions.c)

// main function
int main() {
    usleep(1000 * 1000);

    // Get shared memory and message queue to external informations for missions
    get_shm_recog_result(&recog, 0);
    get_mqid_ctrl(&ctrl);
    MSG("Process has been ready!");

    for (int i = 0; mission_list[i]; i++) {
        init_drive(); // Init drive mission

        fnInit_t  init  = mission_list[i]; // mission init function
        fnCheck_t check = NULL;            // mission check function
        fnRun_t   run   = NULL;            // mission run function
        fnClean_t clean = NULL;            // mission clear function

        init(&check); // Initialize and get check functions.

        if (!check) continue;
        while (!check(&run))
            do_drive();       // Do drive until the mission is detected.
        if (run) run(&clean); // Run the mission.
        if (clean) clean();   // Clear the mission.
    }

    MSG("All missions are completed!");
    return 0;
}

ctrlboard_msg_state_t command(ctrlboard_cmd_code cmd, int data) {
    static ctrlboard_byte_container container;
    static unsigned char            bytec;
    container.c_int32 = data;
    switch (cmd) {
    case CMD_DESIRE_SPEED:
        bytec = 2;
        break;
    case CMD_DESIRE_ENCODER_COUNT:
        bytec = 4;
        break;
    case CMD_STEERING_SERVO_CONTROL:
        bytec = 2;
        break;
    case CMD_CAMERA_X_SERVO_CONTROL:
        bytec = 2;
        break;
    case CMD_CAMERA_Y_SERVO_CONTROL:
        bytec = 2;
        break;
    case CMD_ENCODER_COUNTER:
        bytec = 2;
        break;
    default:
        bytec = 1;
        break;
    }
    return send_ctrlboard(ctrl, cmd, bytec, &container);
}