#ifndef DETECT_OBJECT_H_
#define DETECT_OBJECT_H_

#ifdef __cplusplus
extern "C" {
#endif

struct TrafficLights
{
    bool red, yellow, green, left, right;
};

struct Point
{
    int x;
    int y;
};

struct StopObstacle
{
    bool exist;
    struct Point center;
    float area;
};

// If outBuf is NULL, it doesn't draw the results.
struct TrafficLights detectLights(unsigned char* srcBuf, int sw, int sh, unsigned char* outBuf, int ow, int oh);
// If outBuf is NULL, it doesn't draw the results.
struct StopObstacle detectStopObstacle(unsigned char* srcBuf, int sw, int sh, unsigned char* outBuf, int ow, int oh);

#ifdef __cplusplus
}
#endif

#endif