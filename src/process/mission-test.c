#include "process.h"

void run_test(fnClean_t *fnClean) {
    for (;;) { usleep(1000 * 100); }
}

/* 체크가 true 면 run으로 들어감 */
bool check_test(fnRun_t *fnRun) {
    *fnRun = run_test;
    return true;
}

void init_test(fnCheck_t *fnCheck) { *fnCheck = check_test; }