#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "config-car.h"
#include "ctrlboard-lib.h"
#include "util.h"

int get_msgq(int *board_id, int *process_id) {
    int msg_flag = IPC_CREAT | 0666;

    // Get Message Queue ID
    if ((*board_id = msgget(KEY_MSGQ_TO_CTRLBOARD, msg_flag)) == -1) {
        ERROR("Cannot get message queue id with the key.");
        return -1;
    }
    if ((*process_id = msgget(KEY_MSGQ_TO_PROCESS, msg_flag)) == -1) {
        ERROR("Cannot get message queue id with the key.");
        return -2;
    }

    MSG("Connected to (key: %d, id: %d)/(key: %d, id: %d)",
        KEY_MSGQ_TO_CTRLBOARD, *board_id, KEY_MSGQ_TO_CTRLBOARD, *process_id);

    return 0;
}

ctrlboard_msg_state_t message_ctrlboard(int msgqid, long msgid,
                                        ctrlboard_cmd_code        code,
                                        ctrlboard_cmd_rw          rw,
                                        unsigned char             bytec,
                                        ctrlboard_byte_container *data) {
    static size_t size = sizeof(ctrlboard_msg) - sizeof(long);
    ctrlboard_msg msg;

    // set message information
    msg.msgid     = msgid;
    msg.cmd.code  = code;
    msg.cmd.rw    = rw;
    msg.cmd.bytec = bytec;
    msg.state     = MSG_STATE_UNKNOWN_ERR;

    if (msg.cmd.rw == CMD_TYPE_WRITE) {
        memcpy(msg.data.bytes, data->bytes, msg.cmd.bytec);
    }

    if (msgsnd(msgqid, &msg, size, 0) == 0) {
        if (msgrcv(msgqid, &msg, size, msg.msgid, 0) >= 0) {
            if (msg.cmd.rw == CMD_TYPE_READ) {
                memcpy(data->bytes, msg.data.bytes, msg.cmd.bytec);
            }
            return msg.state;
        } else {
            return MSG_STATE_RECEIVE_ERR;
        }
    } else {
        return MSG_STATE_SEND_ERR;
    }
}
