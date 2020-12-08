#include "process.h"

recog_result *recog;
mqid_ctrl     ctrl;

// default functions
bool default_fnCheck() { return true; }
void default_fnRun() { return; }
void default_fnClean() { return; }

extern fnInit_t mlist_contest[];  // mission list for contest  (in missions.c)
extern fnInit_t mlist_practice[]; // mission list for practice (in missions.c)

// main function
int main() {
    usleep(1000 * 1000);

    // Get shared memory and message queue to external informations for missions
    get_shm_recog_result(&recog, 0);
    get_mqid_ctrl(&ctrl);
    MSG("Process has been ready!");

    fnInit_t *mlist; // mission list
    mlist = mlist_contest;
#ifdef MODE_PRACTICE
    mlist = mlist_practice;
#endif
    for (int i = 0; mlist[i]; i++) {
        init_drive(); // Init drive mission

        fnInit_t  init  = mlist[i];        // mission init function
        fnCheck_t check = default_fnCheck; // mission check function
        fnRun_t   run   = default_fnRun;   // mission run function
        fnClean_t clean = default_fnClean; // mission clear function

        init(&check); // Initialize and get check functions.
        while (!check(&run))
            do_drive(); // Do drive until the mission is detected.
        run(&clean);    // Run the mission.
        clean();        // Clear the mission.
    }

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