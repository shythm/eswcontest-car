#include "detect.h"
#include "mask-thresh.h"
#include "recognize-lib.h"
#include <math.h>
#include <opencv2/opencv.hpp>
#include <stdio.h>

using namespace cv;
using namespace std;

#define W VPE_OUTPUT_W
#define H VPE_OUTPUT_H
const Size sizeOrigin = Size(VPE_OUTPUT_W, VPE_OUTPUT_H);
const Size sizeSmall  = Size(VPE_OUTPUT_W / 6, VPE_OUTPUT_H / 6);

const double vanish    = 0;   // Y position of vanish point
const double range     = 300; // TEST
const double viewRange = 0.4; // ROI, higher, closer(crop image)
const float  detectLineRatio =
    0.45; // Detection line position. If 0, top, if 1, bottom.

void getRoiPerspectiveTransform(Mat *M) {
    // Vanish와 range가 주어질 때, y좌표에 따른 x좌표를 계산해보자.
    // 자명히 (y,x)=(vanish,W/2)와 (y,x)=(H,W+range)를 지난다.
    // 그러므로 dy = H-vanish, dx = W+range-W/2 = W/2+range이다.
    // 그러므로 직선의 방정식은
    // x1 = (W/2+range)*(y-vanish)/(H-vanish)+W/2
    // x2 = W/2-(W/2+range)*(y-vanish)/(H-vanish)

    // I don't know why but getPerspeciveTransform does not work properly when
    // all Point2f positions are integer.
    double  wHalf  = W / 2 + 0.0001;
    double  roiY   = viewRange * H;
    double  xDelta = (wHalf + range) * (roiY - vanish) / (H - vanish);
    Point2f src[4];
    src[0] = Point2f(wHalf - xDelta, roiY);
    src[1] = Point2f(wHalf + xDelta, roiY);
    src[2] = Point2f(W + range, H);
    src[3] = Point2f(-range, H);

    Point2f dst[4];
    dst[0] = Point2f(0, 0);
    dst[1] = Point2f(W, 0);
    dst[2] = Point2d(W, H);
    dst[3] = Point2f(0, H);

    *M = getPerspectiveTransform(src, dst);
}

int bar_height = sizeSmall.height / 10;

recog_tl_lane_t cal_tl_lane(Mat img, recog_arg *arg) {
    uchar  min, max, thresh;
    uchar *bar        = img.ptr(bar_height);
    int    bar_len    = img.size().width;
    int    left_first = -1, right_last = bar_len;

    int i = 0;
    for (; i < bar_len; i++) {
        if (bar[i] == 0) continue;
        else {
            min = max = bar[i++];
            break;
        }
    }
    for (; i < bar_len; i++) {
        if (bar[i] == 0) continue;
        if (bar[i] < min) min = bar[i];
        if (max < bar[i]) max = bar[i];
    }
    thresh = (min + 3 * max) / 4;

    for (i = 0; i < bar_len / 2; i++)
        if (bar[i] > thresh) {
            left_first = i;
            break;
        }
    for (i = bar_len - 1; i >= bar_len / 2; i--)
        if (bar[i] > thresh) {
            right_last = i;
            break;
        }

    recog_tl_lane_t ret_val;
    ret_val.is_left_lane  = (left_first < 0) ? false : true;
    ret_val.is_right_lane = (right_last >= bar_len) ? false : true;
    ret_val.position =
        left_first + right_last - bar_len; //(left-len/2)+(right-len/2)

    printf("hello %d:%d \n", left_first, right_last);
#if 1
    // FOR DISPLAY
    cvtColor(img, img, COLOR_GRAY2BGR);
    printf("bye %d:%d \n", left_first, right_last);
    for (int i = 0; i < bar_len; i++) {
        if (i == left_first || i == right_last) { // (B,G,R) = (0,0,255)
            img.at<Vec3b>(bar_height, i)[0] = 0;
            img.at<Vec3b>(bar_height, i)[1] = 0;
            img.at<Vec3b>(bar_height, i)[2] = 255;
        } else { // (B,G,R) - (255,0,0)
            img.at<Vec3b>(bar_height, i)[0] = 255;
            img.at<Vec3b>(bar_height, i)[1] = 0;
            img.at<Vec3b>(bar_height, i)[2] = 0;
        }
    }
    resize(img, img, sizeOrigin, INTER_NEAREST);
    // Copy processed image to display
    copy(img.data, img.data + W * H * 3, arg->display_input);

#endif
    return ret_val;
}

unsigned char   raw_[W * H * 3];
recog_tl_lane_t get_tl_lane_t(recog_arg *arg) {
    static Mat  perspM;
    static bool initialized = false;

    if (!initialized) { // get transform matrix
        getRoiPerspectiveTransform(&perspM);
        initialized = true;
    }

    // read camera image
    copy(arg->camera_output, arg->camera_output + W * H * 3, raw_);
    Mat img(H, W, CV_8UC3, raw_);

    warpPerspective(img, img, perspM, Size(W, H));
    resize(img, img, sizeSmall, INTER_NEAREST);
    cvtColor(img, img, COLOR_BGR2GRAY);
    recog_tl_lane_t ret_val = cal_tl_lane(img, arg);
    img.release();
    return ret_val;
}