#include "control.h"
#include "recognize-lib.h"

#define CHECK(name)         \
    MISSION_FUNC_NAME(name) \
    (&state)

int main()
{
    recog_result *input;
    get_shm_recog_result(&input, 0);
    State state;
    state.input = input;
    int missionsCount = sizeof(state.missions) / sizeof(Mission);
    Mission *missions = (Mission *)(&state.missions);
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
        // Therefore, if priority of the overpass is 1 and drive is 0, the overpass will be executed.
        // However, if priority of both overpass and drive are 1, the drive will be executed.
        int maxPriority = -1;
        MissionFunction mission = NULL;
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
            return -1;
    }
    return 0;
}