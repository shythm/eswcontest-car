#include "car-header.h"
#include "process.h"
#include <math.h>
#include <signal.h>
#include <unistd.h>

/************************************/
/*   Constants for hill mission   */
/************************************/
#define VELO_HILL   300
#define LENGTH_HILL 110                      // (단위 : cm)
#define TICK_HILL   LENGTH_HILL *TICK_PER_CM // (단위 : tick)

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
        *fnRun = run_hill;
        return true;
    }

    return false;
}

/* 언덕을 만나면 일정 길이 만큼 직진 후 언덕이 종료됐다고 판단 */
void run_hill(fnClean_t *fnClean) {
    MSG("START MISSION => HILL");
    recog->lane.enabled = false;                   // 차선 그만보고
    ctrld_write(CMD_STEERING_SERVO_CONTROL, 1500); // 방향 값 가운데로 맞추고
    // beep(10);
    move(VELO_HILL, TICK_HILL);                 // 앞으로 좀 갔다가
    recog->ext_data.call_init_lane_info = true; // 차선 초기화 한번 해주고
    // beep(10);
    *fnClean = clean_hill; // 끝.
    MSG("CLEAR MISSION => HILL");
    // (미션이 끝나면 자동으로 주행해서 서보모터 고정은 끝난다.)
}

void clean_hill(void) { recog->slope.enabled = false; }
