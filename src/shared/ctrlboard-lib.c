#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "config-car.h"
#include "ctrlboard-lib.h"
#include "util.h"

int get_mqid_ctrl(mqid_ctrl *mgid) {
    int id_snd   = -1;
    int id_rcv   = -1;
    int msg_flag = IPC_CREAT | 0666;

    // Get Message Queue ID for sending to ctrlboard process
    if ((id_snd = msgget(KEY_MSGQ_CTRLBOARD, msg_flag)) == -1) {
        ERROR("Cannot get message queue id with the key.");
        return -1;
    }

    // Get Message Queue ID for recieving from ctrlboard process
    if ((id_rcv = msgget(KEY_MSGQ_CTRLBOARD_RCV, msg_flag)) == -1) {
        ERROR("Cannot get message queue id with the key.");
        return -1;
    }

    // store message queue ID
    mgid->id_snd = id_snd;
    mgid->id_rcv = id_rcv;

    MSG("Success to get message queue of ctrlboard(id_snd: %d, id_rcv: %d)",
        KEY_MSGQ_CTRLBOARD, mgid->id_snd, mgid->id_rcv);

    return 0;
}

ctrlboard_msg_state_t comm_ctrlboard(mqid_ctrl mqid, long mid,
                                     ctrlboard_cmd_code code,
                                     ctrlboard_cmd_rw rw, unsigned char bytec,
                                     ctrlboard_byte_container *data) {
    static size_t size = sizeof(ctrlboard_msg) - sizeof(long);
    ctrlboard_msg msg;

    // set message information
    msg.msgid      = mid;
    msg.cmd.code   = code;
    msg.cmd.rw     = rw;
    msg.cmd.bytec  = bytec;
    msg.state      = MSG_STATE_UNKNOWN_ERR;
    msg.is_for_snd = 0;

    if (msg.cmd.rw == CMD_TYPE_WRITE) {
        memcpy(msg.data.bytes, data->bytes, msg.cmd.bytec);
    }

    if (msgsnd(mqid.id_snd, &msg, size, 0) == 0) {
        if (msgrcv(mqid.id_rcv, &msg, size, msg.msgid, 0) >= 0) {
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

ctrlboard_msg_state_t send_ctrlboard(mqid_ctrl mqid, long mid,
                                     ctrlboard_cmd_code        code,
                                     unsigned char             bytec,
                                     ctrlboard_byte_container *data) {
    static size_t size = sizeof(ctrlboard_msg) - sizeof(long);
    ctrlboard_msg msg;

    // set message information
    msg.msgid      = mid;
    msg.cmd.code   = code;
    msg.cmd.rw     = CMD_TYPE_WRITE;
    msg.cmd.bytec  = bytec;
    msg.state      = MSG_STATE_UNKNOWN_ERR;
    msg.is_for_snd = 1;

    if (msg.cmd.rw == CMD_TYPE_WRITE) {
        memcpy(msg.data.bytes, data->bytes, msg.cmd.bytec);
    }

    if (msgsnd(mqid.id_snd, &msg, size, 0) == 0) {
        return MSG_STATE_SUCCESS;
    } else {
        return MSG_STATE_SEND_ERR;
    }
}