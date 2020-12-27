#include "process.h"

recog_result *recog;
short         target_velo;

extern fnInit_t mission_list[]; // mission list for contest  (in missions.c)

// main function
int main() {
    usleep(1000 * 1000);

    get_shm_recog_result(&recog, 0); // Get external informations for missions
    ctrld_init();                    // Initialize ctrlboard-direct
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
            do_drive(); // Do drive until the mission is detected.
        if (run) {      // Run the mission.
            record_ticks(init);
            run(&clean);
            record_ticks(init);
        }
        if (clean) clean(); // Clear the mission.
    }

    MSG("All missions were completed!");
    return 0;
}
