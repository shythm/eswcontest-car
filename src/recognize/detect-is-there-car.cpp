#include "detect-is-there-car.h"
#include "recognize-lib.h"
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

vector_car_pos detectIsThereCar(recog_arg *arg) {
    unsigned char *cam_data  = arg->camera_output;
    unsigned char *disp_data = arg->display_input;
    // Mat            output_img();
    //이미지 받아와서 mat 클래스로 감싼다
    static unsigned char cam_copied[VPE_OUTPUT_W * VPE_OUTPUT_H * 3];
    copy(cam_data, cam_data + sizeof(cam_copied), cam_copied);
    Mat src_img(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, cam_copied);

    // show image on display
    Mat car_disp_img(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, disp_data);
    src_img.copyTo(car_disp_img);

    vector_car_pos ret;

    ret.left   = false;
    ret.center = false;
    ret.right  = false;

    // 함수 작성
    return ret;
}