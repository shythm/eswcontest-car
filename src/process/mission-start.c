#include "car-header.h"
#include "process.h"

void start_with_PSD(fnClean_t *fnClean) {
    volatile int cnt = 0, flag = 0;

    // wait until psd is ready.
    while (!recog->psd.valid) { usleep(1000); }
    MSG("Wait for start signal ...");

    while (get_is_on_stop_line()) { // Calibration Alert
        beep(30);
        MSG("Please Calibration IR Sensors!");
        usleep(500000);
    }

    // wait until the car is on the overpass
    cnt = 0, flag = 0;
    while (1) {
        for (int i = 1; i < PSD_COUNT; i++) {
            if (recog->psd.value[i] < 20.f) cnt++;
        }
        if (cnt >= 3) flag++;
        if (flag >= 1000) break;
    }

    // wait until obstacle appear in front of car
    cnt = 0, flag = 0;
    while (1) {
        if (recog->psd.value[PSD_FRONT] < 20.f) {
            cnt++;
            if (cnt > 1000) break;
        } else
            cnt = 0;
    }
    // wait until obstacle disappear in front of car
    cnt = 0, flag = 0;
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
    ctrld_write(CMD_ENCODER_COUNTER, 0);
    *fnCheck = check_start;
}