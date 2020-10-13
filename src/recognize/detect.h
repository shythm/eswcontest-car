#ifndef _DETECT_H
#define _DETECT_H

#include "ctrlboard-lib.h"
#include "recognize-lib.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct TrafficLights {
    bool red, yellow, green, left, right;
};
struct Point {
    int x;
    int y;
};
struct StopObstacle {
    bool         exist;
    struct Point center;
    float        area;
};

struct TrafficLights detectLights(recog_arg *arg);
struct StopObstacle  detectStopObstacle(recog_arg *arg);
bool                 detectSlope(recog_arg *arg);
void                 detectLane(recog_arg *arg, vector_lane *result);
bool                 detectEndZone(recog_arg *arg);

#ifdef __cplusplus
}
#endif

#endif