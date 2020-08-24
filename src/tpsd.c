#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "recognize-lib.h"

int main(int argc, char** argv) {
    key_t shm_key_recognize;
    int shm_id_recognize;
    recog_result* rr;

    if (argc != 2) {
        printf("Usage %s: [shared memory key of recognize] \n", argv[0]);
        return -1;
    }
    shm_key_recognize = atoi(argv[1]);

    if (get_shm_recog_result(shm_key_recognize, &shm_id_recognize, &rr, 0) != 0) {
        printf("An error occurred while getting shared memory. \n");
        return -1;
    }

    for (;;) {
        printf("< PSD VALUE > \n");
        printf("%2.2f %2.2f %2.2f %2.2f %2.2f %2.2f \n",
                rr->psd.value[PSD_FRONT], rr->psd.value[PSD_RIGHT_1], rr->psd.value[PSD_RIGHT_2],
                rr->psd.value[PSD_BACK], rr->psd.value[PSD_LEFT_2], rr->psd.value[PSD_LEFT_1]);
        printf("\n");
        usleep(500000);
    }

    return 0;
}