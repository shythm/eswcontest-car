#ifndef _CARPSD_LIB_H
#define _CARPSD_LIB_H

#include <stdint.h>
#include <sys/ipc.h>
#include "util.h"

/* PSD Sensor Types */
#define PSD_COUNT       6
#define PSD_FRONT       0
#define PSD_RIGHT_1     1
#define PSD_RIGHT_2     2
#define PSD_BACK        3
#define PSD_LEFT_2      4
#define PSD_LEFT_1      5

/*
 * This is shared memory structure for the results of
 * converting analog signal of psd sensor to digital.
 */
typedef struct _psd_data {
    uint16_t raw_value[PSD_COUNT];      // raw value of psd sensor (12bit)
    float processed_value[PSD_COUNT];   // processed value of psd sensor (unit: cm)
} psd_data;

/* 
 * Initialize a shared memory of the psd data.
 * In the other processes except for the carpsd process, set init to zero(0).
 * Return value 0 represents succeed to initialize.
 */
int get_shm_psd_data(key_t key, int* id, psd_data** pd, int init);

#endif /* _CARPSD_LIB_H */