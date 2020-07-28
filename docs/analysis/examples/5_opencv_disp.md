# 5_opencv_disp 코드 분석

* 그림 파일을 불러와서 OpenCV로 처리한 다음 LCD에 결과를 출력하는 예제
* 그리고 OpenCV 처리 시간을 LCD로 출력

## main 함수 탐구

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

