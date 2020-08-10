#ifndef _CARPSD_H
#define _CARPSD_H

#include <stdint.h>
#include <sys/ipc.h>

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
 * Get shared memory of psd data structure by key.
 * The carpsd process should be turned on. Because the carpsd process allocates shared memory.
 * You can access the shared memory by the pd argument.
 * On success, 0 is returned. On error, -1 is returned.
 */
int get_shm_psd_data(key_t key, psd_data** pd);

#endif