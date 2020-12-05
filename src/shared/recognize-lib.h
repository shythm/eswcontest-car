#ifndef _RECOGNIZE_LIB_H
#define _RECOGNIZE_LIB_H

#include "ctrlboard-lib.h"
#include "util.h"
#include <stdbool.h>
#include <sys/ipc.h>

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

/*
 * This is source data for recognition. This is only for recognition
 * process.
 */
struct _recog_result;
typedef struct _recog_arg {
    mqid_ctrl             ctrl;
    unsigned char         camera_output[VPE_OUTPUT_IMG_SIZE];
    unsigned char *       display_input;
    struct _recog_result *shm_rr;
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
    bool        initialize;
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
recog_is_on_lane_t get_is_on_lane(recog_arg *arg);

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

typedef struct _recog_tl_lane_data {
    bool  enable;
    float value;
} recog_tl_lane_data;
float get_tl_lane(recog_arg *arg);

/******************************************************/
/* <END SECTION OF RECOGNITION RESULTS>               */
/******************************************************/

/******************************************************/
/* <START SECTION OF SHARED MEMORY>                   */
/******************************************************/

// these are for psd data
#define PSD_COUNT   6
#define PSD_FRONT   0
#define PSD_RIGHT_1 1
#define PSD_RIGHT_2 2
#define PSD_BACK    3
#define PSD_LEFT_2  4
#define PSD_LEFT_1  5
typedef float psd_data_t;
#define PSD_DISTANCE_MIN 4.0f
#define PSD_DISTANCE_MAX 30.0f
typedef struct _recog_psd_data {
    psd_data_t value[PSD_COUNT];
    bool       valid;
} recog_psd_data;

/*
 * This is shared memory structure for the results of recognition processing.
 * You can add some fields to share the output of the method which you have been
 * made.
 */
typedef struct _recog_result {
    recog_sample_data          sample;
    recog_is_on_stop_line_data is_on_stop_line;
    recog_is_on_end_point_data is_on_end_point;
    recog_traffic_light_data   traffic_light;
    recog_lane_data            lane;
    recog_is_on_lane_data      is_on_lane;
    recog_is_on_slope_data     is_on_slope;
    recog_is_on_overpass_data  is_on_overpass;
    recog_is_in_tunnel_data    is_in_tunnel;
    recog_curr_velocity_data   curr_velocity;
    recog_stop_obstacle_data   stop_obstacle;
    recog_is_there_car_data    is_there_car;
    recog_psd_data             psd;
    recog_tl_lane_data         tl_lane;
} recog_result;

/******************************************************/
/* <END SECTION OF SHARED MEMORY>                     */
/******************************************************/

/*
 * Initialize a shared memory of the recogition results.
 * In the other processes except for the recognize process, set init to zero(0).
 */
int get_shm_recog_result(recog_result **rr, int init);

#endif /* _RECOGNIZE_LIB_H */
