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

#define CMD_COUNT 16

// command codes according to ctrl_cmd enum.
static const int cmd_code[CMD_COUNT] = {
    0x90, 0x91, 0x92, 0x93, 0x94, 0x96, 0x97, 0x98,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA5, 0xA7, 0xB0, 0xB1,
};
// byte counts of command argument
static const int cmd_bytec[CMD_COUNT] = {
    1, 2, 1, 1, 1, 1, 4, 1, 1, 1, 1, 2, 2, 2, 4, 1,
};
// command states for ignoring reduplication writing.
static int cmd_state[CMD_COUNT];

int ctrld_init(void) {
    MSG("Initialize the control board");

    /* UART configuration */
    uart_fd = open(SERIAL_DEVICE, O_RDWR | O_NOCTTY);
    if (uart_fd < 0) {
        ERROR("!- Configuring UART device(%s) fail", SERIAL_DEVICE);
        return 1;
    }

    struct termios newtio;
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag     = BAUDRATE | CS8 | CLOCAL | CREAD; // | CRTSCTS;
    newtio.c_iflag     = IGNPAR;
    newtio.c_oflag     = 0;
    newtio.c_lflag     = 0;
    newtio.c_cc[VTIME] = 0; // inter-character timer unused
    newtio.c_cc[VMIN]  = 1; // blocking read until 8 chars received

    tcflush(uart_fd, TCIFLUSH);
    tcsetattr(uart_fd, TCSANOW, &newtio);

    MSG("-- Configuring UART device(%s) done", SERIAL_DEVICE);

    /* Get states of the control board */
    for (ctrl_cmd cmd = 0; cmd < CMD_COUNT; cmd++) {
        if (ctrld_read(cmd, &cmd_state[cmd])) {
            ERROR("!- Getting the states fail");
            return 1;
        }
    }

    MSG("-- Getting current states done");

    return 0;
}

void ctrld_write(ctrl_cmd cmd, int val) {
    if (val == cmd_state[cmd]) {
        // ignore reduplication writing operation
        return;
    } else {
        cmd_state[cmd] = val;
    }

    int           i, curr;
    unsigned char buf[UART_BUF_SIZE];

    buf[0] = cmd_code[cmd];      // 1st byte: cmd code
    buf[1] = cmd_bytec[cmd] + 2; // 2nd byte: cmd byte count + default length(2)
    buf[2] = 1;                  // 3rd byte: cmd type(1: write)

    // next byte(s): cmd argument
    curr = 3;
    for (int i = 0; i < cmd_bytec[cmd]; i++) {
        buf[curr] = 0xFF & (val >> (8 * i));
        curr++;
    }

    // last byte: checksum
    buf[curr] = 0;
    for (int i = 0; i < curr; i++) {
        // sum all of the bytes except for the checksum byte
        buf[curr] += buf[i];
    }

    // write to the uart device (byte count: cur_buf_i + 1)
    write(uart_fd, buf, curr + 1);

    usleep(1000 * 3);
}

int ctrld_read(ctrl_cmd cmd, int *val) {
    unsigned char bufw[UART_BUF_SIZE];
    unsigned char bufr[UART_BUF_SIZE];

    bufw[0] = cmd_code[cmd];               // 1st byte: cmd code
    bufw[1] = 2;                           // 2nd byte: default length(2)
    bufw[2] = 2;                           // 3rd byte: cmd type(2: read)
    bufw[3] = bufw[0] + bufw[1] + bufw[2]; // last byte: checksum

    write(uart_fd, bufw, 4); // write to the uart device (byte count: 4)

    // read from the uart device
    // (byte count: two reserved bytes + output byte(s) + checksum byte)
    read(uart_fd, bufr, 3 + cmd_bytec[cmd]);

#ifndef DISABLE_READ_CHECKSUM
    // check the checksum
    int chks = 0;
    int last = 2 + cmd_bytec[cmd];
    for (int i = 0; i < last; i++) {
        // first, sum all of the read bytes except for checksum
        chks += bufr[i];
    }
    // second, proceed with the modulo operation. then, we will get checksum
    // from read bytes
    chks %= 256;

    if (bufr[last] != chks) { return CMDR_CHKSUM_ERR; }
#endif

    *val = 0;
    for (int i = 0; i < cmd_bytec[cmd]; i++) {
        // store the output bytes
        *val += bufr[2 + i] << (8 * i);
    }

    return CMDR_SUCCESS;
}