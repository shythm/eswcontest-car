#ifndef _CONTROL_H_
#define _CONTROL_H_
#include "recognize-lib.h"

/*

    Missions

    - drive
    - overpass
    - roundabout
    - trafficLight
    - obstacle
    - tunnel
    - slope
    - parking
    - overtaking
*/

typedef void (*MissionFunction)();

typedef struct
{
    int priority;
    MissionFunction function;
} Mission;

typedef struct
{
    recog_result *input;
    struct
    {
        Mission drive;
        Mission overpass;
        Mission roundabout;
        Mission trafficLight;
        Mission obstacle;
        Mission tunnel;
        Mission slope;
        Mission parking;
        Mission overtaking;
    } missions;

    // Some custom variables
} State;

#define MISSION_FUNC_NAME(name) name
#define MISSION_CONDITION(name) \
    __attribute__((weak)) void MISSION_FUNC_NAME(name)(State * state) {}

MISSION_CONDITION(drive)
MISSION_CONDITION(overpass)
MISSION_CONDITION(roundabout)
MISSION_CONDITION(trafficLight)
MISSION_CONDITION(obstacle)
MISSION_CONDITION(tunnel)
MISSION_CONDITION(slope)
MISSION_CONDITION(parking)
MISSION_CONDITION(overtaking)

#undef MISSION_CONDITION

#endif