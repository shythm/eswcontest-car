#include <string.h>
#include "ctrlboard-lib.h"

ctrlboard_msg_state_t message_ctrlboard
(int msgqid, long msgid, ctrlboard_cmd_code code, ctrlboard_cmd_rw rw, unsigned char bytec, ctrlboard_byte_container* data) {
    static size_t size = sizeof(ctrlboard_msg) - sizeof(long);
    ctrlboard_msg msg;

    // set message information
    msg.msgid = msgid;
    msg.cmd.code = code;
    msg.cmd.rw = rw;
    msg.cmd.bytec = bytec;
    if (msg.cmd.rw == CMD_TYPE_WRITE) {
        memcpy(msg.data.bytes, data->bytes, msg.cmd.bytec);
    }

    if (msgsnd(msgqid, &msg, size, 0) == 0) {
        if (msgrcv(msgqid, &msg, size, msg.msgid, 0) >= 0) {
            if (msg.cmd.rw == CMD_TYPE_READ) {
                memcpy(data->bytes, msg.data.bytes, msg.cmd.bytec);
            }
            return msg.state;
        }
    }
}
