#ifndef _LANE_DETECTION
#define _LANE_DETECTION
#include "recognize-update.h"

#ifdef __cplusplus
extern "C" {
#endif

void detectLane(recog_arg *arg, vector_lane *result);

#ifdef __cplusplus
}
#endif
#endif