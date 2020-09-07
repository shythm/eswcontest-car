#include "control.h"
#include "recognize-lib.h"

#define CHECK(name)         \
    MISSION_FUNC_NAME(name) \
    (&state)

#define MISSION_CONDITION(name)                    \
    /* A mission check function for mission name*/ \
    __attribute__((weak)) MISSION_CONDITION_DEF(name) {}

MISSION_CONDITION(drive)
MISSION_CONDITION(overpass)
MISSION_CONDITION(roundabout)
MISSION_CONDITION(trafficLight)
MISSION_CONDITION(obstacle)
MISSION_CONDITION(tunnel)
MISSION_CONDITION(slope)
MISSION_CONDITION(parking)
MISSION_CONDITION(overtaking)

int main()
{
    // Get shared memory
    recog_result *input;
    get_shm_recog_result(&input, 0);

    // Initialize state
    State state = {0};
    state.input = input;

    // Initialize mission-associated variables
    int missionsCount = sizeof(state.missions) / sizeof(Mission),
        maxPriority;

    // Check if missions structrue is contaminated.
    if (sizeof(state.missions) % sizeof(Mission) != 0)
    {
        printf("Mission conversion error occurred.\n");
        return -1;
    }
    Mission *missions = (Mission *)(&state.missions);
    MissionFunction mission;
    while (1)
    {
        CHECK(drive);
        CHECK(overpass);
        CHECK(roundabout);
        CHECK(trafficLight);
        CHECK(obstacle);
        CHECK(tunnel);
        CHECK(slope);
        CHECK(parking);
        CHECK(overtaking);

        // Functions are executed in the order of highest priority.
        // Among the same priorities, they are executed in the order of checking first.
        // For example, if priority of the overpass is 1 and drive is 0, the overpass will be executed.
        // However, if priority of both overpass and drive are 1, the drive will be executed.
        // Therefore, if no mission has selected, drive will be executed by default.
        maxPriority = -1;
        mission = NULL;
        for (int i = 0; i < missionsCount; i++)
        {
            if (missions[i].priority > maxPriority)
            {
                maxPriority = missions[i].priority;
                mission = missions[i].function;
                missions[i].priority = 0;
            }
        }
        if (mission)
            mission();
        // No mission has a priority of 1 or higher.
        else
            return -2;
    }
    return 0;
}