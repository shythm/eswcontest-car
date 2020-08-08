#ifndef _CARMMAP_H
#define _CARMMAP_H

#include <stdbool.h>
#include "carprop.h"

/*
 * This is memory map structure for the output of the camera.
 * Camera manager should fill this structure.
 */
struct camera_mmap
{
    unsigned char data[3 * CAMERA_OUTPUT_H * CAMERA_OUTPUT_W];
};

/*
 * memory map structure for display data
 */
struct display_mmap {
    unsigned char* buf;
    // unsigned char input[3 * DISPLAY_W * DISPLAY_H];
};

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
    SO_NOME,
    SO_EXIST_FAR,
    SO_EXIST_NEAR
};

/* 
 * This is the memory map structure for results of recognition processing.
 * You can add some fields to share the output of the function which you have been made.
 * Also, you are only allowed to read this structure, NOT TO WRITE.
 */
struct recognition_result
{
    bool is_on_stop_line;
    bool is_on_end_drive;
    double psd_value[6];                        // unit: cm
    enum traffic_light_t traffic_light_value;
    double left_lane_pos;                       // unit: none
    double left_lane_curv;                      // unit: none
    double right_lane_pos;                      // unit: none
    double right_lane_curv;                     // unit: none
    enum lane_t is_on_lane;
    bool is_on_overpass;
    bool is_on_slope;
    double delta_slope;                         // unit: degree
    double curr_velocity;                       // unit: cm/s
    bool is_in_parking_mission;
    enum stop_obstacle_t is_there_stop_obstacle;
    bool is_in_tunnel;
    int relative_road_brightness;               // unit: none
    bool is_there_car;
};

#endif /* _CARMMAP_H */
