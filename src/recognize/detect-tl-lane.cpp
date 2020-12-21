#include "recognize-lib.h"
#include <math.h>
#include <opencv2/opencv.hpp>
#include <stdio.h>

using namespace cv;
using namespace std;

#define W               VPE_OUTPUT_W
#define H               VPE_OUTPUT_H
#define W_SMALL         VPE_OUTPUT_W / 4
#define H_SMALL         VPE_OUTPUT_H / 4
#define DISPLAY_TL_LANE 1

const Size   sizeOrigin   = Size(W, H);
const Size   sizeSmall    = Size(W_SMALL, H_SMALL);
const double vanish       = 0;   // Y position of vanish point
const double range        = 300; // TEST
const double viewRange    = 0.4; // ROI, higher, closer(crop image)
const int    bar_height   = H_SMALL / 10;
const Scalar lower_yellow = Scalar(15, 20, 20);
const Scalar upper_yellow = Scalar(65, 255, 255);

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
    dst[2] = Point2f(W, H);
    dst[3] = Point2f(0, H);

    *M = getPerspectiveTransform(src, dst);
}

float get_tl_lane_t(recog_arg *arg) {
    static Mat           perspM;
    static bool          initialized  = false;
    static float         pre_position = 0;
    static unsigned char raw_[W * H * 3];

    if (!initialized) { // get transform matrix
        getRoiPerspectiveTransform(&perspM);
        initialized = true;
    }

    // read camera image
    copy(arg->camera_output, arg->camera_output + W * H * 3, raw_);
    Mat img(H, W, CV_8UC3, raw_);

    // convert to HSV
    cvtColor(img, img, COLOR_BGR2HSV);
    warpPerspective(img, img, perspM, Size(W, H));
    resize(img, img, sizeSmall, INTER_NEAREST);

    // yello mask
    Mat img_yellow;
    inRange(img, lower_yellow, upper_yellow, img_yellow);

#if DISPLAY_TL_LANE
    // img (for display) convert to BGR
    cvtColor(img, img, COLOR_HSV2BGR);
#endif
    // get position
    uchar *bar      = img_yellow.ptr(bar_height);
    int    bar_len  = img_yellow.size().width;
    float  position = 0;
    int    pnum     = 0;

    for (int i = 0; i < bar_len; i++) {
        if (bar[i] == 0) {
#if DISPLAY_TL_LANE
            img.at<Vec3b>(bar_height, i)[0] = 255;
            img.at<Vec3b>(bar_height, i)[1] = 0;
            img.at<Vec3b>(bar_height, i)[2] = 0;
#endif
            continue;
        }
#if DISPLAY_TL_LANE
        img.at<Vec3b>(bar_height, i)[0] = 0;
        img.at<Vec3b>(bar_height, i)[1] = 0;
        img.at<Vec3b>(bar_height, i)[2] = 255;
#endif
        pnum++;
        position += (i - bar_len / 2);
    }
    if (pnum) position /= (float)pnum;

#if DISPLAY_TL_LANE
    // printf("%6.3F\n", position);
    resize(img, img, sizeOrigin);
    copy(img.data, img.data + W * H * 3, arg->display_input);
#endif
    img.release();
    img_yellow.release();
    pre_position = (position + pre_position) / 2; // Low Pass Filter
    return pre_position;
}

// *********************************************************
// THESE FUNCTIONS ARE FOR UPDATE recog_result STRUCTURE.
// *********************************************************
extern "C" float get_tl_lane(recog_arg *arg) { return get_tl_lane_t(arg); }