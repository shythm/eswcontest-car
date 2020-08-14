#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <signal.h>
#include <time.h>

#include "util.h"
#include "carpsd-lib.h"

#define I2C_DEVICE      "/dev/i2c-2"
#define I2C_BUF_SIZE    8
#define I2C_DELAY_US    2000

#define PSD_FRONT_CMD       0x8C
#define PSD_RIGHT_1_CMD     0xCC
#define PSD_RIGHT_2_CMD     0x9C
#define PSD_BACK_CMD        0xDC
#define PSD_LEFT_2_CMD      0xAC
#define PSD_LEFT_1_CMD      0xEC

int init_psd_i2c(int* fd);
int init_shm_psd(psd_data** pd);
int get_psd_raw_value(int fd, uint16_t* value);
int get_psd_processed_value(uint16_t* const raw, float* processed);

int main(int argc, char** argv) {
    key_t shm_key;
    int shm_id;
    int i2c_fd;
    psd_data* shm_pd;

    /* Get the arguments from the command line. */
    if (argc != 2) {
        printf("usage %s [shared memory key] \n", argv[0]);
        return 1;
    }
    shm_key = atoi(argv[1]);    // 1st argv: get shared memory key

    /* Initialize I2C communication */
    if (init_psd_i2c(&i2c_fd) != 0) {
        ERROR("Cannot initialize I2C communication.");
        return -1;
    }

    /* Initialize shared memory */
    if (get_shm_psd_data(shm_key, &shm_id, &shm_pd, 1) != 0) {
        ERROR("Cannot initialize shared memory.");
        return -1;
    }

    /* Update PSD value */
    for (;;) {
        // get the raw psd values
        get_psd_raw_value(i2c_fd, shm_pd->raw_value);
        // get the processed psd values
        get_psd_processed_value(shm_pd->raw_value, shm_pd->processed_value);
    }

    return 0;
}

int init_psd_i2c(int* fd) {
    *fd = open(I2C_DEVICE, O_RDWR);

    if(*fd < 0) {
        perror("Opening i2c device node");
        return -1;
    }

    if (ioctl(*fd, I2C_SLAVE, 0x4b) < 0) {
        // TODO: 0x4b is a magic number. I don't know what it means. So, find that means.
        perror("Selecting i2c device");
        return -1;
    }

    MSG("I2C Device %s has been initialized.", I2C_DEVICE);
    return 0;
}

int get_psd_raw_value(int fd, uint16_t* value) {
    unsigned char buf[I2C_BUF_SIZE];
    unsigned char buf_read[I2C_BUF_SIZE];
    unsigned char psd_channel[PSD_COUNT] = {
        PSD_FRONT_CMD,
        PSD_RIGHT_1_CMD,
        PSD_RIGHT_2_CMD,
        PSD_BACK_CMD,
        PSD_LEFT_2_CMD, 
        PSD_LEFT_1_CMD
    };
    useconds_t delay = I2C_DELAY_US;
    size_t buf_read_size = 2;
    int ret, i;

    for (i = 0; i < PSD_COUNT; i++) {
        // command to i2c
        write(fd, psd_channel + i, 1);
        usleep(delay);

        // get the psd raw value from i2c
        if (read(fd, buf_read, buf_read_size) != buf_read_size) {
            perror("Reading i2c device");
            return -1;
        }
        usleep(delay);

        value[i] = ((buf_read[0] & 0b00001111) << 8) + buf_read[1];
    }

    return 0;
}

int get_psd_processed_value(uint16_t* const raw, float* processed) {
    int i;
    float x;

    x = raw[PSD_FRONT];
    processed[PSD_FRONT] = 51.83f * expf(-0.001981f * x) + 17.8f * expf(-0.0004166f * x);

    x = raw[PSD_RIGHT_1];
    processed[PSD_RIGHT_1] = 52.04f * expf(-0.001964f * x) + 18.16f * expf(-0.0003931f * x);

    x = raw[PSD_RIGHT_2];
    processed[PSD_RIGHT_2] = 51.58f * expf(-0.001936f*x) + 17.79f * expf(-0.0003686f*x);

    x = raw[PSD_BACK];
    processed[PSD_BACK] = 57.7f * expf(-0.002206f * x) + 19.1f * expf(-0.0004304f * x);

    x = raw[PSD_LEFT_2];
    processed[PSD_LEFT_2] = 490.1f * expf(-0.004111f * x) + 24.61f * expf(-0.0004845f * x);

    x = raw[PSD_LEFT_1];
    processed[PSD_LEFT_1] = 638.6f * expf(-0.004488f * x) + 26.45f * expf(-0.000508f * x);

    for (i = 0; i < PSD_COUNT; i++) {
        if (processed[i] <= 4.0f)
            processed[i]=4.000f;
        else if (processed[i] >= 30.0f)
            processed[i] = 30.0f;
    }
    
    return 0;
}
