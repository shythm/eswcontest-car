#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "carpsd-lib.h"

int main(int argc, char** argv) {
    key_t shm_key_carpsd;
    int shm_id_carpsd;
    psd_data* pd;

    if (argc != 2) {
        printf("Usage %s: [key of shared memory of carpsd] \n", argv[0]);
    }
    shm_key_carpsd = atoi(argv[1]);

    if (get_shm_psd_data(shm_key_carpsd, &shm_id_carpsd, &pd, 0) != 0) {
        printf("An error occurred while getting shared memory. \n");
    }

    for (;;) {
        printf("<RAW PSD VALUE> \n");
        printf("%5d %5d %5d %5d %5d %5d \n",
                pd->raw_value[PSD_FRONT], pd->raw_value[PSD_RIGHT_1], pd->raw_value[PSD_RIGHT_2],
                pd->raw_value[PSD_BACK], pd->raw_value[PSD_LEFT_2], pd->raw_value[PSD_LEFT_1]);
        printf("<PROCESSED PSD VALULE> \n");
        printf("%2.2f %2.2f %2.2f %2.2f %2.2f %2.2f \n",
                pd->processed_value[PSD_FRONT], pd->processed_value[PSD_RIGHT_1], pd->processed_value[PSD_RIGHT_2],
                pd->processed_value[PSD_BACK], pd->processed_value[PSD_LEFT_2], pd->processed_value[PSD_LEFT_1]);
        printf("\n");
        usleep(500000);
    }

    return 0;
}