#include "lane-detection.h"
#include "mask-thresh.h"
#include <math.h>
#include <opencv2/opencv.hpp>
#include <stdio.h>

#define W VPE_OUTPUT_W
#define H VPE_OUTPUT_H

using namespace cv;
using namespace std;

// Color constants
const Scalar blue(255, 0, 0);
const Scalar red(0, 0, 255);
const Scalar green(0, 255, 0);
const Scalar yellow(0, 255, 255);

// This is possible because W(=VPE_OUT_W) and H(=VPE_OUTPUT_H) are actually
// constants defined with preprocessor. Therefore, dynamic memory allocation is
// not requried.
unsigned char raw[W * H * 3];

const Size sizeOrigin = Size(VPE_OUTPUT_W, VPE_OUTPUT_H);
const Size sizeSmall  = Size(VPE_OUTPUT_W / 8, VPE_OUTPUT_H / 8);

const double vanish    = 0;   // Y position of vanish point
const double range     = 300; // TEST
const double viewRange = 0.4; // ROI, higher, closer(crop image)
const float  detectLineRatio =
    0.45; // Detection line position. If 0, top, if 1, bottom.

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

void init_detection_info(Detection_Info *di) {
    di->initState      = true;
    di->laneL          = 0;
    di->laneR          = sizeSmall.width - 1;
    di->scoreThreshold = -16;
}

void update(uchar *dots, Detection_Info *di) {
    float lScoreMax = -9999;
    float rScoreMax = -9999;
    float lScore, rScore;
    int   posL, posR;

    // Get left lane position and right lane position
    for (int i = 0; i < sizeSmall.width; i++) {
        float dot = (dots[i] + 1) / 256.f;

        // -abs(dot - di.laneL) is a heuristic function.
        lScore = -(abs(i - di->laneL) + 2) / dot;
        rScore = -(abs(i - di->laneR) + 2) / dot;

        if (lScore > lScoreMax) {
            lScoreMax = lScore;
            posL      = i;
        }

        if (rScore > rScoreMax) {
            rScoreMax = rScore;
            posR      = i;
        }
    }

    // Check whether lane is detected.
    // For first frame, scoreThreshold is quiet low(-16). Therefore, it easily
    // detects lane.
    bool detectL = lScoreMax > di->scoreThreshold;
    bool detectR = rScoreMax > di->scoreThreshold;

    // If we detect both left and right lane, gradually increase threshold.
    if (di->initState) {
        if (detectL && detectR) { di->scoreThreshold++; }
        // If threshold reached at some level, stop increasing.
        if (di->scoreThreshold == maxDetectThreshold) di->initState = false;
    }

    // If no line detected, use position of previous frame.
    if (!detectL && !detectR) {
        posL = di->laneL;
        posR = di->laneR;
    }

    // If only one lane is detected, get the lane position from another lane
    // Actually, the condition !detectL || !detectR is not detectR^detectL.
    // But eventaully it works identically.
    if (!detectL || !detectR) {
        if (detectR) { posL = posR - dist; }
        if (detectL) { posR = posL + dist; }
    }

    // Update position
    di->laneL = posL;
    di->laneR = posR;
}

void detectLane(recog_arg *arg, vector_lane *result) {
    static Mat perspM;        // Matirx for perspective transform
    static Mat colorMask;     // Matrix used for mask(for yellow detection)
    static int detectLinePos; // Line detection position
    static Detection_Info detectionInfo; //

    static bool init = true;
    if (init) {
        Point2f pts[4];
        getRoiPerspectiveTransform(&perspM, pts);
        init                         = false;
        detectLinePos                = sizeSmall.height * detectLineRatio;
        arg->shm_rr->lane.initialize = true;
    }

    if (arg->shm_rr->lane.initialize) {
        init_detection_info(&detectionInfo);
        arg->shm_rr->lane.initialize = false;
    }

    // Copy image and wrap raw data with Mat object
    copy(arg->camera_output, arg->camera_output + W * H * 3, raw);
    Mat img(H, W, CV_8UC3, raw);

    // Convert to small perspective small size image
    warpPerspective(img, img, perspM, Size(W, H));
    resize(img, img, sizeSmall, INTER_NEAREST);

    // Now I can do complicated tasks because image is very small.
    cvtColor(img, img, COLOR_BGR2GRAY);

#define DIVIDER 32.f
    float  cur, max;
    uchar *row;
    max = 0;
    for (int i = 0; i < sizeSmall.height; i++) {
        row = img.ptr(i);
        for (int j = 0; j < sizeSmall.width; j++) {
            cur = exp(row[j] / DIVIDER);
            if (cur > max) max = cur;
        }
    }

    for (int i = 0; i < sizeSmall.height; i++) {
        row = img.ptr(i);
        for (int j = 0; j < sizeSmall.width; j++) {
            cur    = exp(row[j] / DIVIDER) * 255 / max;
            row[j] = (char)(cur);
        }
    }

    row = img.ptr(detectLinePos);

    // Update detectionInfo
    update(row, &detectionInfo);

    // Restore colorspace
    cvtColor(img, img, COLOR_GRAY2BGR);

    // Display result. You must remove here at release version
    row = img.ptr(detectLinePos);
    for (int i = 0; i < sizeSmall.width; i++) {
        if (row[i * 3]) {
            row[i * 3]     = 0;
            row[i * 3 + 1] = 0;
        } else {
            row[i * 3] = 255;
        }
    }

    if (detectionInfo.laneL >= 0 || detectionInfo.laneL < sizeSmall.width) {
        row[detectionInfo.laneL * 3 + 0] = 0;
        row[detectionInfo.laneL * 3 + 1] = 255;
        row[detectionInfo.laneL * 3 + 2] = 255;
    }

    if (detectionInfo.laneR >= 0 || detectionInfo.laneR < sizeSmall.width) {
        row[detectionInfo.laneR * 3 + 0] = 255;
        row[detectionInfo.laneR * 3 + 1] = 255;
        row[detectionInfo.laneR * 3 + 2] = 0;
    }

    // Restore size
    resize(img, img, sizeOrigin, INTER_NEAREST);

    // Copy processed image to display
    // copy(img.data, img.data + W * H * 3, arg->display_input);

    // center position = (left+width)/2 - imgWidth/2;
    //                 = (left+width-imgWidth)/2
    // Because we use gain in process, constant is not required.
    // And to reverse direction, multiply -1.
    result->position =
        -(detectionInfo.laneL + detectionInfo.laneR - sizeSmall.width);
}