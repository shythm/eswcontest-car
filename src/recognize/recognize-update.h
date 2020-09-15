#ifndef _RECOGNIZE_UPDATE_H
#define _RECOGNIZE_UPDATE_H

#include "util.h"
#include <stdbool.h>

#define CAPTURE_IMG_W      1280
#define CAPTURE_IMG_H      720
#define CAPTURE_IMG_SIZE   (CAPTURE_IMG_W * CAPTURE_IMG_H * 2) // YUYU : 16bpp
#define CAPTURE_IMG_FORMAT "uyvy"

#define VPE_OUTPUT_W        320
#define VPE_OUTPUT_H        180
#define VPE_OUTPUT_IMG_SIZE (VPE_OUTPUT_W * VPE_OUTPUT_H * 3)
#define VPE_OUTPUT_FORMAT   "bgr24"

#define OVERLAY_DISP_FORCC FOURCC('A', 'R', '2', '4')
#define OVERLAY_DISP_W     480
#define OVERLAY_DISP_H     272

#include "ctrlboard-lib.h"

/*
 * This is source data for recognition. This is only for recognition
 * process.
 */
typedef struct _recog_arg {
    mqid_ctrl      ctrl;
    unsigned char  camera_output[VPE_OUTPUT_IMG_SIZE];
    unsigned char *display_input;
} recog_arg;

/******************************************************/
/* <START SECTION OF RECOGNITION RESULTS>             */
/* THESE STRUCTURE MUST BE CONTAIN EANBLE FIELD AND   */
/* RESULT VALUES                                      */
/******************************************************/

// for sample data
#define SAMPLE_COUNT 2
typedef struct _recog_sample_data {
    bool          enabled;
    unsigned char value[SAMPLE_COUNT];
} recog_sample_data;
unsigned char *get_sample(recog_arg *arg);

// for is_on_stop_line data
typedef struct _recog_is_on_stop_line_data {
    bool enabled;
    bool value;
} recog_is_on_stop_line_data;
bool get_is_on_stop_line(recog_arg *arg);

// for is_on_end_point data
typedef struct _recog_is_on_end_point_data {
    bool enabled;
    bool value;
} recog_is_on_end_point_data;
bool get_is_on_end_point(recog_arg *arg);

// for traffic_light data
typedef enum _recog_traffic_light_t {
    TL_NONE,
    TL_RED,
    TL_YELLOW,
    TL_GREEN,
    TL_LEFT,
} recog_traffic_light_t;
typedef struct _recog_traffic_light_data {
    bool                  enabled;
    recog_traffic_light_t value;
} recog_traffic_light_data;
recog_traffic_light_t get_traffic_light(recog_arg *arg);

// for lane data
typedef struct _vector_lane {
    float left_pos;
    float left_curv;
    float right_pos;
    float right_curv;
    float position;
} vector_lane;
typedef struct _recog_lane_data {
    bool        enabled;
    vector_lane value;
} recog_lane_data;
vector_lane get_lane(recog_arg *arg);

// for is_on_lane data
typedef enum _recog_is_on_lane_t {
    ON_LANE_NONE,
    ON_LANE_LEFT,
    ON_LANE_RIGHT,
} recog_is_on_lane_t;
typedef struct _recog_is_on_lane_data {
    bool               enabled;
    recog_is_on_lane_t value;
} recog_is_on_lane_data;
bool get_is_on_lane(recog_arg *arg);

// for is_on_slope data
typedef struct _recog_is_on_slope_data {
    bool enabled;
    bool value;
} recog_is_on_slope_data;
bool get_is_on_slope(recog_arg *arg);

// for is_on_overpass data
typedef struct _recog_is_on_overpass_data {
    bool enabled;
    bool value;
} recog_is_on_overpass_data;
bool get_is_on_overpass(recog_arg *arg);

// for is_in_tunnel data
typedef struct _recog_is_in_tunnel_data {
    bool enabled;
    bool value;
} recog_is_in_tunnel_data;
bool get_is_in_tunnel(recog_arg *arg);

// for curr_velocity data
typedef struct _recog_curr_velocity_data {
    bool  enabled;
    float value;
} recog_curr_velocity_data;
float get_curr_velocity(recog_arg *arg);

// for stop_obstacle data
typedef struct _recog_stop_obstacle_t {
    int   pos_x;
    int   pos_y;
    float area;
} recog_stop_obstacle_t;
typedef struct _recog_stop_obstacle_data {
    bool                  enabled;
    recog_stop_obstacle_t value;
} recog_stop_obstacle_data;
recog_stop_obstacle_t get_stop_obstacle(recog_arg *arg);

// for is_there_car data
typedef struct _recog_is_there_car_data {
    bool enabled;
    bool value;
} recog_is_there_car_data;
bool get_is_there_car(recog_arg *arg);

/******************************************************/
/* <END SECTION OF RECOGNITION RESULTS>               */
/******************************************************/

#endif /* _RECOGNIZE_UPDATE_H */
