#include "ctrlboard-lib.h"
#include "recognize-lib.h"
#include "process.h"
#include <time.h>

// 회전교차로를 주행하는데 필요한 상수.
// 준호가 작성한 mission-drive에서 참고하였고,
// 원래 정의에서 MAX_VELO를 줄였다.
#define GAIN_P          14.4  // P gain of PID control
#define GAIN_I          0.00f // I gain of PID control
#define ANTI_WINDUP     500   // Anti windup of I error
#define MAX_VELO        75    // Maximum velocity
#define CURVE_DECEL     150   // The smaller this value, the more it slows down.
#define CURVE_THRESHOLD 25    // lane.value.position_with_white 값이 이 값 이상이라면 회전 교차로를 주행중이라고 판단.
                              // lane.value.position_with_white 값이 이 값 이상이다가, 다시 이 값보다 작은 상태가 일정 시간 이상 유지될 경우,
                              // 회전 교차로가 끝난 것으로 인식하고 미션을 종료함.

typedef ctrlboard_byte_container container;

void init_roundabout(State *state);
void check_roundabout(State *state);
void do_roundabout(State *state);
void go_forward(recog_result *input, float *error_sum, int *steering_result, short *velocity_result);

void init_roundabout(State *state) {
    state->input->is_on_stop_line.enabled = true;
}

void check_roundabout(State *state) {
    container data;

    if (state->input->is_on_stop_line.value) {
        data.c_int16 = 0;
        send_ctrlboard(state->ctrl, CMD_DESIRE_SPEED, 2, &data);
        state->missions.roundabout.priority = 2;
        state->missions.roundabout.function = do_roundabout;
    } else {
        state->missions.roundabout.priority = 0;
        state->missions.roundabout.function = NULL;
    }
}

void do_roundabout(State *state) {
    command(CMD_DESIRE_SPEED, 0);
    while (state->input->psd.value[PSD_FRONT] > 29)
        usleep(1000*500);
    
    int steering, velocity;
    float error_sum = 0;
    bool inCurve = false;
    clock_t exitedCurve = -1;

    while (exitedCurve == -1 || (clock() - exitedCurve) / CLOCKS_PER_SEC < 1) {
        if (state->input->psd.value[PSD_FRONT] < 29 || state->input->psd.value[PSD_LEFT_1] < 29)
            command(CMD_DESIRE_SPEED, 0);
        else
            go_forward(state->input, &error_sum, &steering, &velocity);
    
        if (steering < CURVE_THRESHOLD) { // 직선 주행 중이라면
            if (inCurve && exitedCurve == -1) {
                inCurve = false;
                exitedCurve = clock();
            }
        } else { // 곡선 주행 중이라면
            if (!inCurve) {
                inCurve = true;
                exitedCurve = -1;
            }
        }
    }

    fflush(stdout);
}

// 준호가 작성한 mission-drive.c의 do_drive에서 참고하였다.
void go_forward(recog_result *input, float *error_sum, int *steering_result, short *velocity_result) {
    int steering_val = 1500;
    // 기존의 do_drive에서는 노란색 차선의 위치(value.position)만 보았으나,
    // 회전 교차로에서는 흰색 정지선도 차선으로 인식해야 하므로 value.position_with_white를 확인한다.
    int pos     = input->lane.value.position_with_white;
    *error_sum += pos * GAIN_I;

    // Anti-windup
    *error_sum = MIN(*error_sum, ANTI_WINDUP);
    *error_sum = MAX(*error_sum, -ANTI_WINDUP);

    // PI-control with pos value and convert control value to steer value
    steering_val = 1500 + (short)(pos * GAIN_P + *error_sum);
    short velocity = (short)(MAX_VELO * CURVE_DECEL / (CURVE_DECEL + abs(pos)));

    // Limit steering range
    steering_val = MIN(steering_val, 2000);
    steering_val = MAX(steering_val, 1000);

    // Send steering value to hardware
    command(CMD_STEERING_SERVO_CONTROL, steering_val);
    // Send velocity to hardware
    command(CMD_DESIRE_SPEED, velocity);

    *steering_result = steering_val;
    *velocity_result = velocity;
}