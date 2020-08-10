#ifndef _CARSHM_H
#define _CARSHM_H

#include <stdbool.h>

/*
 * 
 */
enum traffic_light_t {
    TL_NONE,
    TL_RED,
    TL_YELLOW,
    TL_GREEN,
    TL_LEFT
};

/*
 *
 */
enum lane_t {
    LANE_NONE,
    LANE_LEFT,
    LANE_RIGHT,
};

/*
 *
 */
enum stop_obstacle_t {
    SO_NONE,
    SO_EXIST_FAR,
    SO_EXIST_NEAR
};

/* 
 * This is shared memory structure for the results of recognition processing.
 * You can add some fields to share the output of the function which you have been made.
 * Also, you are only allowed to read this structure, NOT TO WRITE.
 */
typedef struct _recognition_result {
    bool is_on_stop_line;
    bool is_on_end_point;
    enum traffic_light_t traffic_light_value;
    float left_lane_pos;                       // unit: none
    float left_lane_curv;                      // unit: none
    float right_lane_pos;                      // unit: none
    float right_lane_curv;                     // unit: none
    enum lane_t is_on_lane;
    bool is_on_overpass;
    bool is_on_slope;
    float curr_velocity;                       // unit: cm/s
    enum stop_obstacle_t is_there_stop_obstacle;
    bool is_in_tunnel;
    bool is_there_car;
} recognition_result;

#endif /* _CARSHM_H */
