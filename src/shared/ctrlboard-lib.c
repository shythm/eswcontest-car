#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "config-car.h"
#include "ctrlboard-lib.h"
#include "util.h"

int get_msgq_id_ctrlboard(int *id, int init) {
    int msgq_id;

    int msg_flag = 0;
    if (init) {
        // WARNING: there is the IPC_EXCL option.
        msg_flag = IPC_CREAT | IPC_EXCL | 0666;
    }

    // Get Message Queue ID
    if ((msgq_id = msgget(KEY_MSGQ_CTRLBOARD, msg_flag)) == -1) {
        ERROR("Cannot get message queue id with the key.");
        return -1;
    }

    // return Messagq Queue ID
    *id = msgq_id;

    if (init) {
        MSG("Message queue(key: %d, id: %d) has been initialized.",
            KEY_MSGQ_CTRLBOARD, msgq_id);
    }

    return 0;
}

int get_mqid_ctrl(mqid_ctrl *mgid, int init) {
    int id_snd = -1;
    int id_rcv = -1;

    int msg_flag = 0;
    if (init) {
        // WARNING: there is the IPC_EXCL option.
        msg_flag = IPC_CREAT | IPC_EXCL | 0666;
    }

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

    if (init) {
        MSG("Message queue(id_snd: %d, id_rcv: %d) has been initialized.",
            KEY_MSGQ_CTRLBOARD, mgid->id_snd, mgid->id_rcv);
    }

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
    msg.msgid            = msgid;
    msg.cmd.code         = code;
    msg.cmd.rw           = rw;
    msg.cmd.bytec        = bytec;
    msg.state            = MSG_STATE_UNKNOWN_ERR;
    msg.is_double_mqline = 0;

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

ctrlboard_msg_state_t comm_ctrlboard(mqid_ctrl mqid, long mid,
                                     ctrlboard_cmd_code code,
                                     ctrlboard_cmd_rw rw, unsigned char bytec,
                                     ctrlboard_byte_container *data) {
    static size_t size = sizeof(ctrlboard_msg) - sizeof(long);
    ctrlboard_msg msg;

    // set message information
    msg.msgid            = mid;
    msg.cmd.code         = code;
    msg.cmd.rw           = rw;
    msg.cmd.bytec        = bytec;
    msg.state            = MSG_STATE_UNKNOWN_ERR;
    msg.is_double_mqline = 0;

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
    msg.msgid            = mid;
    msg.cmd.code         = code;
    msg.cmd.rw           = CMD_TYPE_WRITE;
    msg.cmd.bytec        = bytec;
    msg.state            = MSG_STATE_UNKNOWN_ERR;
    msg.is_double_mqline = 1;

    if (msg.cmd.rw == CMD_TYPE_WRITE) {
        memcpy(msg.data.bytes, data->bytes, msg.cmd.bytec);
    }

    if (msgsnd(mqid.id_snd, &msg, size, 0) == 0) {
        msgrcv(mqid.id_rcv, &msg, size, msg.msgid, IPC_NOWAIT);
        return MSG_STATE_SUCCESS;
    } else {
        return MSG_STATE_SEND_ERR;
    }
}