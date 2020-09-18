#include "recognize-update.h"
#include "ctrlboard-lib.h"

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

//받아온 container.uint8_t의 LSB가 제일 왼쪽, 흰색이 0.
bool get_is_on_stop_line(recog_arg *arg) {
    static ctrlboard_byte_container container;
    static uint8_t                  stop_line =
        0xC1; //이진수로 1100 0001임. 가운데 다섯개가 흰색인지..

    if (comm_ctrlboard(arg->ctrl, RECOG_ID_IS_ON_STOP_LINE, CMD_LINE_SENSOR,
                       CMD_TYPE_READ, 1, &container) == MSG_STATE_SUCCESS) {
        if (container.c_uint8 == stop_line) { return true; }
    } else {
        return false;
    }
}
/* END OF get_is_on_stop_line SECTION */

/* START OF get_is_on_end_point SECTION */
#define RECOG_ID_IS_ON_END_POINT 103L

bool get_is_on_end_point(recog_arg *arg) { return false; }
/* END OF get_is_on_end_point SECTION */

/* START OF get_traffic_light SECTION */
#define RECOG_ID_GET_TRAFFIC_LIGHT 104L

recog_traffic_light_t get_traffic_light(recog_arg *arg) { return TL_NONE; }
/* END OF get_traffic_light SECTION */

/* START OF get_lane SECTION */
#define RECOG_ID_GET_LANE 105L

vector_lane get_lane(recog_arg *arg) {
    static vector_lane result;

    return result;
}
/* END OF get_lane SECTION */

/* START OF is_on_lane SECTION */
#define RECOG_ID_IS_ON_LANE 106L

recog_is_on_lane_t get_is_on_lane(recog_arg *arg) {
    static ctrlboard_byte_container container;
    static uint8_t                  bitmask_left  = 0x01;
    static uint8_t                  bitmask_right = 0x40;
    if (comm_ctrlboard(arg->ctrl, RECOG_ID_IS_ON_LANE, CMD_LINE_SENSOR,
                       CMD_TYPE_READ, 1, &container) == MSG_STATE_SUCCESS) {
        if (container.c_uint8 & bitmask_left) {
            return ON_LANE_LEFT;
        } else if (container.c_uint8 & bitmask_right) {
            return ON_LANE_RIGHT;
        } else {
            return ON_LANE_NONE;
        }
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

    return result;
}
/* END OF stop_obstacle SECTION */

/* START OF is_there_car SECTION */
#define RECOG_GET_IS_THERE_CAR 112L

bool get_is_there_car(recog_arg *arg) { return false; }
/* END OF is_there_car SECTION */