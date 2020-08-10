#ifndef _CTRLBOARD_H
#define _CTRLBOARD_H

#define MAX_CMD_BYTE_CNT  4

/* Command codes */
#define CMD_SPEED_CONTROL_ON_OFF            0x90
#define CMD_DESIRE_SPEED                    0x91
#define CMD_SPEED_PID_PROPORTIONAL          0x92
#define CMD_SPEED_PID_INTEGRAL              0x93
#define CMD_SPEED_PID_DIFFERENTAL           0x94
#define CMD_POSITION_CONTROL_ON_OFF         0x96
#define CMD_DESIRE_ENCODER_COUNT            0x97
#define CMD_POSITION_PROPORTION_POINT       0x98
#define CMD_FRONT_A_REAL_LIGHT_CONTROL      0xA0
#define CMD_RIGHT_A_LEFT_FLICKER_CONTROL    0xA1
#define CMD_SOUND                           0xA2
#define CMD_STEERING_SERVO_CONTROL          0xA3
#define CMD_CAMERA_X_SERVO_CONTROL          0xA5
#define CMD_CAMERA_Y_SERVO_CONTROL          0xA7
#define CMD_ENCODER_COUNTER                 0xB0
#define CMD_LINE_SENSOR                     0xB1

/* Command types */
#define CMD_TYPE_WRITE     1
#define CMD_TYPE_READ      2

/* Message states */
typedef char ctrlboard_msg_state_t;
#define MSG_STATE_SUCCESS       0
#define MSG_STATE_TYPE_ERR      -1
#define MSG_STATE_CHKSUM_ERR    -2

/*
 * Command structure for control board.
 * It contains the command code, type(read or write), and the counts of the arguments.
 */
typedef struct _ctrlboard_cmd_t {
    // the command code
    unsigned char code;
    // the command type (CTRLBOARD_CMD_WRITE or CTRLBOARD_CMD_READ)
    unsigned char rw;
    // the count of bytes
    unsigned char bytec;
} ctrlboard_cmd_t;

/*
 * Message structure for ctrlboard process.
 * It contains the message id(for return the message), command, state, and bytes.
 * The state field represends whether the processing of the message had been succeed.
 * The bytes field is used for getting the required arguments in write command
 * or storing the output value in read command.
 */
typedef struct _ctrlboard_msg {
    // the message id
    long msgid;
    // the command to be sent
    ctrlboard_cmd_t cmd;
    // whether the processing of the message had been succeed
    ctrlboard_msg_state_t state;
    // the required arguments in write command or the output value in read command
    unsigned char bytes[MAX_CMD_BYTE_CNT];
} ctrlboard_msg;

#endif