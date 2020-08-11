# 예제코드 1번 분석(feat. 준서)

카메라에서 입력을 받아서 LCD로 출력하는 기본예제

[toc]

## 주요 열거형 및 구조체 설명

```c
typedef enum {
    DUMP_NONE,
    DUMP_CMD,
    DUMP_READY,
    DUMP_WRITE_TO_FILE,
    DUMP_DONE
}DumpState;
```

덤프 (이미지 캡처)를 위한 정적변수 나열

___


```c
typedef struct _DumpMsg{
    long type;
    int  state_msg;
}DumpMsg;
```

덤프 작업을 하면서 필요한 변수들을 나타낸 구조체

___

```c
struct thr_data {
    struct display *disp;
    struct v4l2 *v4l2;

    DumpState dump_state;
    unsigned char dump_img_data[CAPTURE_IMG_SIZE]; // dump image size

    int msgq_id;
    bool bfull_screen; 
    bool bstream_start; 
    pthread_t threads[3];
};
```

스레드 구성과 실행에 필요한 변수들과 구조체들을 나타낸 구조체

___

## 주요 API 설명 (모두 함수)

### 카메라 데이터 처리 관련

`v4l2_open` : 영상 캡처를 위한 초기화(권한획득)

`v4l2_reqbufs` : 영상 저장할 큐 버퍼 메모리 할당

`v4l2_streamon` : 영상 캡처 시작

`v4l2_qbuf` : 영상 큐 처리 권한을 driver에게 줘서 다음 영상 프레임 요청. application에서 driver로 소유권 이전 카메라에서 영상 들어오면 driver가 queue에 데이터 저장

`v4l2_dqbuf` : 영상 큐 처리 권한을 driver에서 application이 가지고 와서 입력된 영상 프레임의 버퍼 인덱스를 사용하여 프레임 처리. driver에서 application으로 소유권 이전해서 application이 영상 queue buffer에서 자료를 꺼내는 기능

### 디스플레이 처리 관련

`disp_open` : 영상 출력 위한 초기화

`disp_get_vid_buffer` : 영상 출력을 위한 버퍼 할당

`disp_post_vid_buffer` : 영상 출력 버퍼에 영상 데이터 입력

## 기본 흐름

![image-20200812002513639](C:\Users\leeju\AppData\Roaming\Typora\typora-user-images\image-20200812002513639.png)

카메라로부터 캡처된 이미지는 해상도를 줄여주는 VPE라는 하드웨어를 통해 축소되어 LCD로 들어온다. 또한 dump 명령어를 통해 임의로 저장할 수 있다.

## 주요 함수

### int main(int argc, char **argv)

해당 함수에서 0,1,-1 등을 반환하는 경우가 있기 때문에 반환형은 int형이어야 한다.

하지만 해당 메인 함수의 실행 과정에서 전달되는 인자가 없기 때문에 전달 인자의 자료형은 굳이 있을 필요는 없을 것 같다.

```c
int main(int argc, char **argv)
{
    struct display *disp; //display 형식의 구조체 포인터 변수 선언
    struct v4l2 *v4l2; //v4l2 형식의 구조체 포인터 변수 선언
    struct thr_data tdata; //thr_data 형식의 tdata 구조체 선언
    int disp_argc = 3; //디스플레이 구조체를 생성하는 데 필요한 멤버의 개수
    char* disp_argv[] = {"dummy", "-s", "4:480x272", "\0"}; // 해당 문자열들의 배열들의 원소에 대한 주소들의 배열
    int ret = 0; //반환 값 변수 선언
    uint32_t fourcc; //32비트 변수 선언(글자 수가 4개인 문자열 저장하려고)
    printf("-- 1_camera_dump_disp example Start --\n"); //예제코드 시작됨을 알림

    tdata.dump_state = DUMP_NONE; //tdata의 멤버인 dump_state에 현재 캡처 이미지를 덤프하지 않는 것으로 초기화 시킴.
    memset(tdata.dump_img_data, 0, sizeof(tdata.dump_img_data)); //메모리 세팅: tdata의 멤버인 dump_img_data를 모두 0으로 바꾼다.

    disp = disp_open(disp_argc, disp_argv); //disp라는 구조체 포인터 변수에 저장되어 있는 주소를 disp_open 함수에 의한 반환값으로 한다. (영상출력을 위한 권한 획득)
    if (!disp) {
        ERROR("disp open error!"); //disp_open함수는 에러 발생시 NULL 값만이 들어있는 구조체를 반환한다. 그 중 하나라도 NULL이 아닐 경우 1을 반환하고 main thread 종료.
        return 1;
    }

    fourcc = getfourccformat(CAPTURE_IMG_FORMAT); //"uyvy" 각각의 문자를 하나의 변수에 저장.
    v4l2 = v4l2_open(fourcc, CAPTURE_IMG_W, CAPTURE_IMG_H); //v4l2_open함수로 영상 캡처를 위한 권한 획득과 초기화. 인자로 전달되고 있는 것들을 잘 보자.
    if (!v4l2) {
        ERROR("v4l2 open error!"); //마찬가지로 v4l2_open함수는 구조체를 반환한다. 잘 작동 될 경우 구조체의 모든 멤버의 값이 NULL이 됨.
        disp_close(disp);
        return 1;
    }


    MSG ("Input(Camera) = %d x %d (%.4s)\nOutput(LCD) = %d x %d (%.4s)",
        CAPTURE_IMG_W, CAPTURE_IMG_H, (char*)&fourcc,
        CAPTURE_IMG_H, CAPTURE_IMG_H, (char*)&fourcc); //코딩 오류이다. 카메라와 LCD의 해상도는 모두 1280x720

    tdata.disp = disp; //tdata 구조체의 멤버인 disp구조체에 초기화한 disp 구조체를 할당
    tdata.v4l2 = v4l2; //tadaa 구조체의 멤버인 v4l2구조체에 초기화한 v4l2 구조체를 할당
    tdata.bfull_screen = true; //전체화면을 사용할 것임을 명시
    tdata.bstream_start = false; // 카메라 스트림 스타트를 위해 false 값 전달

    // 메시지큐 사용을 위해 key를 할당 받고 사용할 큐의 ID 획득
    if(-1 == (tdata.msgq_id = msgget((key_t)DUMP_MSGQ_KEY, IPC_CREAT | 0666))) {
        fprintf(stderr, "%s msg create fail!!!\n", __func__);
        return -1;
    }

    pexam_data = &tdata;

    ret = pthread_create(&tdata.threads[0], NULL, capture_thread, &tdata); //capture_thread가 실행되는 서브스레드를 하나 만든다.
    if(ret) {
        MSG("Failed creating capture thread");
    }
    pthread_detach(tdata.threads[0]); //capture_thread를 메인 스레드로부터 독립시킨다. 

    ret = pthread_create(&tdata.threads[1], NULL, capture_dump_thread, &tdata); //capture_dump_thread가 실행되는 서브스레드를 하나 만든다.
    if(ret) {
        MSG("Failed creating capture dump thread");
    }
    pthread_detach(tdata.threads[1]); //capture_dump_thread를 메인 스레드로부터 독립시킨다.

    ret = pthread_create(&tdata.threads[2], NULL, input_thread, &tdata); //input_thread가 실행되는 서브스레드를 하나 만든다.
    if(ret) {
        MSG("Failed creating input thread");
    }
    pthread_detach(tdata.threads[2]); //input_thread를 메인 스레드로부터 독립시킨다. 


    /* register signal handler for <CTRL>+C in order to clean up */
    //컨트롤 씨를 하면 모든 스레드가 종료되도록 한다.
    if(signal(SIGINT, signal_handler) == SIG_ERR) {
        MSG("could not register signal handler");
        closelog();
        exit(EXIT_FAILURE);
    }

    pause();

    return ret;
}
```

### capture_thread

카메라에서 프레임들을 메세지 큐에 저장만 하는 스레드

### capture_dump_thread

명령어를 입력하면 해당 시간의 프레임을 사진 파일로 저장하는 스레드

### input_thread

대기하고 있다가 dump라는 명령어가 입력되면 이를 capture_dump_thread에 전달하고 사진 저장까지 완료되면 저장이 완료되었다고 메시지를 주는 스레드

### signal_handler

사용자가 Ctrl + C를 입력하여 해당 함수에 SIGINT 인자가 전달되면 모든 스레드와 함수를 종료시키고 예제코드 종료 메시지를 반환한다. 



