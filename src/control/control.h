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

#define MSGQ_ID_CONTROL 299

typedef void (*MissionFunction)();

typedef struct
{
    int priority;
    MissionFunction function;
} Mission;

typedef struct
{
    recog_result *input;
    int msgq_id;
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

#define MISSION_FUNC_NAME(name) check_##name
#define MISSION_CONDITION_DEF(name) void MISSION_FUNC_NAME(name)(State * state)

MISSION_CONDITION_DEF(drive);
MISSION_CONDITION_DEF(overpass);
MISSION_CONDITION_DEF(roundabout);
MISSION_CONDITION_DEF(trafficLight);
MISSION_CONDITION_DEF(obstacle);
MISSION_CONDITION_DEF(tunnel);
MISSION_CONDITION_DEF(slope);
MISSION_CONDITION_DEF(parking);
MISSION_CONDITION_DEF(overtaking);

#endif