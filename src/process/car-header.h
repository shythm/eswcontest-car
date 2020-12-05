#ifndef _CAR_HEADER_H_
#define _CAR_HEADER_H_
#include "process.h"
#include "recognize-lib.h"

#define TICK_PER_CM 19.7628f
#define RADIUS      37.00f
#define PI          3.141592f

typedef ctrlboard_byte_container container;

mqid_ctrl mqid;

int   read_encoder_counter();
short read_desire_speed();
short read_steering();
void  set_encoder_counter(int encoder_counter);
void  set_steering(short steering);
void  set_desire_speed(short speed);
void  beep(unsigned char time);
void  move(short speed, int desire_encoder);
void  set_camer_Yservo(short y_servo);

#endif