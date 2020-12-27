#include "car-header.h"

int read_encoder_counter() {
    int ret;
    while (1) {
        if (ctrld_read(CMD_ENCODER_COUNTER, &ret)) {
            ERROR("Fail to read encoder counter.\n");
            usleep(2000);
        } else {
            break;
        }
    }
    return ret;
}

short read_desire_speed() {
    int ret;
    if (ctrld_read(CMD_DESIRE_SPEED, &ret)) {
        ERROR("Fail to read desire speed.\n");
    }
    return (short)ret;
}

short read_steering() {
    int ret;
    if (ctrld_read(CMD_STEERING_SERVO_CONTROL, &ret)) {
        ERROR("Fail to read steering servo control speed.\n");
    }
    return (short)ret;
}

void set_steering(short steering) { //
    ctrld_write(CMD_STEERING_SERVO_CONTROL, steering);
}

void set_desire_speed(short speed) { //
    ctrld_write(CMD_DESIRE_SPEED, speed);
}

void beep(unsigned char time) { //
    ctrld_write(CMD_SOUND, 0);
    ctrld_write(CMD_SOUND, time);
}

void move(short speed, int desire_encoder) {
    // Caution: initial encoder as 0
    // set_encoder_counter(0);
    set_desire_speed(speed);
    if (desire_encoder > 0) {
        desire_encoder += read_encoder_counter();
        while (desire_encoder > read_encoder_counter()) usleep(1000);
    } else {
        desire_encoder += read_encoder_counter();
        while (desire_encoder < read_encoder_counter()) usleep(1000);
    }
    set_desire_speed(0);
    usleep(100000);
}

void set_camera_Yservo(short y_servo) { //
    ctrld_write(CMD_CAMERA_Y_SERVO_CONTROL, y_servo);
}

#define IR_SENSOR_COUNT     7
#define IR_ACTIVE_THRESHOLD 3
bool get_is_on_stop_line() {
    static int     value;
    static uint8_t ir_active_cnt;

    // LSB가 left, 검은색이 1 흰색이 0 마지막 안쓰는 MSB는 0으로 고정
    if (ctrld_read(CMD_LINE_SENSOR, &value) == CMDR_SUCCESS) {
        ir_active_cnt = 0;
        for (int i = 0; i < IR_SENSOR_COUNT; i++) {
            if (!(value & (0x01 << i))) ir_active_cnt++;
        }

        if (ir_active_cnt >= IR_ACTIVE_THRESHOLD) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}
