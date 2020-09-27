#include "recognize-update.h"

#define FRAME_ERROR 30


bool detectTurnnel (recog_arg arg) {
    static int mean_frame=0;
    unsigned char* cam_data = arg->
    //현재 프레임의 평균값을 구한다.
    
    //이전 프레임들의 총 평균값에 또 평균한다.

    //평균 값보다 일정 값 이상 어두워지면 주변이 어두워진 것으로 판단. (터널 진입!)

}