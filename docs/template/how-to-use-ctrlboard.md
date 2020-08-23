# ctrlboard 사용법

## 개요
- 제어보드에 직접적으로 접근하는 프로세스
- **제어보드의 자원을 여러 스레드/프로세스가 동시에 점유하게 되면 읽기 명령의 목적지가 불문명해지는 문제가 존재한다.**
    - 이러한 문제를 해결하기 위해 ctrlboard 프로세스를 따로 만들었다. 그리고 명령을 message queue에 넣어 순차적으로 실행시키고 message id를 통해 읽기 명령의 결과물을 전달할 위치를 명확하게 지정해주었다.
- ctrlboard가 일종의 서버 역할을 하며, 클라이언트는 `ctrlboard-lib.h`를 include 해서 ctrlboard와 message queue 통신을 수행한다.
    - 따라서 ctrlboard 프로세스가 실행되어야 제어 보드에 명령을 내릴 수 있다.

## 기본 셋팅
1. `#include "ctrlboard-lib.h"` 매크로를 추가한다.
1. ctrlboard 프로세스를 실행할 때 사용한 message queue key 값을 가지고 있어야 한다.
    - 이를 위해서 shell을 통해 외부로부터 messag queue key 값을 인자로 받아온다.
    ``` c
    int main(int argc, char** argv) {
        key_t msgq_key; // 'sys/ipc.h' and `sys/msg.h` must be included.

        if (argc != 2) {
            printf("Usage %s: [Message queue key of ctrlboard process] \n", argv[0]);
        }
        msgq_key = (key_t)atoi(argv[1]); // 'stdlib.h' must be included.
    }
    ```
1. `msgget` 함수를 이용해서 message queue id를 얻는다.
    - message queue id는 ctrlboard 프로세스에 메시지를 보내는 `message_ctrlboard` 함수에 사용된다.
1. `message_ctrlboard` 함수를 이용해서 ctrlboard에 메시지를 보낸다.

## message_ctrlboard 함수 설명
``` c
ctrlboard_msg_state_t message_ctrlboard
(int msgqid, long msgid, ctrlboard_cmd_code code, ctrlboard_cmd_rw rw, unsigned char bytec, ctrlboard_byte_container* data);
```
- 첫 번째 인자: ctrlboard의 message queue id
- 두 번째 인자: message를 구분할 message id
    - message queue에서 message를 구분하는데 사용한다. 따라서 `message_ctrlboard` 함수를 사용하는 서로 다른 함수는 `msgid`가 각각 달라야 한다.
- 세 번째 인자: 제어보드 명령어
    - `ctrlboard-lib.h`의 `ctrlboard_cmd_code`를 참조바람.
- 네 번째 인자: 쓰기 명령일 때에는 `CMD_TYPE_WRITE` 지정, 읽기 명령을 내릴 때에는 `CMD_TYPE_READ` 지정.
- 다섯 번째 인자: 쓰기 명령일 때에는 보낼 바이트 수, 읽기 명령일 때에는 받을 바이트 수. (바이트 수에 대한 정보는 `/docs/reference/지능형 무인 자동차 스펙.pdf` 참조)
- 여섯 번째 인자: `ctrlboard_byte_container`의 주소를 받아 쓰기 명령일 때에는 이 주소에 있는 값을 쓰기 메시지의 인자로 사용한다. 그리고 읽기 명령일 때에는 제어 보드로부터 받은 데이터를 해당 주소를 참조하여 저장한다.
    - `ctrlboard_byte_container` 타입은 union(공용체)로 구성되어 있어, 제어보드 명령어 마다 필요한 인자의 크기를 모두 지원할 수 있게끔 구현되어 있다.
    - 만약, 제어보드 명령어가 signed 16bit integer의 인자를 필요로 할 때, `ctrlboard_byte_container`의 `container_int16` 필드에 정보를 담으면 된다.
- 반환 값: `ctrlboard-lib.h`의 `ctrlboard_msg_state_t` 중에 하나를 반환한다.
    - `MSG_STATE_SUCCESS`: 메시지 주고받기 성공 상태
    - `MSG_STATE_TYPE_ERR`: 네 번째 인자에 문제가 있을 때 발생하는 오류 코드
    - `MSG_STATE_CHKSUM_ERR`: 읽기 명령 시 제어 보드로 부터 받은 데이터의 checksum이 일치하지 않을 때 발생하는 오류 코드

## 쓰기 메시지 예시
``` c
int msgq_id = msgget(100, 0); // 첫 번째 인자에 messagq queue key 넣기
int msg_id = 't' + 'e' + 's' + 't';
ctrlboard_byte_container container;

// SPEED_CONTROL 제어
container.container_uint8 = 1; // SPEED_CONTROL 명령에 사용되는 인자의 byte 수는 1개 -> container_uint8에 데이터 삽입.
if (message_ctrlboard(msgq_id, msg_id, CMD_SPEED_CONTROL_ON_OFF, CMD_TYPE_WRITE, 1, &container) == MSG_STATE_SUCCESS) { // ctrlboard_byte_container 공용체의 주소값을 전달해야 한다.
    printf("SPEED_CONTROL를 ON으로 제어 성공! \n");
} else {
    printf("SPEED_CONTROL 제어 실패. \n");
}

// DESIRE_SPEED 제어
container.container_int16 = 300; // DESIRE_SPEED 명령에 사용되는 인자의 byte 수는 2개 -> container_int16에 데이터 삽입.
if (message_ctrlboard(msgq_id, msg_id, CMD_DESIRE_SPEED, CMD_TYPE_WRITE, 2, &container) == MSG_STATE_SUCCESS) {
    printf("DESIRE_SPEED 제어 성공! \n");
} else {
    printf("DESIRE_SPEED 제어 실패. \n");
}
```

## 읽기 메시지 예시
``` c
int msgq_id = msgget(100, 0); // 첫 번째 인자에 messagq queue key 넣기
int msg_id = 'i' + 'r';
ctrlboard_byte_container container;

// 라인 센서값 가져오기
if (message_ctrlboard(msgq_id, msg_id, CMD_LINE_SENSOR, CMD_TYPE_READ, 1, &container) == MSG_STATE_SUCCESS) { // LINE_SNESOR 명령어는 1 바이트의 결과를 반환한다.
    printf("라인 센서값을 받아 왔습니다.\n");
    printf("%d", container.container_uint8); // 1 바이트의 결과를 받아오기 때문에 container_uint8 필드를 이용해서 데이터를 읽어온다.
} else {
    printf("라인 센서값을 받아오지 못했습니다.\n");
}
```

## union(공용체)의 활용
``` c
#define MAX_CMD_BYTE_CNT  4

typedef union _ctrlboard_byte_container {
    char            bytes[MAX_CMD_BYTE_CNT];
    unsigned char   container_uint8;            // unsigned 8bit integer
    short           container_int16;            // signed   16bit integer
    int             container_int32;            // signed   32bit integer
} ctrlboard_byte_container;
```

- 위의 공용체는 `ctrlboard-lib.h`에 선언되어 있다.
- 공용체란 공용체에 정의되어 있는 서로 다른 자료형들이 같은 메모리 공간을 사용할 수 있도록 해주는 c언어 예약어이다.
    - 위의 예시에서, 그리고 우리가 사용하는 임베디드 보드 에서는, LSB가 char형 배열에 가장 첫 번째 element에 저장되고, MSB는 char형 배열에 가장 마지막 element에 저장된다.
    - 따라서 제어 보드에 명령을 내릴 때 char형 배열의 0번째 index 부터 (필요한 인자 수 - 1)번째 index까지 보내면 된다.
- 공용체를 사용한 이유는 제어 보드에 명령을 내릴 때 필요한 인자의 수(바이트 수)가 각각 다르기 때문에 제어 보드로부터 데이터를 읽거나 제어 보드에 쓸 때 이에 맞춰 주어야 했다.
    - 예를 들어서, 16bit signed integer를 전송하기 위해 첫 번째 바이트에는 하위 8bit, 두 번째 바이트에는 상위 8bit를 bit shift 연산자를 이용해 전송해야 했다.
- 그래서 공용체를 이용해 필요한 인자의 수에 해당하는 자료형을 공용체에 넣어줌으로써 8bit 씩 나누어 전송하기 편하게 만들었다.