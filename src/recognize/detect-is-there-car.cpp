#include "detect-is-there-car.h"
#include "recognize-lib.h"
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

vector_car_pos detectIsThereCar(recog_arg *arg) {
    unsigned char *      cam_data = arg->camera_output;
    static unsigned char src_data[VPE_OUTPUT_W * VPE_OUTPUT_H * 3];
    copy(cam_data, cam_data + sizeof(src_data), src_data);
    Mat src(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, src_data);

    Mat temp;
    cvtColor(src, temp, COLOR_BGR2GRAY);
    cvtColor(temp, temp, COLOR_GRAY2BGR);

    // show image on display
    Mat car_disp_img(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, arg->display_input);
    temp.copyTo(car_disp_img);

    // Car position
    vector_car_pos ret;

    ret.left   = false;
    ret.center = false;
    ret.right  = false;

    return ret;
}