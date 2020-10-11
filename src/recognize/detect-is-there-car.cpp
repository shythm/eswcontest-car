#include "detect-is-there-car.h"
#include "recognize-lib.h"
#include <math.h>
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

    int    y, x;
    float  cur, max;
    uchar *yPtr;
    max = 0;
#define DIVIDER 32.f
    // Find max
    for (y = 0; y < VPE_OUTPUT_H; y++) {
        yPtr = temp.ptr<uchar>(y);
        for (x = 0; x < VPE_OUTPUT_W / 2; x++) {
            cur = exp((float)(yPtr[x]) / DIVIDER);
            if (cur > max) max = cur;
        }
    }

    for (y = 0; y < VPE_OUTPUT_H; y++) {
        yPtr = temp.ptr<uchar>(y);
        for (x = 0; x < VPE_OUTPUT_W / 2; x++) {
            cur     = exp((float)(yPtr[x]) / DIVIDER) * 255 / max;
            yPtr[x] = (char)(cur);
        }
    }

    cvtColor(temp, temp, COLOR_GRAY2BGR);

    // show image on display
    Mat car_disp_img(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, arg->display_input);
    // temp.copyTo(car_disp_img);

    // Car position
    vector_car_pos ret;

    ret.left   = false;
    ret.center = false;
    ret.right  = false;

    return ret;
}