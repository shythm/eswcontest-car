#ifndef _RECOGNIZE_SHM_H
#define _RECOGNIZE_SHM_H

#include <stdbool.h>

/*******************************************************/
/* <START SECTION OF RECOGNITION RESULTS>              */
/* THESE STRUCTURE MUST BE CONTAIN EANBLED FIELD AND   */
/* RESULT VALUES                                       */
/*******************************************************/

// If the car is on end zone, it will be stored true value. Else, false.
typedef struct {
    bool enabled;
    bool value;
} recog_is_on_end_zone_data;

// It will be stored traffic light type.
typedef enum {
    TL_NONE   = 0b0000,
    TL_RED    = 0b0001,
    TL_YELLOW = 0b0010,
    TL_GREEN  = 0b0100,
    TL_LEFT   = 0b1000,
} recog_traffic_light_t;
typedef struct {
    bool                  enabled;
    recog_traffic_light_t value;
} recog_traffic_light_data;

// It will be stored lane position.
typedef struct {
    float pos_yl;   // position of yellow lane
    float pos_yawl; // position of yellow and white lane
} vector_lane;
typedef struct {
    bool        enabled;
    vector_lane value;
} recog_lane_data;

// If there is uphill, it will be stored SLOPE_UPHILL value.
// If there is downhill, it will be sotred SLOPE_DOWNHILL value.
typedef enum {
    SLOPE_NONE = 0,
    SLOPE_UPHILL,
    SLOPE_DOWNHILL,
} recog_slope_t;
typedef struct {
    bool          enabled;
    recog_slope_t value;
} recog_slope_data;

// It will be stored position(x, y) and area of the stop obstacle.
typedef struct {
    int   pos_x;
    int   pos_y;
    float area;
} recog_stop_obstacle_t;
typedef struct {
    bool                  enabled;
    recog_stop_obstacle_t value;
} recog_stop_obstacle_data;

// These are for psd data
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

// It will be stored lane position only for traffic light mission.
typedef struct {
    bool  enabled;
    float value;
} recog_tl_lane_data;

// It will be stored the empty road information.
typedef struct {
    bool enabled;
    int  value; // -1: left, 1: right is empty road
} recog_empty_road_data;

// It will be stored position of the stop line [0 ~ 1].
// If there isn't stop line, it will be stored -1.
typedef struct {
    bool  enabled;
    float value;
} recog_stop_line_pos_data;

/******************************************************/
/* <END SECTION OF RECOGNITION RESULTS>               */
/******************************************************/

/******************************************************/
/* <START SECTION OF SHARED MEMORY>                   */
/******************************************************/

typedef volatile struct { // for external access of recognize process
    bool call_init_lane_info;
} external_data;

/*
 * This is shared memory structure for the results of recognition processing.
 * You can add some fields to share the output of the method which you have been
 * made.
 */
typedef volatile struct {
    recog_is_on_end_zone_data is_on_end_zone;
    recog_traffic_light_data  traffic_light;
    recog_lane_data           lane;
    recog_slope_data          slope;
    recog_stop_obstacle_data  stop_obstacle;
    recog_psd_data            psd;
    recog_tl_lane_data        tl_lane;
    recog_empty_road_data     empty_road;
    recog_stop_line_pos_data  stop_line_pos;
    external_data             ext_data;
} recog_result;

/******************************************************/
/* <END SECTION OF SHARED MEMORY>                     */
/******************************************************/

/*
 * Initialize a shared memory of the recogition results.
 * In the other processes except for the recognize process, set init to zero(0).
 */
int get_shm_recog_result(recog_result **rr, int init);

#endif /* _RECOGNIZE_SHM_H */
