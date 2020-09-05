#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdlib.h>

#include "config-car.h"
#include "util.h"
#include "ctrlboard-lib.h"

void clean_car() {
    // add clean process
    int id_msgq;
    int id_msg = 900 + 'c' + 'l' + 'e' + 'a' + 'n' + 'c' + 'a' + 'r';
    ctrlboard_byte_container container;

    if (get_msgq_id_ctrlboard(&id_msgq, 0) == 0) {
        // Set desire speed to zero
        container.c_int16 = 0;
        printf("id?: %d s?: %d\n", id_msgq, message_ctrlboard(id_msgq, id_msg, CMD_DESIRE_SPEED, CMD_TYPE_WRITE, 2, &container));
        fflush(stdout);
    }
}

int del_msgq_ctrlboard() {
    int id_msgq = 999;

    if ((id_msgq = msgget(KEY_MSGQ_CTRLBOARD, 0)) == -1) {
        return -1;
    }

    if (msgctl(id_msgq, IPC_RMID, NULL) == -1) {
        printf("%d\n", id_msgq);
        return -1;
    }

    return 0;
}

int del_shm_recognize() {
    int id_shm;

    if ((id_shm = shmget(KEY_SHM_RECOGNIZE, 0, 0)) == -1) {
        return -1;
    }

    if (shmctl(id_shm, IPC_RMID, NULL) == -1) {
        return -1;
    }
    
    return 0;
}

int main(int argc, char** argv) {
    clean_car();

    // delete message queue of ctrlboard
    if (del_msgq_ctrlboard() == 0) {
        MSG("Success to delete message queue of ctrlboard");
    } else {
        MSG("Fail to delete message queue of ctrlboard");
    }

    // delete shared memory of recognize
    if (del_shm_recognize() == 0) {
        MSG("Success to delete shared memory of recognize");
    } else {
        MSG("Fail to delete shared memory of recognize");
    }

    return 0;
}