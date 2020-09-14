#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <termios.h>

#include "ctrlboard-lib.h"
#include "util.h"

#define UART_BUF_SIZE 8
#define BAUDRATE      B19200
#define SERIAL_DEVICE "/dev/ttyS2" // ttyHS0, ttyHS1, ttyHS3 are available

int                   ctrlboard_init(int *fd);
ctrlboard_msg_state_t command_ctrlboard(int uart, ctrlboard_cmd_t *cmd,
                                        char *bytes);

int main(int argc, char **argv) {
    int msgq_id;
    int uard_fd;

    /* Initialize to communicate with control board */
    if (ctrlboard_init(&uard_fd) != 0) {
        ERROR("Cannot iniailize control borad.");
        return -1;
    }

    /* Initialize to communicate other processes by message queue */
    if (get_msgq_id_ctrlboard(&msgq_id, 1) != 0) {
        ERROR("Cannot initialize message queue id.");
        return -1;
    }

    /* Message processing part */
    ctrlboard_msg msg;
    size_t        msg_size = sizeof(ctrlboard_msg) - sizeof(long);

    for (;;) {
        // Wait until receive a message. (block state)
        if (msgrcv(msgq_id, (void *)&msg, msg_size, 0, 0) != -1) {
            printf("MSGSND:%d,v=%d\n", msg.cmd.code, msg.msgid);
            // Command to the control board and get the return value.
            msg.state = command_ctrlboard(uard_fd, &msg.cmd, msg.data.bytes);
            // Send the message if the message queue is availiable. (block
            // state)
            msgsnd(msgq_id, (void *)&msg, msg_size, 0); // wait(block)
            usleep(1000);
        }
    }

    pause();
    return 0;
}

int ctrlboard_init(int *fd) {
    struct termios newtio;

    /* UART configuration */
    if ((*fd = open(SERIAL_DEVICE, O_RDWR | O_NOCTTY)) < 0) {
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

    tcflush(*fd, TCIFLUSH);
    tcsetattr(*fd, TCSANOW, &newtio);

    MSG("UART Device %s has been initialized.", SERIAL_DEVICE);
    return 0;
}

ctrlboard_msg_state_t command_ctrlboard(int uart_fd, ctrlboard_cmd_t *cmd,
                                        char *bytes) {
    unsigned char buf[UART_BUF_SIZE];
    unsigned char read_buf[UART_BUF_SIZE];
    int           i, cur_buf_i, temp;

    /*
     * NOTICE
     * If this is write command, there is the required arguments.
     * -> In this case use bytes for the requried arguments.
     * Else if this is read command, there is no required arguments.
     * -> In this case use bytes for storing the outputs.
     *    Because there is no required arguments in read command.
     */

    // <1st byte>: The commnad Code
    buf[0] = cmd->code;

    // <2nd byte>: The length of the code(The length of the required arguments)
    if (cmd->rw == CMD_TYPE_WRITE) {
        buf[1] = 2 + cmd->bytec; // default length + 2
    } else if (cmd->rw == CMD_TYPE_READ) {
        buf[1] = 2; // default length
    } else {
        return MSG_STATE_TYPE_ERR; // return the error code
    }

    // <3rd byte>: The command type
    buf[2] = cmd->rw;

    // <Remaining Byte(s)> for the write command: Fill the required arguments
    cur_buf_i = 3;
    if (cmd->rw == CMD_TYPE_WRITE) {
        for (i = 0; i < cmd->bytec; i++) {
            buf[cur_buf_i] = bytes[i]; // Fill the required argumnets
            cur_buf_i++;
        }
    }

    // <Last Byte>: Checksum
    for (i = 0, buf[cur_buf_i] = 0; i < cur_buf_i; i++) {
        buf[cur_buf_i] +=
            buf[i]; // sum all of the bytes except for checksum byte
    }

    // Write to the uart device (byte count: cur_buf_i + 1)
    write(uart_fd, buf, cur_buf_i + 1);

    if (cmd->rw == CMD_TYPE_READ) {
        // Read from the uart device
        // (byte count: two reserved bytes + output byte(s) count + last one
        // checksum byte)
        read(uart_fd, read_buf, 3 + cmd->bytec);

#ifndef DISABLE_OUTPUT_CHECKSUM
        // Check the checksum
        for (i = 0, temp = 0; i < 2 + cmd->bytec; i++) {
            temp += read_buf[i];
        }
        if (read_buf[2 + cmd->bytec] != (temp % 256)) {
            return MSG_STATE_CHKSUM_ERR; // return the error code
        }
#endif

        // Store the output bytes
        for (i = 0; i < cmd->bytec; i++) {
            bytes[i] = read_buf[2 + i];
        }
    }

    return MSG_STATE_SUCCESS;
}
