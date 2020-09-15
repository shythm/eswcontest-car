#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <termios.h>

#include "config-car.h"
#include "util.h"
#include <fcntl.h>

#define BAUDRATE      B19200
#define SERIAL_DEVICE "/dev/ttyS2"

int stop_car() {
    struct termios newtio;
    int            fd;

    /* UART configuration */
    if ((fd = open(SERIAL_DEVICE, O_RDWR | O_NOCTTY)) < 0) return -1;
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag     = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag     = IGNPAR;
    newtio.c_oflag     = 0;
    newtio.c_lflag     = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN]  = 1;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    // Write stop commend to UART
    unsigned char buf[8] = {
        0x91, 4, 1, 0, 0, 0x96,
    };
    write(fd, buf, 6);
    buf[0] = 0x90;
    buf[1] = 3;
    buf[4] = 0x94;
    write(fd, buf, 5);
    return 0;
}

int del_ipcs() {
    int id_snd, id_rcv, id_shm, r = 0;

    if ((id_snd = msgget(KEY_MSGQ_CTRLBOARD, 0)) == -1) r = -1;
    if (msgctl(id_snd, IPC_RMID, NULL) == -1) r = -1;
    if ((id_rcv = msgget(KEY_MSGQ_CTRLBOARD_RCV, 0)) == -1) r = -1;
    if (msgctl(id_rcv, IPC_RMID, NULL) == -1) r = -1;
    if ((id_shm = shmget(KEY_SHM_RECOGNIZE, 0, 0)) == -1) r = -1;
    if (shmctl(id_shm, IPC_RMID, NULL) == -1) r = -1;

    return r;
}

int main(int argc, char **argv) {
    if (stop_car() != 0) MSG("Could not stop car");
    if (del_ipcs() != 0) MSG("Fail to delete ipcs");

    return 0;
}