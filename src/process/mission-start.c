#include "car-header.h"
#include "process.h"

void start_with_PSD(recog_result *input) {
    int cnt = 0;

    // wait until psd is ready.
    while (!input->psd.valid) { usleep(1000); }

    // wait until obstacle appear in front of car
    cnt = 0;
    while (1) {
        if (input->psd.value[PSD_FRONT] < 20.f) {
            cnt++;
            if (cnt > 1000) break;
        } else
            cnt = 0;
    }

    // wait until obstacle disappear in front of car
    cnt = 0;
    while (1) {
        if (input->psd.value[PSD_FRONT] > 20.f) {
            cnt++;
            if (cnt > 1000) break;
        } else
            cnt = 0;
    }

    beep(50);
    sleep(1);
}

void do_start(State *state) { start_with_PSD(state->input); }