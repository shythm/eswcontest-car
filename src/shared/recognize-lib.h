#ifndef _RECOGNIZE_LIB_H
#define _RECOGNIZE_LIB_H

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

typedef volatile struct { bool call_init_lane_info; } external_data;

typedef struct { // only for recognize process
    // mqid_ctrl      ctrl;
    external_data *pext_data;
    unsigned char  camera_output[VPE_OUTPUT_IMG_SIZE];
    unsigned char *display_input;
} recog_arg;

/******************************************************/
/* <START SECTION OF RECOGNITION RESULTS>             */
/* THESE STRUCTURE MUST BE CONTAIN EANBLE FIELD AND   */
/* RESULT VALUES                                      */
/******************************************************/

// for is_on_stop_line data
typedef struct {
    bool enabled;
    bool value;
} recog_is_on_stop_line_data;

// for is_on_end_zone data
typedef struct {
    bool enabled;
    bool value;
} recog_is_on_end_zone_data;

// for traffic_light data
typedef enum {
    TL_NONE,
    TL_RED,
    TL_YELLOW,
    TL_GREEN,
    TL_LEFT,
} recog_traffic_light_t;
typedef struct {
    bool                  enabled;
    recog_traffic_light_t value;
} recog_traffic_light_data;

// for lane data
typedef struct {
    float pos_yl;   // position of yellow lane
    float pos_yawl; // position of yellow and white lane
} vector_lane;
typedef struct {
    bool        enabled;
    vector_lane value;
} recog_lane_data;

// for is_on_lane data
typedef enum {
    ON_LANE_NONE,
    ON_LANE_LEFT,
    ON_LANE_RIGHT,
} recog_is_on_lane_t;
typedef struct {
    bool               enabled;
    recog_is_on_lane_t value;
} recog_is_on_lane_data;

// for is_on_slope data
typedef struct {
    bool enabled;
    bool value;
} recog_is_on_slope_data;

// for is_in_tunnel data
typedef struct {
    bool enabled;
    bool value;
} recog_is_in_tunnel_data;

// for stop_obstacle data
typedef struct {
    int   pos_x;
    int   pos_y;
    float area;
} recog_stop_obstacle_t;
typedef struct {
    bool                  enabled;
    recog_stop_obstacle_t value;
} recog_stop_obstacle_data;

// for is_there_car data
typedef enum {
    TC_NONE,
    TC_LEFT,
    TC_CENTER,
    TC_RIGHT,
} recog_is_there_car_t;
typedef struct {
    bool                 enabled;
    recog_is_there_car_t value;
} recog_is_there_car_data;

// these are for psd data
#define PSD_COUNT        6
#define PSD_FRONT        0
#define PSD_RIGHT_1      1
#define PSD_RIGHT_2      2
#define PSD_BACK         3
#define PSD_LEFT_2       4
#define PSD_LEFT_1       5
#define PSD_DISTANCE_MIN 4.0f
#define PSD_DISTANCE_MAX 30.0f
typedef float psd_data_t;
typedef struct {
    psd_data_t value[PSD_COUNT];
    bool       valid;
} recog_psd_data;

typedef struct _recog_tl_lane_data {
    bool  enable;
    float value;
} recog_tl_lane_data;

/******************************************************/
/* <END SECTION OF RECOGNITION RESULTS>               */
/******************************************************/

/******************************************************/
/* <START SECTION OF SHARED MEMORY>                   */
/******************************************************/

/*
 * This is shared memory structure for the results of recognition processing.
 * You can add some fields to share the output of the method which you have been
 * made.
 */
typedef volatile struct {
    recog_is_on_stop_line_data is_on_stop_line;
    recog_is_on_end_zone_data  is_on_end_zone;
    recog_traffic_light_data   traffic_light;
    recog_lane_data            lane;
    recog_is_on_lane_data      is_on_lane;
    recog_is_on_slope_data     is_on_slope;
    recog_is_in_tunnel_data    is_in_tunnel;
    recog_stop_obstacle_data   stop_obstacle;
    recog_is_there_car_data    is_there_car;
    recog_psd_data             psd;
    recog_tl_lane_data         tl_lane;
    external_data              ext_data;
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
