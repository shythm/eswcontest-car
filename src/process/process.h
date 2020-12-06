#ifndef _PROCESS_H_
#define _PROCESS_H_
#include "config-car.h"
#include "ctrlboard-lib.h"
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

typedef struct {
    int             priority;
    MissionFunction function;
} Mission;

typedef struct {
    recog_result *input;
    mqid_ctrl     ctrl;
    struct {
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

#define MISSION_FUNC_NAME(name)      check_##name
#define MISSION_INIT_FUNC_NAME(name) init_##name
#define MISSION_CONDITION_DEF(name)  void MISSION_FUNC_NAME(name)(State * state)
#define MISSION_INIT_DEF(name)       void MISSION_INIT_FUNC_NAME(name)(State * state)

MISSION_CONDITION_DEF(drive);
MISSION_CONDITION_DEF(overpass);
MISSION_CONDITION_DEF(roundabout);
MISSION_CONDITION_DEF(trafficLight);
MISSION_CONDITION_DEF(obstacle);
MISSION_CONDITION_DEF(tunnel);
MISSION_CONDITION_DEF(slope);
MISSION_CONDITION_DEF(parking);
MISSION_CONDITION_DEF(overtaking);

MISSION_INIT_DEF(drive);
MISSION_INIT_DEF(overpass);
MISSION_INIT_DEF(roundabout);
MISSION_INIT_DEF(trafficLight);
MISSION_INIT_DEF(obstacle);
MISSION_INIT_DEF(tunnel);
MISSION_INIT_DEF(slope);
MISSION_INIT_DEF(parking);
MISSION_INIT_DEF(overtaking);

ctrlboard_msg_state_t command(ctrlboard_cmd_code cmd, int data);

#endif