#ifndef _CTRLBOARD_LIB_H
#define _CTRLBOARD_LIB_H

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#define MAX_CMD_BYTE_CNT  4

/* Command codes */
typedef enum _ctrlboard_cmd_code {
    CMD_SPEED_CONTROL_ON_OFF            = 0x90,
    CMD_DESIRE_SPEED                    = 0x91,
    CMD_SPEED_PID_PROPORTIONAL          = 0x92,
    CMD_SPEED_PID_INTEGRAL              = 0x93,
    CMD_SPEED_PID_DIFFERENTAL           = 0x94,
    CMD_POSITION_CONTROL_ON_OFF         = 0x96,
    CMD_DESIRE_ENCODER_COUNT            = 0x97,
    CMD_POSITION_PROPORTION_POINT       = 0x98,
    CMD_FRONT_A_REAL_LIGHT_CONTROL      = 0xA0,
    CMD_RIGHT_A_LEFT_FLICKER_CONTROL    = 0xA1,
    CMD_SOUND                           = 0xA2,
    CMD_STEERING_SERVO_CONTROL          = 0xA3,
    CMD_CAMERA_X_SERVO_CONTROL          = 0xA5,
    CMD_CAMERA_Y_SERVO_CONTROL          = 0xA7,
    CMD_ENCODER_COUNTER                 = 0xB0,
    CMD_LINE_SENSOR                     = 0xB1,
} ctrlboard_cmd_code;

/* Command types */
typedef enum _ctrlboard_cmd_rw {
    CMD_TYPE_WRITE  = 1,
    CMD_TYPE_READ   = 2,
} ctrlboard_cmd_rw;

/* Message states */
typedef enum _ctrlboard_msg_state_t {
    MSG_STATE_SUCCESS       = 0,
    MSG_STATE_UNKNOWN_ERR   = -1,
    MSG_STATE_TYPE_ERR      = -2,
    MSG_STATE_CHKSUM_ERR    = -3,
    MSG_STATE_SEND_ERR      = -4,
    MSG_STATE_RECEIVE_ERR   = -5,
} ctrlboard_msg_state_t;

/*
 * Command structure for control board.
 * It contains the command code, type(read or write), and the counts of the arguments.
 */
typedef struct _ctrlboard_cmd_t {
    // the command code
    ctrlboard_cmd_code code;
    // the command type (CTRLBOARD_CMD_WRITE or CTRLBOARD_CMD_READ)
    ctrlboard_cmd_rw rw;
    // the count of bytes
    unsigned char bytec;
} ctrlboard_cmd_t;

/* Byte Container */
typedef union _ctrlboard_byte_container {
    char            bytes[MAX_CMD_BYTE_CNT];
    unsigned char   c_uint8;            // container unsigned 8bit integer
    short           c_int16;            // container signed   16bit integer
    int             c_int32;            // container signed   32bit integer
} ctrlboard_byte_container;

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
    ctrlboard_byte_container data;
} ctrlboard_msg;

/*
 * Get the message queue id of ctrlboard. You should write 0 to init argument except for ctrlboard process.
 */
int get_msgq_id_ctrlboard(int* id, int init);

/*
 * Send a message to the control board.
 * When ctrlboard_cmd_rw is CMD_TYPE_WRITE, bytes is used for the arguments.
 * When ctrlboard_cmd_rw is CMD_TYPE_READ,  bytes is used for storing the ouptut of the control board.
 */
ctrlboard_msg_state_t message_ctrlboard
(int msgqid, long msgid, ctrlboard_cmd_code code, ctrlboard_cmd_rw rw, unsigned char bytec, ctrlboard_byte_container* data);

#endif /* _CTRLBOARD_LIB_H */