#include "detect-end-zone.h"
#include "recognize-update.h"
#include <algorithm>
#include <numeric>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

bool detectEndZone(unsigned char *cam_data, int cam_w, int cam_h,
                   unsigned char *disp_data, int disp_w, int disp_h) {
    //이미지 받아와서 mat 클래스로 감싼다
    unsigned char *cam_copied = new unsigned char[cam_w * cam_h * 3];
    copy(cam_data, cam_data + cam_w * cam_h * 3, cam_copied);
    Mat src_img(cam_h, cam_w, CV_8UC3, cam_copied);

    // region of interst 영역을 자르고 출력 디스플레이 사이즈에 맞게 리사이즈.
    Mat cut_img =
        src_img(Range(90, 135), Range(0, VPE_OUTPUT_W)).clone(); //깊은 복사임
    Mat cut_disp_img = Mat::zeros(68, 480, CV_8UC3); // 68 = 45*(272/180)
    resize(cut_img, cut_disp_img, Size(68, 480), CV_INTER_LINEAR);

    //자동차 화면에 띄울 검은색의 디스플레이 사이즈의 인스턴스를 하나 만든다.
    Mat cut_full_disp_img = Mat::zeros(disp_h, disp_w, CV_8SC3);
    //리사이즈한 인스턴스를 적절한 곳에 붙여넣는다.
    for (int i = 0; i < cut_disp_img.rows; i++) {
        for (int j = 0; j < cut_disp_img.cols; j++) {
            cut_full_disp_img.at<Vec3b>(68 + i, j) =
                cut_disp_img.at<Vec3b>(i, j);
        }
    }
    //자동차 화면에 띄운다.
    Mat car_disp_img(disp_h, disp_w, CV_8UC3, disp_data);
    cut_full_disp_img.copyTo(car_disp_img);
    //자동차 화면에 띄우는 용으로 만든다

    //노란색으로 이미지 마스킹하기

    // binarization(이진화)

    // salt와 pepper 없애기

    //한 row에 대해 rising edge 개수 세서 판단하기
}