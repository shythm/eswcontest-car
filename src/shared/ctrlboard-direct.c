#include "ctrlboard-direct.h"

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "util.h"

#define UART_BUF_SIZE 8
#define BAUDRATE      B19200
#define SERIAL_DEVICE "/dev/ttyS2"

static int uart_fd;

int ctrld_init(void) {
    struct termios newtio;

    /* UART configuration */
    uart_fd = open(SERIAL_DEVICE, O_RDWR | O_NOCTTY);
    if (uart_fd < 0) {
        ERROR("Serial %s Device Error", SERIAL_DEVICE);
        return 1;
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag     = BAUDRATE | CS8 | CLOCAL | CREAD; // | CRTSCTS;
    newtio.c_iflag     = IGNPAR;
    newtio.c_oflag     = 0;
    newtio.c_lflag     = 0;
    newtio.c_cc[VTIME] = 0; // inter-character timer unused
    newtio.c_cc[VMIN]  = 1; // blocking read until 8 chars received

    tcflush(uart_fd, TCIFLUSH);
    tcsetattr(uart_fd, TCSANOW, &newtio);

    MSG("UART Device %s was initialized.", SERIAL_DEVICE);
    return 0;
}

int get_cmd_bytec(ctrl_cmdc cmdc) {
    switch (cmdc) {
    case CMDC_DESIRE_ENCODER_COUNT:
    case CMDC_ENCODER_COUNTER:
        return 4;
    case CMDC_DESIRE_SPEED:
    case CMDC_STEERING_SERVO_CONTROL:
    case CMDC_CAMERA_X_SERVO_CONTROL:
    case CMDC_CAMERA_Y_SERVO_CONTROL:
        return 2;
    default:
        return 1;
    }
}

void ctrld_write(ctrl_cmdc code, int value) {
    static int           bytec, i, cur_buf_i;
    static unsigned char buf[UART_BUF_SIZE];

    bytec = get_cmd_bytec(code);

    buf[0] = code;      // 1st byte: cmd code
    buf[1] = 2 + bytec; // 2nd byte: 2(default length) + cmd byte count
    buf[2] = 1;         // 3rd byte: cmd type(1: write)

    // next byte(s): cmd arguments
    cur_buf_i = 3;
    for (i = 0; i < bytec; i++) {
        buf[cur_buf_i] = 0xFF & (value >> (8 * i));
        cur_buf_i++;
    }

    // last byte: checksum
    buf[cur_buf_i] = 0;
    for (i = 0; i < cur_buf_i; i++) {
        // sum all of the bytes except for checksum byte
        buf[cur_buf_i] += buf[i];
    }

    // write to the uart device (byte count: cur_buf_i + 1)
    write(uart_fd, buf, cur_buf_i + 1);
    usleep(1000 * 3);
}

int ctrld_read(ctrl_cmdc code, int *value) {
    int bytec = get_cmd_bytec(code);

    unsigned char bufw[UART_BUF_SIZE];
    unsigned char bufr[UART_BUF_SIZE];

    bufw[0] = code; // 1st byte: cmd code
    bufw[1] = 2;    // 2nd byte: default length
    bufw[2] = 2;    // 3rd byte: cmd type(2: read)

    bufw[3] = bufw[0] + bufw[1] + bufw[2]; // last byte: checksum
    write(uart_fd, bufw, 4); // write to the uart device (byte count: 4)

    // read from the uart device
    // (byte count: two reserved bytes + output byte(s) + checksum byte)
    read(uart_fd, bufr, 3 + bytec);

#ifndef DISABLE_OUTPUT_CHECKSUM
    // check the checksum
    int chks = 0;
    for (int i = 0; i < 2 + bytec; i++) {
        // first, sum all of the read bytes except for checksum
        chks += bufr[i];
    }
    // second, proceed with the modulo operation. then, we will get checksum
    // from read bytes
    chks %= 256;

    if (bufr[2 + bytec] != chks) { return CMDR_CHKSUM_ERR; }
#endif

    *value = 0;
    for (int i = 0; i < bytec; i++) {
        // store the output bytes
        *value += bufr[2 + i] << (8 * i);
    }

    return CMDR_SUCCESS;
}