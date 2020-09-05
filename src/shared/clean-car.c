#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <termios.h>

#include "config-car.h"
#include "util.h"
#include <fcntl.h>

#define BAUDRATE B19200
#define SERIAL_DEVICE "/dev/ttyS2"

void clean_car()
{
    struct termios newtio;
    int fd;

    /* UART configuration */
    if ((fd = open(SERIAL_DEVICE, O_RDWR | O_NOCTTY)) < 0)
    {
        ERROR("Serial %s Device Error", SERIAL_DEVICE);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD; // | CRTSCTS;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; // inter-character timer unused
    newtio.c_cc[VMIN] = 1;  // blocking read until 8 chars received

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    unsigned char buf[8];

    buf[0] = 0x90;
    buf[1] = 3; // default length + 2
    buf[2] = 1;
    buf[3] = 0;
    buf[4] = 0;
    for (int i = 0; i < 4; i++)
    {
        buf[4] += buf[i]; // sum all of the bytes except for checksum byte
    }

    // Write to the uart device (byte count: cur_buf_i + 1)
    write(fd, buf, 5);
}

int del_msgq_ctrlboard()
{
    int id_msgq = 999;

    if ((id_msgq = msgget(KEY_MSGQ_CTRLBOARD, 0)) == -1)
    {
        return -1;
    }

    if (msgctl(id_msgq, IPC_RMID, NULL) == -1)
    {
        printf("%d\n", id_msgq);
        return -1;
    }

    return 0;
}

int del_shm_recognize()
{
    int id_shm;

    if ((id_shm = shmget(KEY_SHM_RECOGNIZE, 0, 0)) == -1)
    {
        return -1;
    }

    if (shmctl(id_shm, IPC_RMID, NULL) == -1)
    {
        return -1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    clean_car();

    // delete message queue of ctrlboard
    if (del_msgq_ctrlboard() == 0)
    {
        MSG("Success to delete message queue of ctrlboard");
    }
    else
    {
        MSG("Fail to delete message queue of ctrlboard");
    }

    // delete shared memory of recognize
    if (del_shm_recognize() == 0)
    {
        MSG("Success to delete shared memory of recognize");
    }
    else
    {
        MSG("Fail to delete shared memory of recognize");
    }

    return 0;
}