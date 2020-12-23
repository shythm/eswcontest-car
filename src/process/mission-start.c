#include "car-header.h"
#include "process.h"

void start_with_PSD(fnClean_t *fnClean) {
    int cnt = 0;

    // wait until psd is ready.
    while (!recog->psd.valid) { usleep(1000); }
    MSG("Wait for start signal ...");

    // wait until obstacle appear in front of car
    cnt = 0;
    while (1) {
        if (recog->psd.value[PSD_FRONT] < 20.f) {
            cnt++;
            if (cnt > 1000) break;
        } else
            cnt = 0;
    }

    // wait until obstacle disappear in front of car
    cnt = 0;
    while (1) {
        if (recog->psd.value[PSD_FRONT] > 20.f) {
            cnt++;
            if (cnt > 1000) break;
        } else
            cnt = 0;
    }

    beep(50);
    MSG("Start mission !!!");
    init_drive(); // Init drive mission
}

bool check_start(fnRun_t *fnRun) {
    *fnRun = start_with_PSD; // regist the run function
    return true;
}

void init_start(fnCheck_t *fnCheck) {
    set_desire_speed(0);
    *fnCheck = check_start;
}