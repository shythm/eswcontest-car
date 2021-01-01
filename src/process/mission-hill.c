#include "car-header.h"
#include "process.h"
#include <math.h>
#include <signal.h>
#include <unistd.h>

/************************************/
/*   Constants for uphill mission   */
/************************************/
#define VELO_HILL 250

void init_hill(fnCheck_t *);
bool check_hill(fnRun_t *);
void run_hill(fnClean_t *);
void clean_hill(void);

void init_hill(fnCheck_t *fnCheck) {
    MSG("UPCOMING MISSION => HILL");
    recog->slope.enabled = true;

    *fnCheck = check_hill;
}

bool check_hill(fnRun_t *fnRun) {
    if (recog->slope.value == SLOPE_UPHILL) {
        MSG("START MISSION => HILL");
        *fnRun = run_hill;
        return true;
    }

    return false;
}

void run_hill(fnClean_t *fnClean) {
    // 언덕을 만나면 4000tick(약 2m) 만큼 직진 후 언덕이 종료됐다고 판단
    move(VELO_HILL, 4000);
    *fnClean = clean_hill;
    MSG("CLEAR MISSION => HILL");
}

void clean_hill(void) { recog->slope.enabled = false; }
