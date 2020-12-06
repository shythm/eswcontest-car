#include "car-header.h"
#include "process.h"
#include "recognize-lib.h"

int read_encoder_counter() {
    container data;
    if (comm_ctrlboard(mqid, MSG_ID_PROCESS, CMD_ENCODER_COUNTER, CMD_TYPE_READ,
                       4, &data) != MSG_STATE_SUCCESS)
        printf("fail to read encoder count, car-header\n");
    return data.c_int32;
}
short read_desire_speed() {
    container data;
    if (comm_ctrlboard(mqid, MSG_ID_PROCESS, CMD_DESIRE_SPEED, CMD_TYPE_READ, 2,
                       &data) != MSG_STATE_SUCCESS)
        printf("fail to read encoder count, car-header\n");
    return data.c_int16;
}
short read_steering() {
    container data;
    if (comm_ctrlboard(mqid, MSG_ID_PROCESS, CMD_STEERING_SERVO_CONTROL,
                       CMD_TYPE_READ, 2, &data) != MSG_STATE_SUCCESS)
        printf("fail to read steering, car-header\n");
    return data.c_int16;
}

void set_encoder_counter(int encoder_counter) {
    container data;
    data.c_int32 = encoder_counter;
    if (send_ctrlboard(mqid, CMD_ENCODER_COUNTER, 4, &data) !=
        MSG_STATE_SUCCESS)
        printf("fail to set encode counter to %d: car-header\n",
               encoder_counter);
}

void set_steering(short steering) {
    container data;
    data.c_int16 = steering;
    if (send_ctrlboard(mqid, CMD_STEERING_SERVO_CONTROL, 2, &data) !=
        MSG_STATE_SUCCESS)
        printf("fail to set steering : car-header ");
}
void set_desire_speed(short speed) {
    container data;
    data.c_int16 = speed;
    if (send_ctrlboard(mqid, CMD_DESIRE_SPEED, 2, &data) != MSG_STATE_SUCCESS)
        printf("fail to set desire_speed");
}

void beep(unsigned char time) {
    container data = {time};
    if (send_ctrlboard(mqid, CMD_SOUND, 1, &data) != MSG_STATE_SUCCESS)
        printf("fail to sound beep: car-header\n");
}

void move(short speed, int desire_encoder) { // Caution: initial encoder as 0
    set_encoder_counter(0);
    set_desire_speed(speed);
    if (desire_encoder > 0) {
        while (desire_encoder > read_encoder_counter(mqid)) usleep(1000);
    } else {
        while (desire_encoder < read_encoder_counter(mqid)) usleep(1000);
    }
    set_desire_speed(0);
    usleep(100000);
}

void set_camer_Yservo(short y_servo) {
    container data;
    data.c_int16 = y_servo;
    if (send_ctrlboard(mqid, CMD_CAMERA_Y_SERVO_CONTROL, 2, &data) !=
        MSG_STATE_SUCCESS)
        printf("fail to set camera Y servo\n");
}