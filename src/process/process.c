#include "process.h"
#include "ctrlboard-lib.h"
#include "recognize-lib.h"
#include <sys/msg.h>

#define CHECK(name)                                                            \
    MISSION_FUNC_NAME(name)                                                    \
    (&state)

#define INIT(name)                                                             \
    MISSION_INIT_FUNC_NAME(name)                                               \
    (&state)

#define MISSION_CONDITION(name)                                                \
    /* A mission check function for mission name*/                             \
    __attribute__((weak)) MISSION_CONDITION_DEF(name) {}

#define MISSION_INIT(name)                                                     \
    /* A mission check function for mission name*/                             \
    __attribute__((weak)) MISSION_INIT_DEF(name) {}

MISSION_CONDITION(drive)
MISSION_CONDITION(overpass)
MISSION_CONDITION(roundabout)
MISSION_CONDITION(trafficLight)
MISSION_CONDITION(obstacle)
MISSION_CONDITION(tunnel)
MISSION_CONDITION(slope)
MISSION_CONDITION(parking)
MISSION_CONDITION(overtaking)

MISSION_INIT(drive)
MISSION_INIT(overpass)
MISSION_INIT(roundabout)
MISSION_INIT(trafficLight)
MISSION_INIT(obstacle)
MISSION_INIT(tunnel)
MISSION_INIT(slope)
MISSION_INIT(parking)
MISSION_INIT(overtaking)

int msgq_id;
int main() {
    usleep(1000 * 1000);

    // Get shared memory
    recog_result *input;
    get_shm_recog_result(&input, 0);

    // Get message queue
    get_msgq_id_ctrlboard(&msgq_id, 0);

    // Initialize state
    State state   = {0};
    state.input   = input;
    state.msgq_id = msgq_id;

    // Initialize mission-associated variables
    int missionsCount = sizeof(state.missions) / sizeof(Mission), maxPriority;

    // Check if missions structrue is contaminated.
    if (sizeof(state.missions) % sizeof(Mission) != 0) {
        printf("Mission conversion error occurred.\n");
        return -1;
    }

    INIT(drive);
    INIT(overpass);
    INIT(roundabout);
    INIT(trafficLight);
    INIT(obstacle);
    INIT(tunnel);
    INIT(slope);
    INIT(parking);
    INIT(overtaking);

    printf("Start process while loop\n");
    Mission *       missions = (Mission *)(&state.missions);
    MissionFunction mission;
    while (1) {
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
        // Among the same priorities, they are executed in the order of checking
        // first. For example, if priority of the overpass is 1 and drive is 0,
        // the overpass will be executed. However, if priority of both overpass
        // and drive are 1, the drive will be executed. Therefore, if no mission
        // has selected, drive will be executed by default.
        maxPriority = -1;
        mission     = NULL;
        for (int i = 0; i < missionsCount; i++) {
            if (missions[i].priority > maxPriority) {
                maxPriority          = missions[i].priority;
                mission              = missions[i].function;
                missions[i].priority = 0;
            }
        }
        if (mission) mission();
        else {
            printf(
                "No mission has a priority of 1 or higher.\nExit process.\n");
            return -2;
        }
    }
    return 0;
}

// There is no value to be read from the hardware in the processing stage.
// Therefore, there is no need to do receive check.
// If there is a value to be read from the hardware, the task must be done in
// 'recognize' step.
int ctrl_msgq(ctrlboard_cmd_code code, unsigned char bytec,
              ctrlboard_byte_container *data) {
    static size_t size = sizeof(ctrlboard_msg) - sizeof(long);
    ctrlboard_msg msg;

    // set message information
    msg.msgid     = MSGQ_ID_PROCESS;
    msg.cmd.code  = code;
    msg.cmd.rw    = CMD_TYPE_WRITE;
    msg.cmd.bytec = bytec;
    msg.state     = MSG_STATE_UNKNOWN_ERR;
    memcpy(msg.data.bytes, data->bytes, msg.cmd.bytec);
    if (msgsnd(msgq_id, &msg, size, 0)) {
        printf("ERR1\n");
        return MSG_STATE_SEND_ERR;
    }
    if (msgrcv(msgq_id, &msg, size, msg.msgid, 0) < 0) {
        printf("ERR2\n");
        return MSG_STATE_RECEIVE_ERR;
    }
    return MSG_STATE_SUCCESS;
}
