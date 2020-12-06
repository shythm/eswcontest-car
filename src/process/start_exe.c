#include "start_exe.h"

void start_with_PSD(recog_result **input) {
    int cnt = 0;
    // wait until obstacle appear in front of car
    while (1) {
        if ((*input)->psd.value[PSD_FRONT] < 20.f) {
            cnt++;
            if (cnt > 1000) break;
        } else
            cnt = 0;
    }

    // wait until obstacle disappear in front of car
    cnt = 0;
    while (1) {
        if ((*input)->psd.value[PSD_FRONT] > 20.f) {
            cnt++;
            if (cnt > 1000) break;
        } else
            cnt = 0;
    }
    sleep(1);
}