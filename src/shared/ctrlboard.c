#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <pthread.h>

#include "ctrlboard.h"
#include "util.h"

#define UART_BUF_SIZE   8
#define BAUDRATE        B19200
#define SERIAL_DEVICE   "/dev/ttyS2"  // ttyHS0, ttyHS1, ttyHS3 are available

int ctrlboard_init(void);
ctrlboard_msg_state_t command_ctrlboard(ctrlboard_cmd_t* cmd, char* bytes);
void* thread_msg_processing(void* args);
void signal_handler(int sig);

static int uart_fd;
static int msgq_key;
static int msgq_id;

int main(int argc, char** argv) {
    /* Get the arguments from the command line. */
    if (argc != 2) {
        printf("usage %s [message queue key] \n", argv[0]);
        return 1;
    }
    msgq_key = atoi(argv[1]);   // 1st argv: get message queue key

    /* Initialize to communicate with control board */
    if (ctrlboard_init() != 0) {
        ERROR("Cannot iniailize control borad.");
        return -1;
    }

    /* Initialize to communicate other processes by message queue */
    // WARNING: there is the IPC_EXCL option.
    msgq_id = msgget((key_t)msgq_key, IPC_CREAT | IPC_EXCL | 0666);
    if (msgq_id == -1) {
        ERROR("Cannot get message queue id with the key(%d). "
              "Please check ipcs commnand and remove the message queue.",
              msgq_key);
        return -1;
    }

    /* Initialize thread to recieve messages */
    pthread_t threads[1];
    if (pthread_create(&threads[0], NULL, thread_msg_processing, NULL)) {
        ERROR("Cannot create thread to recieve messages");
        return -1;
    }
    pthread_detach(threads[0]);
    
    /* Register interrupt(CRTL+C) handler */
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        ERROR("Cannot register signal handler");
        return -1;
    }

    pause();

    return 0;
}

int ctrlboard_init(void) {
    struct termios newtio;
    char *fd_serial = SERIAL_DEVICE;
    int addr = 0x4b;
    int ret;

    /* UART configuration */
    uart_fd = open(fd_serial, O_RDWR | O_NOCTTY);
    if (uart_fd < 0) {
        ERROR("Serial %s Device Error", fd_serial);
        return 1;
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD; // | CRTSCTS;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;     // inter-character timer unused
    newtio.c_cc[VMIN] = 1;      // blocking read until 8 chars received

    tcflush(uart_fd, TCIFLUSH);
    tcsetattr(uart_fd, TCSANOW, &newtio);
    
    MSG("UART Device %s has been initialized.", SERIAL_DEVICE);
    return 0;
}

void* thread_msg_processing(void* args) {
    ctrlboard_msg msg;
    size_t msg_size = sizeof(ctrlboard_msg) - sizeof(long);

    for (;;) {
        // Wait until receive a message. (block state)
        if (msgrcv(msgq_id, (void*)&msg, msg_size, 0, 0) != -1) {
            // Command to the control board and get the return value.
            msg.state = command_ctrlboard(&msg.cmd, msg.bytes);
            // Send the message if the message queue is availiable. (block state)
            msgsnd(msgq_id, (void*)&msg, msg_size, 0); // wait(block)
        }
    }
}

ctrlboard_msg_state_t command_ctrlboard(ctrlboard_cmd_t* cmd, char* bytes) {
    unsigned char buf[UART_BUF_SIZE];
    unsigned char read_buf[UART_BUF_SIZE];
    int i, cur_buf_i, temp;

    /* 
     * NOTICE
     * If this is write command, there is the required arguments.
     * -> In this case use ctrlboard_cmd_t.bytes for the requried arguments.
     * Else if this is read command, there is no required arguments.
     * -> In this case use ctrlboard_cmt_t.bytes for storing the outputs.
     *    Because there is no required arguments in read command.
     */

    // <1st byte>: The commnad Code
    buf[0] = cmd->code;

    // <2nd byte>: The length of the code(The length of the required arguments)
    if (cmd->rw == CMD_TYPE_WRITE) {
        buf[1] = 2 + cmd->bytec;      // default length + 2
    } else if (cmd->rw == CMD_TYPE_READ) {
        buf[1] = 2;                 // default length
    } else {
        return MSG_STATE_TYPE_ERR;  // return the error code
    }

    // <3rd byte>: The command type
    buf[2] = cmd->rw;

    // <Remaining Byte(s)> for the write command: Fill the required arguments
    cur_buf_i = 3;
    if (cmd->rw == CMD_TYPE_WRITE) {
        for (i = 0; i < cmd->bytec; i++) {
            buf[cur_buf_i] = bytes[i];   // Fill the required argumnets
            cur_buf_i++;
        }
    }

    // <Last Byte>: Checksum
    for (i = 0, buf[cur_buf_i] = 0; i < cur_buf_i; i++) {
        buf[cur_buf_i] += buf[i];   // sum all of the bytes except for checksum byte
    }

    // Write to the uart device (byte count: cur_buf_i + 1)
    write(uart_fd, buf, cur_buf_i + 1);

    if (cmd->rw == CMD_TYPE_READ) {
        // Read from the uart device
        // (byte count: two reserved bytes + output byte(s) count + last one checksum byte)
        read(uart_fd, read_buf, 3 + cmd->bytec);

        #ifndef DISABLE_OUTPUT_CHECKSUM
        // Check the checksum
        for (i = 0, temp = 0; i < 2 + cmd->bytec; i++) {
            temp += read_buf[i];
        }
        if (read_buf[2 + cmd->bytec] != (temp % 256)) {
            return MSG_STATE_CHKSUM_ERR;    // return the error code
        }
        #endif

        // Store the output bytes
        for (i = 0; i < cmd->bytec; i++) {
            bytes[i] = read_buf[2 + i];
        }
    }
    
    return MSG_STATE_SUCCESS;
}

void signal_handler(int sig) {
    /*
     * NOTICE
     * Because this catches SIGINT(2), parameters of kill commnad in shell script
     * MUST include '-2'.
     */
    if (sig == SIGINT) {
        msgctl(msgq_id, IPC_RMID, NULL); // Delete Message Queue
        MSG("Message queue had been deleted(key: %d, id: %d).", msgq_key, msgq_id);
    }
}
