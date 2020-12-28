#ifndef _CAR_HEADER_H_
#define _CAR_HEADER_H_

#include "ctrlboard-direct.h"
#include "process.h"
#include <stdbool.h>

#define TICK_PER_CM 19.7628f
#define RADIUS      37.00f
#define PI          3.141592f

#define CONSTRAIN(val, min, max) ((val < min) ? min : ((max < val) ? max : val))

int   read_encoder_counter();
short read_desire_speed();
short read_steering();
void  set_encoder_counter(int encoder_counter);
void  set_steering(short steering);
void  set_desire_speed(short speed);
void  beep(unsigned char time);
void  move(short speed, int desire_encoder);
void  set_camera_Yservo(short y_servo);
bool  get_is_on_stop_line();

#endif