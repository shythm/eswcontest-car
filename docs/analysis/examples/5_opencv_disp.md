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

Video4Linux(v4l)의 2번째 버전으로, 리눅스 시스템에서 실시간 비디오 촬영을 위한 드라이버와 API을 모아둔 것이다.

* https://en.wikipedia.org/wiki/Video4Linux
* USB 웹캠, TV 튜너 등등 카메라와 관련된 장비들을 지원하고 표준화된 출력을 제공해서 프로그래머가 리눅스 시스템에 비디오 촬영 기능을 간단히 구현할 수 있다.
* OpenCV, Skype, PyGame, VLC Media Player 등등이 v4l2를 지원한다.
  * 그래서 OpenCV를 하기 전에 v4l2를 이용해서 카메라 데이터를 불러온 것이다.



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

* struct `OpenCVMode`
* enum `DumpState`
  * Thread간에 Dump 상태를 나타내는 열거형이다.
  * Thread간에 이 Dump 상태를 공유하고 이를 통해 각 Thread에서 Dump 상태를 이용해서 적절히 Dump 파일을 처리한다.
* struct `DumpMsg`
* struct `thr_data`
  * Thread간 데이터 공유용으로 사용하는 구조체이다.
  * 보니까 예제 파일마다 다 다른 구조를 가지고 있다.
* `static uint32_t getfourccformat(char*)`
* `static int allocate_output_buffers(struct thr_data*)`
* `static void free_output_buffers(struct buffer**, uint32_t, bool)`
* `void signal_handler(int)`
* `static void draw_operatingtime(struct display*, uint32_t)`
* `static void cv_disp_update(struct thr_data*)`
* `static void cv_savetojpeg(unsigned char*, int, int)`
* `static void cv_exam(struct thr_data*, char*, char*, OpenCVMode)`
* `void* input_thread(void*)`
* `void* capture_dump_thread(void*)`
* `int main(int, char**)`



### main 함수 분석

main 함수에 있는 코드를 중요한 statement 단위로 분석했다. 이때 해당 statement에 대한 상세한 탐구는 하위 항목으로 기술했다. 이 함수의 정의와 함께 볼 때는 하단의 항목을 순서대로 보면된다.

* `display` 구조체
  
  * `display-kms.h` 파일에 선언되어 있다.
  * `disp_open(int, char**)` 함수의 반환값으로 얻을 수 있다.
* `disp_open(int, char**)` 함수
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
  
    * 여기서 알 수 있는 점이 `connector` 구조체에는 LCD 커넥터의 번호와 해상도(아닐 수도 있음)같은 것이 담겨져 있다는 사실이다.
    * `connector` 구조체의 변수의 개수는 총 10개이다(`display_kms` 구조체 참고).
    * 
  
  * `display-kms.c` 파일에 있다.
    
    * `display-kms.c`는 `util.c`를 필요로 한다.

