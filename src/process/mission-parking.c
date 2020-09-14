#include "ctrlboard-lib.h"
#include "process.h"
#include "recognize-lib.h"

#define TICK_PER_CM 19.7628f
#define RADIUS      37.00f
#define PI          3.141592f

void position_control_on();
void speed_control_on();
int  read_encoder_counter(int);
void set_desire_encoder_count(int);
void set_steering(short steering);
void set_encoder_counter(int encoder_counter);
void set_desire_speed(short);
void do_parking_vertical(State *state);
void do_parking_parallel(State *state);
void beep(char time);

typedef ctrlboard_byte_container container;

recog_result *rr;
int           parking_complete = 0;
long          msgid            = 'p' + 'a' + 'r' + 'k';

void init_parking(State *state) {
    container data;
    int       ret = get_shm_recog_result(&rr, 0);
    if (ret == -1) printf("failt to get shm_recog_result\n");
    state->missions.drive.priority = 0;
    // set_steering(2000);
    // beep(100);
    // position_control_on();
    // set_desire_encoder_count(1000);
}

int encoder_count1 = 0, encoder_count2 = 0;
/*
           ________________
          |               |
__________|    parking    |__________
|         |     area      |         |
|_________|               |_________|
          ^               ^
          |               |
          |               encoder_count1
          encoder_count2

<------- progress direction of car

*/

void check_parking(State *state) {
    // beep(50);
    // return;

    static enum _parking_state { NONE, READY, DECISION } parking_state = 0;

    container data;
    // printf("PSD_RIGHT_2: %f\n", (float)(rr->psd.value[PSD_RIGHT_2]));

    switch (parking_state) {
    case NONE: { // no obstacle sensed
        state->missions.drive.priority = 0;
        if (parking_complete >= 2) break;
        if ((rr)->psd.value[PSD_RIGHT_2] < 26.f) parking_state = READY;
        break;
    }
    case READY: { // first obstacle sensed

        state->missions.drive.priority = 0;
        if ((rr)->psd.value[PSD_RIGHT_2] > 29.f) {
            encoder_count1 =
                read_encoder_counter(state->msgq_id); // parking area start
            set_desire_speed(50);
            parking_state = DECISION;
        }
        break;
    }
    case DECISION: { // parking area sensed

        state->missions.drive.priority = 0;
        if ((rr)->psd.value[PSD_RIGHT_2] < 26.f) {
            int distance = read_encoder_counter(state->msgq_id) -
                           encoder_count1; // width of parking area [tick]
            if (25.f * TICK_PER_CM < distance &&
                distance < 45.f * TICK_PER_CM) {
                set_desire_speed(0);
                beep(50);
                // parking vertical
                state->missions.drive.priority = 10;
                state->missions.drive.function = do_parking_vertical;

            } else if (45.f * TICK_PER_CM < distance &&
                       distance < 56.f * TICK_PER_CM) {
                // parking parallel
                state->missions.drive.priority = 10;
                state->missions.drive.function = do_parking_parallel;
            } // else: here isn't parking area.
            parking_state = NONE;
        }
        break;
    }
    default:
        state->missions.drive.priority = 0;
        parking_state                  = NONE;
        break;
    }
}

void do_parking_vertical(State *state) {
    int so_enable = (rr)->stop_obstacle.enabled;
    int tl_enable = (rr)->traffic_light.enabled;

    (rr)->stop_obstacle.enabled = false;
    (rr)->traffic_light.enabled = false;

    container data;

    int   distance       = 0; // [tick]
    int   desire_encoder = 0;
    short steering       = 1500;

    // position_control_on();
    set_encoder_counter(0);
    // set_desire_encoder_count(0);
    set_steering(1500);
    sleep(1);

    // regress
    set_desire_encoder_count(-1000);
    set_desire_speed(-50);
    while (1) {
        if (rr->psd.value[PSD_RIGHT_2] < 26.f) break;
        usleep(1000);
    }
    // while (1) {
    //     if (rr->psd.value[PSD_RIGHT_2] > 29.f) break;
    //     usleep(1000);
    // }
    set_desire_speed(0);
    // set_desire_encoder_count(0);
    sleep(1);

    // progress
    // set_encoder_counter(0);
    // desire_encoder = 10.f * TICK_PER_CM;
    // set_desire_speed(50);
    // // set_desire_encoder_count(14.7f * TICK_PER_CM);
    // while (1) {
    //     if (desire_encoder < read_encoder_counter(state->msgq_id)) break;
    //     usleep(1000);
    // }
    // set_desire_speed(0);
    // sleep(1);

    // steering to 1000
    set_steering(1000);
    sleep(1);

    // turn 90-degree backward
    set_encoder_counter(0); // initialize encoder counter as 0
    desire_encoder = -(RADIUS * PI / 2 * TICK_PER_CM); // 30*(pi/2)
    // set_desire_encoder_count(desire_encoder);
    set_desire_speed(-50);
    while (1) {
        if (desire_encoder > read_encoder_counter(state->msgq_id)) break;
        usleep(1000);
    }
    set_desire_speed(0);
    sleep(1);

    // steering to 1500
    set_steering(1500);
    sleep(1);

    // regress until the distance from the wall is 10cm
    // set_encoder_counter(0);          // initialize encoder counter as 0
    // set_desire_encoder_count(-1000); // regress
    set_desire_speed(-50);
    while (1) {
        if ((rr)->psd.value[PSD_BACK] < 11.f) {
            desire_encoder = read_encoder_counter(state->msgq_id);
            // desire_encoder will be used to go straight after parking-complete
            // stop
            // set_encoder_counter(0);
            // set_desire_encoder_count(0);
            set_desire_speed(0);
            break;
        }
        usleep(1000);
    }

    // beep
    beep(50);
    sleep(1);

    // go straight as the car regressed
    // set_desire_encoder_count(desire_encoder);
    set_encoder_counter(0);
    desire_encoder = -desire_encoder;
    set_desire_speed(50);
    while (desire_encoder > read_encoder_counter(state->msgq_id))
        usleep(10000);
    set_desire_speed(0);
    sleep(1);

    // steering to 1000
    set_steering(1000);
    sleep(1);

    // turn 90-degree forward
    set_encoder_counter(0);
    desire_encoder = RADIUS * PI / 2 * TICK_PER_CM;
    // set_desire_encoder_count(desire_encoder);
    set_desire_speed(50);
    while (desire_encoder > read_encoder_counter(state->msgq_id))
        usleep(10000);
    set_desire_speed(0);
    sleep(1);

    // steering to 1500
    set_steering(1500);
    sleep(1);

    parking_complete++;
    // init_drive(&state);

    (rr)->stop_obstacle.enabled = so_enable;
    (rr)->traffic_light.enabled = tl_enable;
    // beep
    beep(200);
    sleep(3);

    speed_control_on();
    return;
}

void do_parking_parallel(State *state) { parking_complete++; }

void position_control_on(void) {
    // this function donesn't include 'initialize encoder count as 0'
    // you should set encoder counter as 0 before enter desire encoder count
    printf("position_control_on");
    container data;

    data.c_uint8 = 1;
    if (ctrl_msgq(CMD_SPEED_CONTROL_ON_OFF, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail to control on speed control: mission-parking\n");

    data.c_int16 = 100;
    if (ctrl_msgq(CMD_DESIRE_SPEED, 2, &data) != MSG_STATE_SUCCESS)
        printf("fail to set desire speed: mission-parking\n");

    data.c_uint8 = 1;
    if (ctrl_msgq(CMD_POSITION_CONTROL_ON_OFF, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail to control on position control: mission-parking\n");

    data.c_uint8 = 10;
    if (ctrl_msgq(CMD_POSITION_PROPORTION_POINT, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail to set position proportion point: mission-parking\n");
}
void speed_control_on(void) {
    container data;

    data.c_int16 = 1810;
    if (ctrl_msgq(CMD_CAMERA_Y_SERVO_CONTROL, 2, &data) != MSG_STATE_SUCCESS)
        printf("fail 1\n");

    data.c_uint8 = 0;
    if (ctrl_msgq(CMD_POSITION_CONTROL_ON_OFF, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail 2\n");

    data.c_uint8 = 1;
    if (ctrl_msgq(CMD_SPEED_CONTROL_ON_OFF, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail 3\n");

    data.c_uint8 = 20;
    if (ctrl_msgq(CMD_SPEED_PID_PROPORTIONAL, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail 4\n");

    data.c_uint8 = 20;
    if (ctrl_msgq(CMD_SPEED_PID_INTEGRAL, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail 5\n");

    data.c_uint8 = 20;
    if (ctrl_msgq(CMD_SPEED_PID_DIFFERENTAL, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail 6\n");

    data.c_int16 = 300;
    if (ctrl_msgq(CMD_DESIRE_SPEED, 2, &data) != MSG_STATE_SUCCESS)
        printf("fail 7\n");
}

int read_encoder_counter(int msgqid) {
    container data;
    if (message_ctrlboard(msgqid, MSGQ_ID_PROCESS, CMD_ENCODER_COUNTER,
                          CMD_TYPE_READ, 4, &data) != MSG_STATE_SUCCESS)
        printf("fail to read encoder count, mission-parking\n");
    return data.c_int32;
}
void set_encoder_counter(int encoder_counter) {
    container data;
    data.c_int32 = encoder_counter;
    if (ctrl_msgq(CMD_ENCODER_COUNTER, 4, &data) != MSG_STATE_SUCCESS)
        printf("fail to write encode counter to %d: mission-parking\n",
               encoder_counter);
}

void set_desire_encoder_count(int desire_encoder) {
    container data;
    data.c_int32 = desire_encoder;
    if (ctrl_msgq(CMD_DESIRE_ENCODER_COUNT, 4, &data) != MSG_STATE_SUCCESS)
        printf("fail to set desire encoder_count: mission-parking\n");
}

void set_steering(short steering) {
    container data;
    data.c_int16 = steering;
    if (ctrl_msgq(CMD_STEERING_SERVO_CONTROL, 2, &data) != MSG_STATE_SUCCESS)
        printf("fail to set steering : mission-parking ");
}
void set_desire_speed(short speed) {
    container data;
    data.c_int16 = speed;
    if (ctrl_msgq(CMD_DESIRE_SPEED, 2, &data) != MSG_STATE_SUCCESS)
        printf("fail to set desire_speed");
}

void beep(char time) {
    container data = {time};
    if (ctrl_msgq(CMD_SOUND, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail to sound beep: mission-parking\n");
    sleep(1);
}