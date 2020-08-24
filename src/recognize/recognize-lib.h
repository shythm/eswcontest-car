#ifndef _RECOGNIZE_LIB_H
#define _RECOGNIZE_LIB_H

#include <sys/ipc.h>
#include "util.h"

/******************************************************/
/* <START SECTION OF SHARED MEMORY>                   */
/******************************************************/
#include "recognize-update.h"

// these are for psd data
#define PSD_COUNT       6
#define PSD_FRONT       0
#define PSD_RIGHT_1     1
#define PSD_RIGHT_2     2
#define PSD_BACK        3
#define PSD_LEFT_2      4
#define PSD_LEFT_1      5
typedef float psd_data_t;
typedef struct _recog_psd_data {
    psd_data_t value[PSD_COUNT];
} recog_psd_data;

/* 
 * This is shared memory structure for the results of recognition processing.
 * You can add some fields to share the output of the method which you have been made.
 */
typedef struct _recog_result {
    recog_sample_data               sample;
    recog_is_on_stop_line_data      is_on_stop_line;
    recog_is_on_end_point_data      is_on_end_point;
    recog_traffic_light_data        traffic_light;
    recog_lane_data                 lane;
    recog_is_on_lane_data           is_on_lane;
    recog_is_on_slope_data          is_on_slope;
    recog_is_on_overpass_data       is_on_overpass;
    recog_is_in_tunnel_data         is_in_tunnel;
    recog_curr_velocity_data        curr_velocity;
    recog_stop_obstacle_data        stop_obstacle;
    recog_is_there_car_data         is_there_car;
    recog_psd_data                  psd;
} recog_result;

/******************************************************/
/* <END SECTION OF SHARED MEMORY>                     */
/******************************************************/

/* 
 * Initialize a shared memory of the recogition results.
 * In the other processes except for the recognize process, set init to zero(0).
 */
int get_shm_recog_result(key_t key, int* id, recog_result** rr, int init);

#endif /* _RECOGNIZE_LIB_H */