#include "ctrlboard-lib.h"
#include "recognize-lib.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> //표준 라이브러리
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h> //usleep등

int main(int argc, char **argv) {
    int msgq_id;

    if (get_msgq_id_ctrlboard(&msgq_id, 0) == -1) {
        printf("메시지 큐 아이디 얻기 실패\n");
        return 1;
    }

    //현재의 라인 센서 값들을 보여주는 부분
    ctrlboard_byte_container container;
    uint8_t bitmask = 0x80; //이진수 1000 0000으로 초기화함.
    for (;;) {
        if (message_ctrlboard(msgq_id, 0103, CMD_LINE_SENSOR, CMD_TYPE_READ, 1,
                              &container) == MSG_STATE_SUCCESS) {
            bitmask = 0x80;
            printf("라인센서값: ");
            for (int i = 0; i < 8; i++) {
                if (container.c_uint8 &
                    bitmask) { // 맨 왼쪽부터 마스킹 해서 검사. 마스킹 결과가
                               // 1일 때(흰색)
                    printf("1 ");
                } else {
                    printf("0 "); //마스킹 결과가 0일 때(검은색)
                }
                bitmask = bitmask >> 1;
            }
            printf("\n");
        } else {
            printf("라인센서값 얻어오기 실패!\n");
        }

        usleep(100000); // 0.1초에 한번씩 받아올거임.
    }
    return 1;
}