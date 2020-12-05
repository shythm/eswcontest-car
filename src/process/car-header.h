#ifndef _CAR_HEADER_H_
#define _CAR_HEADER_H_
#include "process.h"
#include "recognize-lib.h"

#define TICK_PER_CM 19.7628f
#define RADIUS      37.00f
#define PI          3.141592f

typedef ctrlboard_byte_container container;

int   read_encoder_counter(mqid_ctrl mqid);
short read_desire_speed(mqid_ctrl mqid);
short read_steering(mqid_ctrl mqid);
void  set_encoder_counter(mqid_ctrl mqid, int encoder_counter);
void  set_steering(mqid_ctrl mqid, short steering);
void  set_desire_speed(mqid_ctrl mqid, short speed);
void  beep(mqid_ctrl mqid, unsigned char time);
void  move(mqid_ctrl mqid, short speed, int desire_encoder);
void  set_camer_Yservo(mqid_ctrl mqid, short y_servo);

#endif