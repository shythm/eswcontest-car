# 5_opencv_disp 코드 분석

* 그림 파일을 불러와서 OpenCV로 처리한 다음 LCD에 결과를 출력하는 예제
* 그리고 OpenCV 처리 시간을 LCD로 출력



## 용어 정리

### 버퍼(Buffer)

데이터를 한 곳에서 다른 한 곳으로 전송하는 동안 일시적으로 그 데이터를 보관하는 메모리의 영역이다.

* 버퍼링(Buffering): 버퍼를 활용하는 방식 또는 버퍼를 채우는 동작을 말한다.
* 큐(Queue)로 구현한다.
  * 그렇다면, 정보를 보내는 쪽에는 Queue에 데이터를 집어넣는 동작만 수행하고, 정보를 받는 쪽에는 Queue에서 데이터를 가져가는 동작(Dequeue)만 수행하면 되는거군.
* 컴퓨터 안의 프로세스 사이에서 데이터를 이동시킬 때 사용한다.

### Dump

코어 덤프(Core Dump), 메모리 덤프(Memory Dump), 시스템 덤프(System Dump)라고도 부르며 컴퓨터 프로그램이 특정 시점에 작업 중이던 메모리 상태를 기록한 것이다.

* 보통 프로그램이 비정상적으로 종료되었을 때 만들어지는 파일이다.
  * 하지만 예제 코드에서는 dump를 현재 화면에 출력되는 이미지 저장이라는 의미로 사용하고 있다.

### v4l2

이 예제에서는 사용하지 않지만, 다른 예제에서는 v4l2를 사용하길래 조사해보았다. Video4Linux(v4l)의 2번째 버전으로, 리눅스 시스템에서 실시간 비디오 촬영을 위한 드라이버와 API을 모아둔 것이다.

* https://en.wikipedia.org/wiki/Video4Linux
* USB 웹캠, TV 튜너 등등 카메라와 관련된 장비들을 지원하고 표준화된 출력을 제공해서 프로그래머가 리눅스 시스템에 비디오 촬영 기능을 간단히 구현할 수 있다.
* OpenCV, Skype, PyGame, VLC Media Player 등등이 v4l2를 지원한다.
  * 그래서 OpenCV를 하기 전에 v4l2를 이용해서 카메라 데이터를 불러온 것이다.

### DRM

https://en.wikipedia.org/wiki/Direct_Rendering_Manager

예제의 `display-kms.h`나 `display-kms.c` 파일을 보면 DRM 관련 헤더 파일(`drm.h`, `omap_drm.h`, `omap_drmif.h`)을 include 하는 것을 볼 수 있다. DRM은 Direct Rendering Manager의 약자로 DRM(Digital right management)와는 전혀 다른 개념이다. 처음에 후자의 뜻인 줄 알고 많이 난감했었다. DRM은 최신 비디오 카드의 GPU와의 인터페이스를 담당하는 리눅스 커널의 서브 시스템이다.

* DRM을 이용하면 커널 단계에서가 아닌 유저 모드의 프로그램이 GPU에 데이터와 명령을 전달할 수 있으며, 디스플레이의 모드를 설정하는 것 등의 작업을 수행할 수 있다.
  * 따로 시스템 콜을 필요로 하지 않고, DRM에 의해 감지된 GPU를 `/dev` 디렉터리 안의 파일로 관리한다(Unix의 원칙인 everything is a file을 지켰다고 함).
    * `/dev/dri/cardX` (where X is a sequential number) 파일이라고 함.
  * 유저 모드에서 GPU에게 명령을 내리려면 이 해당 DRM device(`/dev/dri/cardX`) 파일을 열어야 한다.
* 이전에는 유저 모드의 프로그램이 직접 비디오 카드에 접근해서 두 개 이상의 프로그램이 이 비디오 카드 자원을 점유할 때 문제가 많이 발생했다.
* 하지만 DRM의 등장으로 인해 여러 프로그램이 비디오 카드의 자원을 서로 협력하여 사용할 수 있게 되었다.
  * DRM을 통해 배타적인 GPU 접근이 가능하다.
  * DRM은 명령 큐, 메모리, 다른 하드웨어 자원을 초기화하고 관리한다.
  * DRM이 프로그램들 간에 중재자 역할을 해준다.
* `libdrm`에 DRM 관련된 헤더 파일이 있음(우리 모형 자동차에는 `/usr/include/libdrm`, WSL에서는 `/usr/include/drm`)
* DRM은 DRM core, DRM driver의 두 부분으로 나뉜다.
  * DRM core: 서로 다른 GPU를 지원하는 DRM driver의 기본적인 프레임워크이다. hardware-independent하다.
  * DRM driver: hardware-dependent하며 특정 GPU를 지원하는 구현체이다. DRM core의 기능 이외에 특정 그래픽 카드에서만 사용할 수 있는 기능을 지원한다.

### KMS

* Mode Setting(Kernel Mode Setting): 디스플레이 컨트롤러의 디스플레이 모드(화면 해상도, 색심도, 주사율)를 활성화하는 소프트웨어 작업을 말한다.

  * 그래픽 카드가 정상적으로 작동하려면 mode setting을 꼭 해주어야 한다. 물론, 해당 디스플레이가 지원되는 범위에서 mode setting을 해야 한다.
  * framebuffer를 사용하기 전에 꼭 mode setting을 해야 한다. framebuffer은 디스플레이에 표시될 내용을 담는 메모리이자 버퍼이다.

* KMS Device Model

  * 위키백과(https://en.wikipedia.org/wiki/Direct_Rendering_Manager)와 https://prographics.tistory.com/1 블로그 참조

  * CRTC(CTR Controller): 스캔 아웃 버퍼(framebuffer)에 있는 픽셀 데이터를 읽고 비디오 모드 타이밍 신호를 생성한다. 사용 가능한 CTRC의 수는 하드웨어가 동시에 처리할 수 있는 독립 출력 장치의 수를 결정하고 디스플레이 당 하나 이상의 CRTC가 필요하다.
  * Encoders: CTRC로부터 생성된 비디오 신호를 Connector에 적합한 포맷으로 Encoding 하는 역할을 한다.
  * Connectors: CTRC에 의해 스캔 아웃된 비디오 신호를 표시할 위치를 나타낸다. 일반적으로 출력장치(모니터, 패널)가 있는 하드웨어의 물리적 커넥터(VGA, DVI, HDMI)를 나타낸다.
  * Planes: 하드웨어 블록이 아니라 스캔아웃엔진(CTRC)이 공급되는 버퍼를 포함하는 메모리 개체이다. framebuffer는 primary plane이라고도 불린다. CTRC가 display mode를 결정하는 소스이기 때문에 각 CRTC에는 연결된 하나의 plane이 꼭 있어야 한다.

* KMS외에 User-space Mode Setting(UMS)도 있음.



## 메시지 큐(Message Queue) 개념 및 관련 함수

이 예제에서 사용하는 `msgget`, `msgsnd`, `msgrcv` 함수는 모두 메시지 큐와 관련된 함수이다. 메시지 큐는 Linux에서 사용되는 IPC(InterProcess Communication) 방법 중 하나이다.

* 아래의 내용은 https://blog.naver.com/57gate/60150005931 블로그에서 발췌한 내용이다.

### 주고 받을 데이터 정하기

* 프로그램 간에 어떤 데이터를 주고 받을 것인지 구조체 형태로 정해주어야 한다. 아래는 우리 예제에서 사용하는 구조체이다.

  ```c
  typedef struct _DumpMsg{
      long type;
      int  state_msg;
  } DumpMsg;
  ```

* 주의할 점은, 첫 번째 필드는 반드시 `long` 타입의 변수여아 한다. 일종의 식별자 역할을 한다.

* 그 아래에 있는 필드들은 실제로 사용하려고 하는 데이터 구조를 나타낸다.

### 메시지 큐 ID 얻기

```c
int msgget(key_t key, int msgflg);
```

* `msgget` 함수를 이용해서 메시지 큐 ID를 얻을 수 있다. 이 메시지 큐 ID는 **메시지 큐를 통해 메시지를 주고 받을 때 식별자로 사용**한다. 아래는 예제에서 발췌한 내용이다.

  ```c
  if(-1 == (tdata.msgq_id = msgget((key_t)DUMP_MSGQ_KEY, IPC_CREAT | 0666))) {
      fprintf(stderr, "%s msg create fail!!!\n", __func__);
      return -1;
  }
  ```

* 첫 번째 인자로는 key 값을 넣는다. 큐 자체의 식별 번호라고 한다.

* 두 번째 인자로는 `IPC_CREATE`나 `IPC_EXCL`를 넣을 수 있다.

  * `IPC_CREATE`: 새롭게 생성된 메시지 큐의 메시지 큐 확인자를 반환하거나, 같은 key 값을 가진 이미 존재하는 큐의 확인자를 반환한다.
  * `IPC_EXCL`: 해당 key에 대한 메시지 큐가 이미 존재하면 실패 처리한다.
  * `IPC_CREATE`와 `IPC_EXCL`을 OR 연산하여 사용하면 새로운 큐가 만들어지거나 큐가 존재하면 -1을 반환시키며 호출에 실패한다. `IPC_EXCL`은 그 자체로는 쓸모가 없지만, `IPC_CREATE`와 함께 사용한다.
  * IPC 객체는 UNIX 파일 시스템 상의 파일 permission 속성을 가진다. 따라서 부가적으로 permission을 추가할 수도 있다(위의 예제에서 `0666`을 OR 연산한 것을 주목).
  * 두 번째 인자에 대한 내용은 http://jullio.pe.kr/cs/lpg/lpg_6_4_2_3.html 사이트를 참고함.

* 반환값: 성공 시 메시지 큐 ID 반환, 실패 시 -1 반환

  * 우리 예제는 `thr_data` 구조체 안에 `msgq_id` 라는 변수에다가 생성된 메시지 큐 ID를 넣어서 Thread끼리 공유하고 있음.

### 메시지 보내기

```c
int msgsnd(int msqid, struct msgbuf *msgp, int msgsz, int msgflg);
```

* `msgsnd` 함수를 이용해서 메시지 큐에 메시지를 넣을 수 있다. 아래는 예제에서 발췌한 내용이다.

  ```c
  DumpMsg dumpmsg;
  dumpmsg.type = DUMP_MSGQ_MSG_TYPE;
  dumpmsg.state_msg = DUMP_CMD;
  if (-1 == msgsnd(data->msgq_id, &dumpmsg, sizeof(DumpMsg)-sizeof(long), 0)) {
      printf("dump cmd msg send fail\n");
  }
  ```

* 첫 번째 인자로는 메지시 큐 ID를 전달한다.

* 두 번째 인자로는 보내려고 하는 메시지(구조체 변수)의 주소를 전달한다.

* 세 번째 인자로는 메시지의 크기를 전달한다.

  * 이때 중요한 점은 **구조체의 첫 번째 필드의 크기는 빼야한다**.

* 네 번째 인자로는 `0` 또는 `IPC_NOWAIT`를 넣을 수 있다.

  * `0`: 무시
  * `IPC_NOWAIT`: 원래는 메시지 큐가 가득 찼을 때 메시지 큐가 비워질 때까지(메시지 큐에 메시지를 쓸 수 있을 때까지)기다리는데(Block 상태, 다음 줄의 명령이 실행되지 않음), 이 인자를 전달하면 기다리지 않고 계속 실행된다(즉, 통제권이 호출한 프로세스에게 돌아간다).
  
* 반환값: 성공했으면 0, 실패했으면 -1

### 메시지 꺼내기

```c
ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
```

* `msgrcv` 함수를 이용해서 메시지 큐에 있는 메시지를 받을 수 있다. 아래는 예제에서 발췌한 내용이다.

  ```c
  if(msgrcv(data->msgq_id, &dumpmsg, sizeof(DumpMsg)-sizeof(long), DUMP_MSGQ_MSG_TYPE, 0) >= 0) {
  ```

* 첫 번째 인자로는 메시지 큐 ID를 전달한다.

* 두 번째 인자로는 전달 받을 메시지(메시지 구조체 변수, 위의 예제에서는 `DumpMsg` 변수)의 주소를 전달한다.

* 세 번째 인자로는 전달 받을 메시지의 크기를 전달한다.

  * 마찬가지로 이때 중요한 점은 **구조체의 첫 번째 필드의 크기는 뺴야한다**.

* 네 번째 인자로는 메시지 큐에 있는 자료 중에 어떤 자료를 읽어 들일지에 대한 옵션이다(**이는 메시지 구조체의 첫 번째 필드로 식별한다**).

  * 0: 큐에 자료가 있다면 첫 번째의 자료를 읽어 들인다.
  * 양수: 메시지 구조체의 첫 번째 필드와 양수로 지정한 값과 같은 자료 중 첫 번째를 읽어들인다.
  * 음수: 메시지 구조체의 첫 번째 필드(`long type`)을 `data_type`이라고 하자. 음수 값을 절댓값으로 변경하고, 이 절댓값과 같거나 보다 제일 작은 `data_type`의 자료를 구한다.

* 다섯 번째 인자로는 `0`, `IPC_NOWAIT`, `MSG_NOERROR`를 넣을 수 있다.
  * `0`: 무시
  * `IPC_NOWAIT`: 이름에서도 알 수 있듯이 메시지를 받을 때까지 기다리지 않는다. 즉 메시지를 받을 때까지 Block 상태가 되는 것을 막으려고 주는 옵션이다.
    * 메시지 큐에 메시지가 없다면 기다리지 않고 -1을 반환한다.
  * `MSG_NOERROR`: 메시지 큐에 있는 자료가 준비된 데이터 크기보다 크다면 초과 부분을 잘라 내고 읽어 있는 부분만 담아 옮긴다. 이 옵션이 없다면 메시지 큐에 자료가 있다고 하더라도 -1로 실패를 의미하는 값이 반환된다.

* 네 번째와 다섯 번째 인자에 대한 정보는 http://forum.falinux.com/zbxe/index.php?document_srl=420636&mid=C_LIB를 참고했다.



## Thread 관련 함수

아래의 내용은 모두 https://bitsoul.tistory.com/157 블로그에서 발췌한 내용이다. Thread 맛집 블로그임.

### pthread_create

```c
int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg)
```

<u>Thread를 만드는 함수</u>

* 첫 번째 매개변수 `thread`: Thread가 성공적으로 생성되었을 때, 생성된 Thread를 식별하기 위해서 사용되는 Thread 식별자이다.
* 두 번째 매개변수 `attr`: Thread 특성을 지정하기 위해 사용하며, 기본 Thread 특성을 이용하고자 할 경우에는 `NULL`을 사용한다.
* 세 번째 매개변수 `start_routine`: 분기시켜서 실행할 Thread 함수이다.
  * 함수 포인터로 반환형이 `void*`이고 매개변수가 `void*`인 함수를 전달할 수 있다.
* 네 번째 매개변수 `arg`: 위 `start_routine` Thread 함수의 매개변수로 넘겨진다.
* 반환값: 성공적으로 Thread가 생성된 경우 0을 반환한다.

### pthread_join

```c
int pthread_join(pthread_t th, void** thread_return)
```

<u>특정 Thread가 종료되기를 기다리는 함수 (즉, 이 함수가 반환될 때까지 다음 줄에 있는 코드는 실행되지 않음)</u>

* 첫 번째 매개변수 `th`: 기다릴 Thread의 식별자
* 두 번째 매개변수 `thread_return`: Thread의 반환값이다.
  * 이 매개변수의 자료형이 `void**`인 이유는 Thread를 만들 때 전달한 함수의 반환형이 `void*`여서 이 **함수 외부의 변수**에 `void*` 값을 넣기 위해서 `void**`를 사용한 것이다.
    * 함수 내부에는 그냥 `void*` 값을 반환한다. 이때 `void*`는 동적 할당된 변수를 가리킨다.
    * 함수 외부에서는 `void*` 값을 받기 위해 `void*`형 변수를 만들고 이 변수의 주소를 넘겨준다. 아니면 원하는 타입의 포인터를 선언하고 그 포인터의 주소를 `void**`로 캐스팅해서 전달한다.
* 반환값: 성공하면 0, 실패하면 에러코드 반환

### pthread_detach

```c
int pthread_detach(pthread_t th)
```

<u>Thread를 분리시키는 함수</u>, 일반적으로 `pthread_create`를 사용하여 Thread를 생성하면  Thread가 종료되더라도 사용했던 모든 자원이 해제되지 않는다. 물론 프로세스 자체가 종료되면 모든 Thread의 자원은 반환된다. 지금의 상황은 프로세스가 돌아가는 상황에서 Thread가 종료되었는데 자원이 반환되지 않아 메모리 누수(Memory Leak) 상황을 해결하기 위한 상황이다.

* 첫 번째 매개변수 `th`: 분리시킬 Thread의 식별자
* 반환값: 성공하면 0, 실패하면 에러코드 반환



## main.c

### Outline

* enum `OpenCVMode`

  * `cv_exam`의 인자로 넘길 열거형으로 어떤 기능을 실행할 지 나타낸다. `cv_exam`에서 이 열거형을 받아 적절한 기능을 수행한다.

* enum `DumpState`
  * Thread간에 Dump 상태를 나타내는 열거형이다.
  * Thread간에 이 Dump 상태를 공유하고 이를 통해 각 Thread에서 Dump 상태를 이용해서 적절히 Dump 파일을 처리한다.
  
* struct `DumpMsg`

  * 메시지 큐에서 메시지를 주고 받을 때 사용하는 구조체이다.
  * 첫 번째 필드는 자료형이 꼭 `long`이어야 한다. 이는 메시지를 자체를 구별하는데 사용한다.
  * 두 번째 필드 이후로는 전달하거나 받을 내용을 담을 때 사용한다.

* struct `thr_data`
  * Thread간 데이터 공유용으로 사용하는 구조체이다.
  * 보니까 예제 파일마다 다 다른 구조를 가지고 있다.
  
* `static uint32_t getfourccformat(char*)`

  * 전달 받은 인자(문자열)과 상응하는 format(FOURCC)을 반환하는 함수이다.

  * FOURCC가 뭐지?

* `static int allocate_output_buffers(struct thr_data*)`

  * output 버퍼를 할당해주는 함수
  * `thr_data` 구조체에 있는 `thr_data` 구조체에 있는 `disp` 필드
  * 그래서 정확히 왜 이걸 해야하는거지?

* `static void free_output_buffers(struct buffer**, uint32_t, bool)`

  * output 버퍼를 해제하는 함수이다.

* `void signal_handler(int)`

  * CTRL + C 단축키를 인식하게 하는 함수이다. `main` 함수 마지막에서 이 handler를 등록함.

* `static void draw_operatingtime(struct display*, uint32_t)`

  * 

* `static void cv_disp_update(struct thr_data*)`

* `static void cv_savetojpeg(unsigned char*, int, int)`

* `static void cv_exam(struct thr_data*, char*, char*, OpenCVMode)`

* `void* input_thread(void*)`: 자세한 내용은 아래에 있음

* `void* capture_dump_thread(void*)`: 자세한 내용은 아래에 있음

* `int main(int, char**)`: 자세한 내용은 아래에 있음

### main 함수 분석

main 함수에 있는 코드를 중요한 statement 단위로 분석했다. 이때 해당 statement에 대한 상세한 탐구는 하위 항목으로 기술했다. 이 함수의 정의와 함께 볼 때는 하단의 항목을 순서대로 보면된다.

* `display` 구조체
  
  * `display-kms.h` 파일에 선언되어 있다.
  * `disp_open(int, char**)` 함수의 반환값으로 얻을 수 있다.
  
* `disp_open(int, char**)` 함수 ()`display-kms.c`에 있음)
  
  * 예제 코드에서는 첫 번째 인자로 `disp_argc`, 두 번째 인자로 `disp_argv`를 인자로 받고 있다.
    * `disp_argc`는 매개 변수의 개수를 나타낸다.
    
    * `disp_argv`는 매개 변수(문자열) 배열으로 이 함수에 전달할 내용을 담고 있다.
    
    * `disp_argv`의 첫 번째 문자열은 dummy 문자열이다. 실제로 이를 처리하는 함수에서 첫 번째 문자열은 아무런 영향을 끼치지 않는다. 왜냐하면 아래와 같이 매개 변수 문자열을 처리하기 때문이다.
    
      ```c
      for (i = 1; i < argc; i++)
      ```
    
  * 내부적으로 `disp_kms_open(int, char**)`을 호출하여 초기화된 `display` 구조체를 반환받는다.
  
    * 이때 `disp_open` 함수에 전달 받은 인자들을 그대로 `disp_kms_open`함수에 전달한다. 즉, `disp_open` 함수가 처리하지 않는 매개변수들은 `disp_kms_open` 함수에서 처리된다.
  
  * 인자로 `-s` `4:480x272`를 주었는데 아래와 같이 이를 처리한다.
  
    ```c
    if (sscanf(argv[i], "%d:%64s", &connector->id, connector->mode_str) != 2
        && sscanf(argv[i], "%d@%d:%64s", &connector->id, &connector->crtc, connector->mode_str) != 3) 
    ```
  
    즉, `connector` 구조체 변수 `connector`의 `id`를 4로, `mode_str`을 480x272로 설정한 것이다.
  
    * 여기서 알 수 있는 점이 `connector` 구조체에는 LCD 커넥터의 번호와 해상도가 담겨져 있다는 사실이다.
    * `connector` 구조체의 변수의 개수는 총 10개이다(`display_kms` 구조체 참고).
    

### input_thread 함수 분석

```c
void * input_thread(void *arg) // thread용 함수라서 arg에는 pthread_create로부터 전달받은 인자가 들어있다.
```

* 메뉴 출력

  * 이미지 불러오기, 얼굴 인식, 이미지 바인딩(인식된 얼굴에), 가장자리 검출, 저장 기능

  * `MSG` 매크로를 이용해서 메뉴를 출력하는데, `fprintf` 함수로 `stderr`에다 출력한다.
    * `stderr`은 `stdin`, `stdout`과 같이 표준 스트림이라고 불리우는 것인데, `stderr`은 표준 오류로 프로그램이 오류 메시지나 진단을 출력하기 위해 일반적으로 쓰이는 또다른 출력 스트림이다. 표준 출력 `stdout`과는 독립적인 스트림이며 별도로 리다이렉트될 수 있다.

* 메뉴 입력 받고, 처리

  * `input_cmd.cpp` 파일에 있는 `StandbyInput` 함수를 통해 입력을 받는다.

  * 입력을 받고 나서 `strncmp` 함수를 통해 명령어를 해독한다.

    * save가 입력됐을 때: `DumpMsg` 구조체를 통해 dump 메시지를 `msgsnd` 함수로 보낸다. dump 메시지를 받은 `capture_dump_thread` 함수는 현재 화면을 jpeg 파일로 저장한다.

    * 1이 입력됐을 때(load image): `cv_exam` 함수를 실행하는데, `OPENCV_MODE_1`의 인자를 넘겨준다. 그리고 `cv_exam` 함수에서 전달 받은 인자를 이용해 명령을 수행한다. 이 상황에서는`cv_exam` 함수에서 `OpenCV_laod_file` 함수를 실행한다.
    * 2가 입력됐을 때(face detection): 입력 1과 비슷하다.
    * 3이 입력됐을 때(binding image): 입력 1과 비슷하다.
    * 4가 입력됐을 때(edge detection): 입력 1과 비슷하다.

### capture_dump_thread 함수 분석

```c
void * capture_dump_thread(void *arg)
```

* 계속해서 `DUMP_CMD` 메시지가 있나 `msgrcv` 함수를 통해 확인한다.
  * `DUMP_CMD` 메시지를 받으면 `cv_exam` 함수(OpenCV 예제 프로그램)를 실행하고 dump_state를 `DUMP_READY`로 바꾼다. 이 `DUMP_READY`는 `cv_exam` 함수에서 인식되어 `DUMP_WRITE_TO_FILE` 메시지를 보낸다.
* `DUMP_WRITE_TO_FILE` 메시지를 받으면, 스크린샷을 찍을 준비가 되었다는 말이므로 `cv_savetojpeg` 함수를 호출한다. 그 후 dump_state를 `DUMP_DONE`으로 바꾸어 dump 과정이 끝났음을 알린다.

### cv_exam 함수 분석

```c
static void cv_exam(struct thr_data* data, char* filename1, char* filename2, OpenCVMode mode)
```

* 디스플레이 크기 만큼 `unsigned char`형 `img` 배열을 만들어 `memset`으로 초기화함.
* `exam_cv.cpp` 파일에 있는 `OpenCV_face_detection`, `OpenCV_binding_image`, `OpenCV_canny_edge_image`, `OpenCV_load_file` 함수를 전달 받은 `mode` 인자를 통해 적절히 수행함.
  * 이때 `gettimeofday` 함수(`main.c`에 없음)를 통해 OpenCV 함수를 실행하기 전과 후의 시간을 측정한 다음에(각각 `st`(start time), `et`(end time)에 저장됨) 두 시간의 차이를 구해 OpenCV 처리 시간을 구하고 출력한다.
  * 출력할 때에는 `main.c`에 있는 `draw_operaitingtime` 함수를 호출함.
* dump_state가 `DUMP_READY`일 때 영상처리를 통해 얻은 이미지(`img`)를 thr_data 구조체 변수인 `data`의 `dump_img_data`에 복사하고 `capture_dump_thread` 쓰레드를 위해 `DUMP_WRITE_TO_FILE` 메시지를 보낸다.
* dump_state가 `DUMP_READY`가 아닐 때에는 디스플레이에 `img` 정보를 출력한다.
  1. `display-kms.c` 파일에 있는 `get_framebuf` 함수를 실행해서 현재 디스플레이의 버퍼를 가져온다.
  2. 버퍼에 `img`의 내용을 `memcpy` 함수를 통해 복사한다.
  3. `main.c`에 있는 `cv_disp_update`를 호출한다.
     * 왜 호출하는거지? 버퍼의 내용을 읽어서 디스플레이에 띄우라는 이야긴가?

## exam_cv.cpp

