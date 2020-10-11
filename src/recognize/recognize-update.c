#include "ctrlboard-lib.h"
#include "recognize-lib.h"

/* START OF get_sample SECTION */
#define RECOG_ID_GET_SAMPLE 101L

unsigned char *get_sample(recog_arg *arg) {
    static unsigned char            result[SAMPLE_COUNT]; // for storing result
    static ctrlboard_byte_container container;

    // checking for first function call
    static bool init = false;
    if (!init) {
        result[0] = 0;
        result[1] = 0;
        init      = true;
    }

    result[0] += arg->camera_output[0]; // example of using camera output data
    // example of using ctrlboard
    comm_ctrlboard(arg->ctrl, RECOG_ID_GET_SAMPLE, CMD_SPEED_PID_PROPORTIONAL,
                   CMD_TYPE_READ, 1, &container);
    result[1] = container.c_uint8;

    return result;
}
/* END OF get_sample SECTION */

/* START OF get_is_on_stop_line SECTION */
#define RECOG_ID_IS_ON_STOP_LINE 102L
#define IR_SENSOR_COUNT          7
#define IR_ACTIVE_THRESHOLD      5
// container.c_uint8의 LSB가 left, 검은색이 1 흰색이 0 마지막 안쓰는 MSB는 0으로
// 고정
bool get_is_on_stop_line(recog_arg *arg) {
    static ctrlboard_byte_container container;
    static uint8_t                  ir_active_cnt;

    if (comm_ctrlboard(arg->ctrl, RECOG_ID_IS_ON_STOP_LINE, CMD_LINE_SENSOR,
                       CMD_TYPE_READ, 1, &container) == MSG_STATE_SUCCESS) {
        ir_active_cnt = 0;
        for (int i = 0; i < IR_SENSOR_COUNT; i++) {
            if (!(container.c_uint8 & (0x01 << i))) ir_active_cnt++;
        }

        if (ir_active_cnt >= IR_ACTIVE_THRESHOLD) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}
/* END OF get_is_on_stop_line SECTION */

/* START OF get_is_on_end_point SECTION */
#define RECOG_ID_IS_ON_END_POINT 103L
#include "detect-end-zone.h"
bool get_is_on_end_point(recog_arg *arg) { return detectEndZone(arg); }
/* END OF get_is_on_end_point SECTION */

/* START OF get_traffic_light SECTION */
#define RECOG_ID_GET_TRAFFIC_LIGHT 104L
#include "detect-object.h"

recog_traffic_light_t get_traffic_light(recog_arg *arg) {
    struct TrafficLights detected =
        detectLights(arg->camera_output, VPE_OUTPUT_W, VPE_OUTPUT_H,
                     arg->display_input, VPE_OUTPUT_W, VPE_OUTPUT_H);

    if (detected.green) return TL_GREEN;
    if (detected.yellow) return TL_YELLOW;
    if (detected.left) return TL_LEFT;
    if (detected.red) return TL_RED;

    return TL_NONE;
}
/* END OF get_traffic_light SECTION */

/* START OF get_lane SECTION */
#define RECOG_ID_GET_LANE 105L
#include "lane-detection.h"
vector_lane get_lane(recog_arg *arg) {
    static vector_lane              result;
    static ctrlboard_byte_container container;
    detectLane(arg, &result);
    return result;
}
/* END OF get_lane SECTION */

/* START OF is_on_lane SECTION */
#define RECOG_ID_IS_ON_LANE 106L

recog_is_on_lane_t get_is_on_lane(recog_arg *arg) {
    static ctrlboard_byte_container container;
    static uint8_t                  bitmask_left  = 0x7C; // 0111 1100 : left
    static uint8_t                  bitmask_right = 0x1F; // 0001 1111 : right
    static uint8_t                  bitmask_left_inv = 0x03; // 0000 0011 : left
    static uint8_t bitmask_right_inv = 0x60; // 0110 0000 : right

    if (comm_ctrlboard(arg->ctrl, RECOG_ID_IS_ON_LANE, CMD_LINE_SENSOR,
                       CMD_TYPE_READ, 1, &container) == MSG_STATE_SUCCESS) {
        if ((~container.c_uint8) & bitmask_left_inv) {
            return ON_LANE_LEFT;
        } else if ((~container.c_uint8) & bitmask_right_inv) {
            return ON_LANE_RIGHT;
        } else {
            // printf("%d,%d\n", container.c_uint8, bitmask_right);
            return ON_LANE_NONE;
        }
    } else {
        return ON_LANE_NONE;
    }
}
/* END OF is_on_lane SECTION */

/* START OF is_on_slope SECTION */
#define RECOG_ID_IS_ON_SLOPE 107L

bool get_is_on_slope(recog_arg *arg) { return false; }
/* END OF is_on_slope SECTION */

/* START OF is_on_overpass SECTION */
#define RECOG_ID_IS_ON_OVERPASS 108L

bool get_is_on_overpass(recog_arg *arg) { return false; }
/* END OF is_on_overpass SECTION */

/* START OF is_in_tunnel SECTION */
#define RECOG_ID_IS_IN_TUNNEL 109L

bool get_is_in_tunnel(recog_arg *arg) { return false; }
/* END OF is_in_tunnel SECTION */

/* START OF curr_velocity SECTION */
#define RECOG_GET_CURR_VELOCITY 110L

float get_curr_velocity(recog_arg *arg) { return 0.0f; }
/* END OF curr_velocity SECTION */

/* START OF stop_obstacle SECTION */
#define RECOG_GET_STOP_OBSTACLE 111L

recog_stop_obstacle_t get_stop_obstacle(recog_arg *arg) {
    static recog_stop_obstacle_t result;

    struct StopObstacle detected =
        detectStopObstacle(arg->camera_output, VPE_OUTPUT_W, VPE_OUTPUT_H,
                           arg->display_input, VPE_OUTPUT_W, VPE_OUTPUT_H);
    if (detected.exist) {
        result.area  = detected.area;
        result.pos_x = detected.center.x;
        result.pos_y = detected.center.y;
    } else {
        result.area  = 0;
        result.pos_x = -1;
        result.pos_y = -1;
    }

    return result;
}
/* END OF stop_obstacle SECTION */

/* START OF is_there_car SECTION */
#define RECOG_GET_IS_THERE_CAR 112L
#include "detect-is-there-car.h"

vector_car_pos get_is_there_car(recog_arg *arg) {
    return detectIsThereCar(arg);
}
/* END OF is_there_car SECTION */