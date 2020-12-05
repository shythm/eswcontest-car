#include "recognize-lib.h"
#include <math.h>
#include <opencv2/opencv.hpp>
#include <stdio.h>

#define W VPE_OUTPUT_W
#define H VPE_OUTPUT_H

using namespace cv;
using namespace std;

// 입력 이미지 사이즈 정보
const Size sizeOrigin = Size(W, H);
const Size sizeSmall  = Size(W / 8, H / 8);

// 차선 정보를 업데이트하기 위한 멤버 변수들
typedef struct _lane_info {
    bool init;
    int  posL;
    int  posR;
    int  threshScore;
} LaneInfo;

// LaneInfo 구조체 초기화
void initLaneInfo(LaneInfo &li) {
    li.init        = true;
    li.posL        = 0;
    li.posR        = sizeSmall.width - 1;
    li.threshScore = -16;
}

// LaneInfo 구조체 업데이트
void updateLaneInfo(vector<int> positions, LaneInfo &li) {
    // constants
    static const int dist           = 20;
    static const int threshScoreMax = -8;

    float lScoreMax = -9999;
    float rScoreMax = -9999;
    int   lScore, rScore, posL, posR;

    // Get left lane position and right lane position
    for (int pos : positions) {
        lScore = -abs(pos - li.posL);
        rScore = -abs(pos - li.posR);

        if (lScore > lScoreMax) {
            lScoreMax = lScore;
            posL      = pos;
        }
        if (rScore > rScoreMax) {
            rScoreMax = rScore;
            posR      = pos;
        }
    }

    // Check whether lane is detected.
    // For first frame, threshScore is quiet low(-16). Therefore, it easily
    // detects lane.
    bool detectL = lScoreMax > li.threshScore;
    bool detectR = rScoreMax > li.threshScore;

    if (li.init) {
        // If we detect both left and right lane, gradually increase threshold.
        if (detectL && detectR) { li.threshScore++; }
        // If threshold reached at some level, stop increasing.
        if (li.threshScore == threshScoreMax) li.init = false;
    }

    // If no line detected, use position of previous frame.
    if (!detectL && !detectR) {
        posL = li.posL;
        posR = li.posR;
    }

    // If only one lane is detected, get the lane position from another lane
    // Actually, the condition !detectL || !detectR is not detectR^detectL.
    // But eventaully it works identically.
    if (!detectL || !detectR) {
        if (detectL) { posR = posL + dist; }
        if (detectR) { posL = posR - dist; }
    }

    // Update position
    li.posL = posL;
    li.posR = posR;
}

// 항공뷰를 위한 행렬 계산
void getRoiPerspectiveTransform(Mat &perspM) {
    static const double vanish    = 16;  // Y position of vanish point
    static const double range     = 300; // TEST
    static const double viewRange = 0.4; // ROI, higher, closer(crop image)

    // Vanish와 range가 주어질 때, y좌표에 따른 x좌표를 계산해보자.
    // 자명히 (y,x)=(vanish,W/2)와 (y,x)=(H,W+range)를 지난다.
    // 그러므로 dy = H-vanish, dx = W+range-W/2 = W/2+range이다.
    // 그러므로 직선의 방정식은
    // x1 = (W/2+range)*(y-vanish)/(H-vanish)+W/2
    // x2 = W/2-(W/2+range)*(y-vanish)/(H-vanish)

    double wHalf  = W / 2 + 0.0001;
    double roiY   = viewRange * H;
    double xDelta = (wHalf + range) * (roiY - vanish) / (H - vanish);

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

    perspM = getPerspectiveTransform(src, dst);
}

void getValidPositions(Mat &img, vector<int> &out, int bl, Scalar l, Scalar u) {
    uchar *row = img.ptr(bl);
    bool   isValid;

    for (int i = 0; i < img.size().width; i++) {
        isValid = true;
        isValid &= (l[0] <= row[i * 3 + 0]) && (row[i * 3 + 0] <= u[0]);
        isValid &= (l[1] <= row[i * 3 + 1]) && (row[i * 3 + 1] <= u[1]);
        isValid &= (l[2] <= row[i * 3 + 2]) && (row[i * 3 + 2] <= u[2]);

        if (isValid) { out.push_back(i); }
    }
}

#define BASE_LINE_RATIO 0.60
#define DISPLAY_RESULT

void detectLane(recog_arg *arg, vector_lane *result) {
    static uchar raw[W * H * 3]; // dynamic memory allocation is not required.
    static Mat   perspM;         // Matirx for perspective transform
    static int   baseline;

    static LaneInfo liY;   // line information of yellow lane
    static LaneInfo liYAW; // line information of white and yellow lane

    // Initialization
    static bool init = true;
    if (init) {
        getRoiPerspectiveTransform(perspM);
        arg->pext_data->call_init_lane_info =
            true; // 외부에서 LaneInfo 초기화하는 용도로 사용됨
        baseline = sizeSmall.height * BASE_LINE_RATIO;
        init     = false;
    }

    if (arg->pext_data->call_init_lane_info) {
        initLaneInfo(liY);
        initLaneInfo(liYAW);
        arg->pext_data->call_init_lane_info = false;
    }

    // Copy image and wrap raw data with Mat object
    copy(arg->camera_output, arg->camera_output + W * H * 3, raw);
    Mat img(H, W, CV_8UC3, raw);

#if 0
    // Image save
    static int frame = 0;
    frame++;
    if (frame % 3 == 0) {
        string name =
            "/home/root/imgs/screenshot2-" + to_string(frame) + ".jpg";
        // cout << "Frame : " << name << endl;
        imwrite(name, img);
    }
#endif

    // Convert to small perspective small size image
    warpPerspective(img, img, perspM, sizeOrigin);
    resize(img, img, sizeSmall, INTER_NEAREST);
    cvtColor(img, img, CV_BGR2HSV);

    // yellow lane
    const static Scalar lowerY(20, 70, 0);
    const static Scalar upperY(40, 255, 255);
    vector<int>         vpOfY; // valid positions of yellow
    getValidPositions(img, vpOfY, baseline, lowerY, upperY);
    updateLaneInfo(vpOfY, liY);

    // yellow and white lane
    const static Scalar lowerW(0, 0, 200);
    const static Scalar upperW(255, 40, 255);
    vector<int>         vpOfYAW;
    for (int vp : vpOfY) { // push back valid positions of yellow
        vpOfYAW.push_back(vp);
    }
    getValidPositions(img, vpOfYAW, baseline, lowerW, upperW);
    updateLaneInfo(vpOfYAW, liYAW);

    // Restore colorspace
    cvtColor(img, img, COLOR_HSV2BGR);

#ifdef DISPLAY_RESULT
    // Display result. You must remove here at release version

    // inRange(img, lowerY, upperY, img);
    // cvtColor(img, img, COLOR_GRAY2BGR);

    uchar *row = img.ptr(baseline);
    for (int i = 0; i < sizeSmall.width; i++) {
        // make color of baseline to white
        row[i * 3 + 0] = 255;
        row[i * 3 + 1] = 255;
        row[i * 3 + 2] = 255;
    }
    if (liYAW.posL >= 0 || liYAW.posL < sizeSmall.width) {
        row[liYAW.posL * 3 + 0] = 0;
        row[liYAW.posL * 3 + 1] = 0;
        row[liYAW.posL * 3 + 2] = 255;
    }
    if (liYAW.posR >= 0 || liYAW.posR < sizeSmall.width) {
        row[liYAW.posR * 3 + 0] = 255;
        row[liYAW.posR * 3 + 1] = 0;
        row[liYAW.posR * 3 + 2] = 0;
    }

    // Restore size
    resize(img, img, sizeOrigin, INTER_NEAREST);
#endif
    // // Copy processed image to display
    // copy(img.data, img.data + W * H * 3, arg->display_input);

    // center position = (left+width)/2 - imgWidth/2;
    //                 = (left+width-imgWidth)/2
    // Because we use gain in process, constant is not required.
    // And to reverse direction, multiply -1.
    result->pos_yl   = -(liY.posL + liY.posR - sizeSmall.width);
    result->pos_yawl = -(liYAW.posL + liYAW.posR - sizeSmall.width);
}

// *********************************************************
// THESE FUNCTIONS ARE FOR UPDATE recog_result STRUCTURE.
// *********************************************************
extern "C" vector_lane get_lane(recog_arg *arg) {
    static vector_lane result;
    detectLane(arg, &result);
    return result;
}