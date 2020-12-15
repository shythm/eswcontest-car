#ifndef _CTRLBOARD_DIRECT_H_
#define _CTRLBOARD_DIRECT_H_

/* Command codes */
typedef enum cmdcode {
    CMDC_SPEED_CONTROL_ON_OFF         = 0x90,
    CMDC_DESIRE_SPEED                 = 0x91,
    CMDC_SPEED_PID_PROPORTIONAL       = 0x92,
    CMDC_SPEED_PID_INTEGRAL           = 0x93,
    CMDC_SPEED_PID_DIFFERENTAL        = 0x94,
    CMDC_POSITION_CONTROL_ON_OFF      = 0x96,
    CMDC_DESIRE_ENCODER_COUNT         = 0x97,
    CMDC_POSITION_PROPORTION_POINT    = 0x98,
    CMDC_FRONT_A_REAL_LIGHT_CONTROL   = 0xA0,
    CMDC_RIGHT_A_LEFT_FLICKER_CONTROL = 0xA1,
    CMDC_SOUND                        = 0xA2,
    CMDC_STEERING_SERVO_CONTROL       = 0xA3,
    CMDC_CAMERA_X_SERVO_CONTROL       = 0xA5,
    CMDC_CAMERA_Y_SERVO_CONTROL       = 0xA7,
    CMDC_ENCODER_COUNTER              = 0xB0,
    CMDC_LINE_SENSOR                  = 0xB1,
} ctrl_cmdc;

/* Command results */
#define CMDR_SUCCESS    0
#define CMDR_CHKSUM_ERR 1

/* Functions */
int  ctrld_init(void);
void ctrld_write(ctrl_cmdc code, int value);
int  ctrld_read(ctrl_cmdc code, int *value);

#endif /* ctrlboard-direct.h */