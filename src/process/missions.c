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

// a list of missions for the contest
fnInit_t mlist_contest[] = {
    init_start,   init_overpass,   init_obstacle,
    init_parking, init_parking,    init_roundabout,
    init_tunnel,  init_overtaking, init_trafficLight,
    NULL,
};

// a list of missions for practice
fnInit_t mlist_practice[] = {
    /*
     * Write the initializing functions here array to run the
     * missions sequentially. You must add NULL at the end element.
     */
    init_start,
    // init_overtaking,
    init_obstacle,
    // init_parking,
    NULL,
};