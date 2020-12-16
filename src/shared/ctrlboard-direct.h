#ifndef _CTRLBOARD_DIRECT_H_
#define _CTRLBOARD_DIRECT_H_

/* Commands for the control board */
typedef enum {
    CMD_SPEED_CONTROL_ON_OFF,
    CMD_DESIRE_SPEED,
    CMD_SPEED_PID_PROPORTIONAL,
    CMD_SPEED_PID_INTEGRAL,
    CMD_SPEED_PID_DIFFERENTAL,
    CMD_POSITION_CONTROL_ON_OFF,
    CMD_DESIRE_ENCODER_COUNT,
    CMD_POSITION_PROPORTION_POINT,
    CMD_FRONT_A_REAL_LIGHT_CONTROL,
    CMD_RIGHT_A_LEFT_FLICKER_CONTROL,
    CMD_SOUND,
    CMD_STEERING_SERVO_CONTROL,
    CMD_CAMERA_X_SERVO_CONTROL,
    CMD_CAMERA_Y_SERVO_CONTROL,
    CMD_ENCODER_COUNTER,
    CMD_LINE_SENSOR,
} ctrl_cmd;

#define CMDR_SUCCESS    0 // command result: success
#define CMDR_CHKSUM_ERR 1 // command result: ckechsum error

/* Initialize the control board. You should call this function before using
 * ctrld_write or ctrld_read function. */
int ctrld_init(void);

/* Write this command to the control board. */
void ctrld_write(ctrl_cmd cmd, int val);

/* Read a state of this command from the control board. */
int ctrld_read(ctrl_cmd cmd, int *val);

#endif /* ctrlboard-direct.h */