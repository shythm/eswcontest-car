#include "util.h"
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define PSD_I2C_DEVICE   "/dev/i2c-2"
#define SERIAL_DEVICE    "/dev/ttyS2"
#define BAUDRATE         B19200
#define UART_BUF_SIZE    8
#define PSD_I2C_DELAY_US 2000
#define PSD_DISTANCE_MIN 4.0f
#define PSD_DISTANCE_MAX 30.f
#define GAIN_P           80.0f
#define TARGET_STEERING  1500
int main(void) {
    int            i2c_fd;
    struct termios newtio;
    int            uart_fd;

    /* I2C configuration*/
    if ((i2c_fd = open(PSD_I2C_DEVICE, O_RDWR)) < 0) {
        ERROR("Failed to open I2C for PSD");
        return 1;
    }
    if (ioctl(i2c_fd, I2C_SLAVE, 0x4b) < 0) {
        ERROR("Failed to ioctl I2C for PSD");
        return 1;
    }
    MSG("I2C Device for PSD(%s) has been initialized.", PSD_I2C_DEVICE);

    /* UART configuration */
    if ((uart_fd = open(SERIAL_DEVICE, O_RDWR | O_NOCTTY)) < 0) {
        ERROR("Failed to open UART");
        return 1;
    }
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag     = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag     = IGNPAR;
    newtio.c_oflag     = 0;
    newtio.c_lflag     = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN]  = 1;
    tcflush(uart_fd, TCIFLUSH);
    tcsetattr(uart_fd, TCSANOW, &newtio);

    unsigned char buf[UART_BUF_SIZE];
    unsigned char buf_psd[2];
    short         speed;
    short         steering;
    float         position;
    float         psd_left;
    float         psd_right;
    int           delay = 0;

    buf[0] = 0x96; // POSITION_CONTROL_ON_OFF
    buf[1] = 3;    // default length(2) + required byte count(1)
    buf[2] = 1;    // command to write
    buf[3] = 0;
    buf[4] = buf[0] + buf[1] + buf[2] + buf[3];
    write(uart_fd, buf, 5);

    buf[0] = 0x90; // SPEED_CONTROL_ON_OFF
    buf[1] = 3;    // default length(2) + required byte count(1)
    buf[2] = 1;    // command to write
    buf[3] = 1;
    buf[4] = buf[0] + buf[1] + buf[2] + buf[3];
    write(uart_fd, buf, 5);

    buf[0] = 0x92; // SPEED_PID_PROPORTIONAL
    buf[1] = 3;    // default length(2) + required byte count(1)
    buf[2] = 1;    // command to write
    buf[3] = 28;
    buf[4] = buf[0] + buf[1] + buf[2] + buf[3];
    write(uart_fd, buf, 5);

    buf[0] = 0x93; // SPEED_PID_INTEGRAL
    buf[1] = 3;    // default length(2) + required byte count(1)
    buf[2] = 1;    // command to write
    buf[3] = 40;
    buf[4] = buf[0] + buf[1] + buf[2] + buf[3];
    write(uart_fd, buf, 5);

    buf[0] = 0x94; // SPEED_PID_DIFFERENTAL
    buf[1] = 3;    // default length(2) + required byte count(1)
    buf[2] = 1;    // command to write
    buf[3] = 40;
    buf[4] = buf[0] + buf[1] + buf[2] + buf[3];
    write(uart_fd, buf, 5);

    speed  = 20;
    buf[0] = 0x91; // CMD_DESIRE_SPEED
    buf[1] = 4;    // default length(2) + required byte count(2)
    buf[2] = 1;    // command to write
    buf[3] = speed & 0xff;
    buf[4] = (speed >> 8) & 0xff;
    buf[5] = buf[0] + buf[1] + buf[2] + buf[3] + buf[4];
    write(uart_fd, buf, 6);

    for (;;) {
        /* get psd value */
        buf_psd[0] = 0xEC;
        write(i2c_fd, buf_psd, 1); // LEFT_1
        usleep(PSD_I2C_DELAY_US);
        if (read(i2c_fd, buf_psd, 2) != 2) {
            ERROR("Failed to read PSD data from I2C.");
            return 1;
        }
        psd_left = ((buf_psd[0] & 0b00001111) << 8) + buf_psd[1];
        psd_left = 638.6f * expf(-0.004488f * psd_left) +
                   26.45f * expf(-0.000508f * psd_left);
        if (psd_left <= PSD_DISTANCE_MIN) {
            psd_left = PSD_DISTANCE_MIN;
        } else if (psd_left >= PSD_DISTANCE_MAX) {
            psd_left = PSD_DISTANCE_MAX;
        }
        usleep(PSD_I2C_DELAY_US);

        buf_psd[0] = 0xCC;
        write(i2c_fd, buf_psd, 1); // RIGHT_1
        usleep(PSD_I2C_DELAY_US);
        if (read(i2c_fd, buf_psd, 2) != 2) {
            ERROR("Failed to read PSD data from I2C.");
            return 1;
        }
        psd_right = ((buf_psd[0] & 0b00001111) << 8) + buf_psd[1];
        psd_right = 52.04f * expf(-0.001964f * psd_right) +
                    18.16f * expf(-0.0003931f * psd_right);
        if (psd_right <= PSD_DISTANCE_MIN) {
            psd_right = PSD_DISTANCE_MIN;
        } else if (psd_right >= PSD_DISTANCE_MAX) {
            psd_right = PSD_DISTANCE_MAX;
        }
        usleep(PSD_I2C_DELAY_US);

        // printf("%f %f \n", psd_left, psd_right);
        // usleep(100 * 1000);

        /* control servo motor */
        position = GAIN_P * (psd_left - psd_right);
        steering = TARGET_STEERING + (short)position;
        if (steering > 2000) {
            steering = 2000;
        } else if (steering < 1000) {
            steering = 1000;
        }

        buf[0] = 0xA3; // STEERING_SERVO_CONTROL
        buf[1] = 4;    // default length(2) + required byte count(2)
        buf[2] = 1;    // command to write
        buf[3] = steering & 0xff;
        buf[4] = (steering >> 8) & 0xff;
        buf[5] = buf[0] + buf[1] + buf[2] + buf[3] + buf[4];
        write(uart_fd, buf, 6);

        delay++;
        if (delay >= 100) {
            // printf("%.2f %.2f %f %d \n", psd_left, psd_right, position,
            //        steering);
            delay = 0;
        }
    }

    return 0;
}