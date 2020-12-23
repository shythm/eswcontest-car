#include "car-header.h"
#include "process.h"
#include <math.h>

/************************************/
/* Constants for roundabout mission */
/************************************/
#define GAIN_P         15.0f // P gain of PID control
#define GAIN_P_RUSH    15.0f
#define VELO           100 // velocity
#define VELO_RUSH      150
#define VELO_STOP_LINE 80 // velocity at stop line

// 차선 위치 보정 정도(실수 전체, 바깥쪽으로 돌려면 음수여야 함)
#define POS_COMP_DEGREE -4.0f
// 곡선 구간을 판단을 위한 상수(양수)
#define POS_CURVE_CONDITION 4.0f
// 회전 교차로 탈출 조건: 얼마나 직선 구간을 주행했는가에 대한 상수(단위: cm)
#define STRAIGHT_DIST_CONDITION 70.0f
// 전방 또는 후방 차량 유무 결정(양수)
#define PSD_STOP_CONDITION 29.0f
// 측면 차량 유무 결정(양수)
#define PSD_SIDE_STOP_CONDITION 8.0f
/************************************/

bool check_roundabout(fnRun_t *);
void run_roundabout(fnClean_t *);
void clean_roundabout(void);

void init_roundabout(fnCheck_t *fnCheck) {
    MSG("UPCOMING MISSION => roundabout");
    recog->stop_line_pos.enable = true;

    *fnCheck = check_roundabout;
}

void clean_roundabout(void) { recog->stop_line_pos.enable = false; }

bool check_roundabout(fnRun_t *fnRun) {
    static float stop_line_pos      = -1.0f;
    static bool  is_there_stop_line = false;

    if (is_there_stop_line && get_is_on_stop_line()) {
        MSG("START MISSION => roundabout");
        recog->ext_data.call_init_lane_info = true; // 차선 인식 정보 초기화
        set_desire_speed(0);
        *fnRun = run_roundabout;
        return true;
    }

    // 카메라에 정지선 감지되면 감속(감지되지 않으면 음수)
    stop_line_pos = recog->stop_line_pos.value;
    if (stop_line_pos > 0.0f) {
        // 정지선에 가까워질수록 속도가 느려짐
        target_velo = target_velo * pow(1.0f - stop_line_pos, 2);
        if (target_velo < VELO_STOP_LINE) {
            // 속도 제한 (최소 VELO_STOP_LINE으로)
            target_velo = VELO_STOP_LINE;
        }
        is_there_stop_line = true;
    }

    return false;
}

static void steering(float pos, float gain_p) {
    // PI-control with pos value and convert control value to steer value
    short steering_val = 1500 + (short)(pos * gain_p);

    // Limit steering range
    steering_val = MIN(steering_val, 2000);
    steering_val = MAX(steering_val, 1000);

    // Send steering value to hardware
    ctrld_write(CMD_STEERING_SERVO_CONTROL, steering_val);
}

void run_roundabout(fnClean_t *fnClean) {
    // 앞에 차량이 지나갈 때까지 기다림
    while (recog->psd.value[PSD_FRONT] > 29.0f) usleep(1000 * 100);

    // 회전 교차로 탈출을 위한 변수들
    float curve_degree       = 0.0f;
    float straight_dist_accu = 0.0f; // 누적된 직선 구간 거리(cm)
    float straight_dist_curr = 0.0f; // 현재 직선 구간 거리(cm)
    bool  is_straight        = false;
    int encoder_prev_count = 0; // 직선 구간에 진입한 시점의 엔코더 값

    // 주행을 위한 변수들
    short velo                = VELO;
    float gain_p              = GAIN_P;
    float straight_dist_total = 0.0f; // 주행한 총 직선 거리
    float pos_comp            = 0.0f; // 보정된 차선 위치

    for (;;) {
        /* 앞 또는 뒤 차량과 충돌 방지 */
        if (recog->psd.value[PSD_FRONT] < PSD_STOP_CONDITION ||
            recog->psd.value[PSD_LEFT_1] < PSD_SIDE_STOP_CONDITION) {
            // 전방에 차량이 감지되면 일단 멈춤
            set_desire_speed(0);
            usleep(1000 * 1000);
        } else if (recog->psd.value[PSD_BACK] < PSD_STOP_CONDITION ||
                   recog->psd.value[PSD_LEFT_2] < PSD_SIDE_STOP_CONDITION) {
            // 후방에 차량이 감지되면 속도 높임
            velo   = VELO_RUSH;
            gain_p = GAIN_P_RUSH;
        }

        /* 조향 및 주행 */
        pos_comp =
            recog->lane.value.pos_yawl + POS_COMP_DEGREE; // 차선 위치 보정
        if (pos_comp < -POS_CURVE_CONDITION) {
            // 차선 위치 제한(우회전을 많이 안하는 방향으로)
            pos_comp = -POS_CURVE_CONDITION;
        }
        steering(pos_comp, gain_p);
        set_desire_speed(velo);

        /* 미션 탈출 검사 */
        if (abs(pos_comp) < POS_CURVE_CONDITION) { // 직선 주행 중이라면
            // 직선 구간 거리 갱신
            if (is_straight == false) {
                straight_dist_accu += straight_dist_curr;
                encoder_prev_count = read_encoder_counter();
                is_straight        = true;
            }
            straight_dist_curr =
                (read_encoder_counter() - encoder_prev_count) / TICK_PER_CM;
        } else { // 곡선 주행 중이라면
            is_straight = false;
        }
        straight_dist_total = straight_dist_accu + straight_dist_curr;
        if (straight_dist_total > STRAIGHT_DIST_CONDITION) { // 미션 탈출 조건
            beep(50);
            break;
        }
    }

    *fnClean = clean_roundabout;
    MSG("CLEAR MISSION => roundabout");
    init_drive(); // Init drive mission
}