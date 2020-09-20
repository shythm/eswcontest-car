#include "detect-end-zone.h"
#include <algorithm>
#include <numeric>
#include <opencv2/opencv.hpp>
#include "recognize-update.h"

using namespace std;
using namespace cv;

bool detectEndZone(unsigned char *cam_data, int cam_w, int cam_h) {
    //이미지 받아와서 mat 클래스로 감싼다
    unsigned char *cam_copied = new unsigned char[cam_w * cam_h * 3];
    copy(cam_data, cam_data + cam_w * cam_h * 3, cam_copied);
    Mat img(cam_h, cam_w, CV_8UC3, cam_copied);

    // region of interst 영역을 자르기
    Mat cut_img(Range()) img 다

    //노란색으로 이미지 마스킹하기

    // binarization(이진화)

    // salt와 pepper 없애기

    //한 row에 대해 rising edge 개수 세서 판단하기
}