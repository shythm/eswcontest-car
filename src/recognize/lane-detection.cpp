#include "lane-detection.h"
#include "mask-thresh.h"
#include <iostream>
#include <math.h>
#include <opencv2/opencv.hpp>

#define W VPE_OUTPUT_W
#define H VPE_OUTPUT_H

using namespace cv;
using namespace std;

// Color constants
const Scalar blue(255, 0, 0);
const Scalar red(0, 0, 255);
const Scalar green(0, 255, 0);
const Scalar yellow(0, 255, 255);

const Size sizeOrigin = Size(VPE_OUTPUT_W, VPE_OUTPUT_H);
const Size sizeSmall  = Size(VPE_OUTPUT_W / 4, VPE_OUTPUT_H / 4);

const double vanish    = 0;   // Y position of vanish point
const double range     = 300; // TEST
const double viewRange = 0.4; // ROI, higher, closer(crop image)

void getRoiPerspectiveTransform(Mat *M, Point2f *src) {
    // Vanish와 range가 주어질 때, y좌표에 따른 x좌표를 계산해보자.
    // 자명히 (y,x)=(vanish,W/2)와 (y,x)=(H,W+range)를 지난다.
    // 그러므로 dy = H-vanish, dx = W+range-W/2 = W/2+range이다.
    // 그러므로 직선의 방정식은
    // x1 = (W/2+range)*(y-vanish)/(H-vanish)+W/2
    // x2 = W/2-(W/2+range)*(y-vanish)/(H-vanish)

    // I don't know why but getPerspeciveTransform does not work properly when
    // all Point2f positions are integer.
    double wHalf  = W / 2 + 0.0001;
    double roiY   = viewRange * H;
    double xDelta = (wHalf + range) * (roiY - vanish) / (H - vanish);

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

const int dist               = 24;
const int maxDetectThreshold = -8;

typedef struct {
    bool initState;
    int  laneL;
    int  laneR;
    int  scoreThreshold;
} Detection_Info;

void detectLane(recog_arg *arg, vector_lane *result) {
    cout << "TEST1" << endl;

    static Mat  perspM;        // Matirx for perspective transform
    static Mat  colorMask;     // Matrix used for mask(for yellow detection)
    static int  detectLinePos; // Line detection position
    static bool isInit = true;
    if (isInit) {
        Point2f pts[4];
        getRoiPerspectiveTransform(&perspM, pts);
        isInit = false;
    }

    // Copy image and wrap raw data with Mat object
    static unsigned char raw[W * H * 3];
    copy(arg->camera_output, arg->camera_output + W * H * 3, raw);
    Mat img(H, W, CV_8UC3, raw);

    warpPerspective(img, img, perspM, Size(W, H));

    // 1. Convert image into grayscale
    cvtColor(img, img, COLOR_BGR2GRAY);

    // 2. Apply Gaussian blur
    GaussianBlur(img, img, Size(3, 3), 0, 0);

    // 3. Calculate edge
    Mat grad_x, grad_y;
    Sobel(img, grad_x, CV_16S, 1, 0, 3);
    Sobel(img, grad_y, CV_16S, 0, 1, 3);
    cout << "TEST2" << endl;

    // 4. Calculate norm of gradient
    for (int i = 0; i < H; i++) {
        uchar *xRow = grad_x.ptr(i);
        uchar *yRow = grad_y.ptr(i);
        uchar *yImg = img.ptr(i);
        for (int j = 0; j < W; j++) { yImg[j] = abs(xRow[j]) + abs(yRow[j]); }
    }

    // 5. Convert image back into bgr colorspace
    cvtColor(img, img, COLOR_GRAY2BGR);

    cout << "TEST3" << endl;
    Mat dispImg(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, arg->display_input);

    cout << "TEST4" << endl;
    img.copyTo(dispImg);

    cout << "TEST5" << endl;
    static Detection_Info detectInfo;
    result->position = -(detectInfo.laneL + detectInfo.laneR - sizeSmall.width);

    cout << "TEST6" << endl;
}