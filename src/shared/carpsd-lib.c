#include <sys/ipc.h>
#include <sys/shm.h>
#include "carpsd-lib.h"

int get_shm_psd_data(key_t key, int* id, psd_data** pd, int init) {
    int getflags, atflags, size;
    if (init) {
        // arguments for the recognize process
        getflags = IPC_CREAT | IPC_EXCL | 0666;
        size = sizeof(psd_data);
        atflags = SHM_RND;
    } else {
        // arguments for the other process
        getflags = 0;
        size = 0;
        atflags = SHM_RDONLY; // the kernal can restrict to access of shared memory by using SHM_RDONLY.
    }

    /* Get shared memory id with the key */
    if ((*id = shmget(key, size, getflags)) == -1) {
        ERROR("Cannot get shared memory id with the key(%d).", key);
        return -1;
    }

    /* Get shared memory address with the id */
    if ((*pd = (psd_data*)shmat(*id, NULL, atflags)) == (psd_data*)-1) {
        ERROR("Cannot allocate shared memory");
        return -1;
    }

    MSG("Shared memory(key: %d, id: %d) has been initialized.", key, *id);
    return 0;
}