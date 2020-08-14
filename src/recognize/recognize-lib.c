#include "recognize-lib.h"

/* 
 * Initialize a shared memory of the recogition results.
 * In the other processes except for the recognize process, set init to zero(0).
 */
int get_shm_recog_result(key_t key, int* id, recog_result** rr, int init) {
    // Reference: https://unabated.tistory.com/entry/%EA%B3%B5%EC%9C%A0-%EB%A9%94%EB%AA%A8%EB%A6%AC-shared-memory
    int getflags, atflags, size;
    if (init) {
        // arguments for the recognize process
        getflags = IPC_CREAT | IPC_EXCL | 0666;
        size = sizeof(recog_result);
        atflags = SHM_RND;
    } else {
        // arguments for the other processes
        getflags = 0;
        size = 0;
        // atflags = SHM_RDONLY; // the kernal can restrict to access of shared memory by using SHM_RDONLY.
        atflags = SHM_RND;
    }

    /* Get shared memory id with the key */
    if ((*id = shmget(key, size, getflags)) == -1) {
        ERROR("Cannot get shared memory id with the key(%d).", key);
        return -1;
    }

    /* Get shared memory address with the id */
    if ((*rr = (recog_result*)shmat(*id, NULL, atflags)) == (recog_result*)-1) {
        ERROR("Cannot allocate shared memory");
        return -1;
    }

    /* Initialize the shared memory contents to zero values */
    // When the shared memory segment is created, it shall be initialized with all zero values.
    // Reference: https://pubs.opengroup.org/onlinepubs/009695399/functions/shmget.html

    MSG("Shared memory(key: %d, id: %d) has been initialized.", key, *id);
    return 0;
}
