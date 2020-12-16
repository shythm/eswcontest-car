#include "process.h"

#define DECLARE_INIT_MISSION(name) void init_##name(fnCheck_t *fnCheck)

// weak functions
#define WEAK_FUNC_INIT_MISSION(name)                                           \
    __attribute__((weak)) DECLARE_INIT_MISSION(name) {}
WEAK_FUNC_INIT_MISSION(start);
WEAK_FUNC_INIT_MISSION(overpass);
WEAK_FUNC_INIT_MISSION(obstacle);
WEAK_FUNC_INIT_MISSION(parking);
WEAK_FUNC_INIT_MISSION(roundabout);
WEAK_FUNC_INIT_MISSION(tunnel);
WEAK_FUNC_INIT_MISSION(overtaking);
WEAK_FUNC_INIT_MISSION(trafficLight);

#ifndef MODE_PRACTICE

// a list of missions for the contest
fnInit_t mission_list[] = {
    init_start,   init_overpass,   init_obstacle,
    init_parking, init_parking,    init_roundabout,
    init_tunnel,  init_overtaking, init_trafficLight,
    NULL,
};

#else

// a list of missions for practice
fnInit_t mission_list[] = {
    /*
     * Write the initializing functions here array to run the
     * missions sequentially. You must add NULL at the end element.
     */
    init_start,   init_overpass, init_obstacle, init_parking,
    init_parking, init_tunnel,   NULL,
};

#endif