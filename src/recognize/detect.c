#include "config-car.h"
#include "ctrlboard-lib.h"
#include "recognize-lib.h"
#include <stdbool.h>

#define IR_SENSOR_COUNT     7
#define IR_ACTIVE_THRESHOLD 5
bool get_is_on_stop_line(recog_arg *arg) {
    static ctrlboard_byte_container container;
    static uint8_t                  ir_active_cnt;

    // LSB가 left, 검은색이 1 흰색이 0 마지막 안쓰는 MSB는 0으로 고정
    if (comm_ctrlboard(arg->ctrl, MSG_ID_RECOGNIZE, CMD_LINE_SENSOR,
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

recog_is_on_lane_t get_is_on_lane(recog_arg *arg) {
    static ctrlboard_byte_container container;
    static uint8_t                  bitmask_left  = 0x7C; // 0111 1100 : left
    static uint8_t                  bitmask_right = 0x1F; // 0001 1111 : right
    static uint8_t                  bitmask_left_inv = 0x03; // 0000 0011 : left
    static uint8_t bitmask_right_inv = 0x60; // 0110 0000 : right

    if (comm_ctrlboard(arg->ctrl, MSG_ID_RECOGNIZE, CMD_LINE_SENSOR,
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