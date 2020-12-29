#include "process.h"

void run_test(fnClean_t *fnClean) {
    for (;;) { usleep(1000 * 100); }
}

bool check_test(fnRun_t *fnRun) {
    *fnRun = run_test;
    return true;
}

void init_test(fnCheck_t *fnCheck) {
    while (!recog->psd.valid) {};
    while (1) {
        printf("left1 %3.1f [cm] right2 %3.1f[cm] \n",
               recog->psd.value[PSD_LEFT_1], recog->psd.value[PSD_RIGHT_1]);
        fflush(stdout);
        usleep(50000);
    }

    *fnCheck = check_test;
}