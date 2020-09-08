
#include "config-car.h"
#include "ctrlboard-lib.h"
#include "recognize-lib.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>

#define ON 1
#define OFF 0

recog_result *result;
pthread_t     thr_data[2];
int           msgq_id;

#define MSGQ_ID 299

int main(int argc, char **argv) {
    usleep(100000);
    int shm_id;

    msgq_id = msgget(KEY_MSGQ_CTRLBOARD, 0);
    if (msgq_id == -1) {
        printf("메세지 아이디 얻기 실패, 프로그램 종료\n");
        return 1;
    }

    get_shm_recog_result(&result, 0);
    result->traffic_light.enabled = false;
    result->stop_obstacle.enabled = true;

    init();

    int                      steering_servo_val = 1500;
    float                    pos                = 0;
    ctrlboard_byte_container container;
    for (;;) {
        pos                = pos * 0.5 + result->lane.value.position * 0.5;
        steering_servo_val = 1500 + (short)(pos);
        if (steering_servo_val > 2000) steering_servo_val = 2000;
        if (steering_servo_val < 1000) steering_servo_val = 1000;
        container.c_int16 = steering_servo_val;
        message_ctrlboard(msgq_id, MSGQ_ID, CMD_STEERING_SERVO_CONTROL,
                          CMD_TYPE_WRITE, 2, &container);
        usleep(20000);
    }

    return 0;
}