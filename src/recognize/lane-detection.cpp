#include "lane-detection.h"
#include "mask-thresh.h"
#include "recognize-update.h"
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

Size sizeOrigin = Size(VPE_OUTPUT_W, VPE_OUTPUT_H);
Size sizeSmall  = Size(VPE_OUTPUT_W / 8, VPE_OUTPUT_H / 8);

double vanish    = 0;   // Y position of vanish point
double range     = 300; //
double viewRange = 0.4; // Region of interest, higher, closer(crop reverse
                        // perspective transformed image)

float detectLineRatio = 0.45;

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

bool isInRange(Point x) { return x.x >= 0 && x.y >= 0 && x.x < W && x.y < H; }

void extremum(vector<Point> *contour, int *size, Point *top, Point *bottom) {
    int maxX = 0, maxY = 0, minX = W * H, minY = W * H;
    for (Point p : *contour) {
        maxX = max(maxX, p.x);
        minX = min(minX, p.x);
        if (p.y > maxY) {
            maxY = p.y;
            *top = p;
        }
        if (p.y < minY) {
            minY    = p.y;
            *bottom = p;
        }
    }
    *size = (maxX - minX) * (maxY - minY);
}

void lineY(Point p1, Point p2, Point2f *a) {
    // let a = dx/dy
    // x-x1 = (y-y1)*a
    // :. x = (y-y1)*a+x1
    // :. x = a*y - y1*a + x1
    // :. x = a*y x1 - y1*a
    float dy = p2.y - p1.y;
    float dx = p2.x - p1.x;
    a->x     = dx / dy;
    a->y     = p1.x - a->x * p1.y;
}

Point dotY(float y, Point2f a) { return Point(y * a.x + a.y, y); }

typedef struct _LaneAnalysis {
    bool init     = true;
    int  laneL    = 0;
    int  laneR    = sizeSmall.width - 1;
    int  dist     = 24;
    int  ScoreMin = -16;
} LaneAnalysis;

LaneAnalysis lAnalysis[2];

enum LaneType {
    ONLY_WITH_YELLOW = 0,
    WITH_YELLOW_AND_WHITE = 1
};

void update(vector<int> dots, LaneType lType) {
    LaneAnalysis& analy = lAnalysis[lType];

    int lScoreMax = -9999;
    int rScoreMax = -9999;
    int lScore, rScore, nl, nr;
    for (int dot : dots) {
        lScore = -abs(dot - analy.laneL);
        rScore = -abs(dot - analy.laneR);
        // cout << dot << "," << laneL << "," << laneR << endl;
        if (lScore > lScoreMax) {
            lScoreMax = lScore;
            nl        = dot;
        }
        if (rScore > rScoreMax) {
            rScoreMax = rScore;
            nr        = dot;
        }
    }

    bool detectL = lScoreMax > analy.ScoreMin;
    bool detectR = rScoreMax > analy.ScoreMin;

    if (analy.init) {
        if (detectL && detectR) { analy.ScoreMin++; }
        if (analy.ScoreMin = -8) analy.init = false;
    }
    if (!detectL && !detectR) {
        nl = analy.laneL;
        nr = analy.laneR;
    }
    if (!detectL || !detectR) {
        if (detectR) { nl = nr - analy.dist; }
        if (detectL) { nr = nl + analy.dist; }
    }

    analy.laneL = nl;
    analy.laneR = nr;
}

// If display is null, this function doesn't copy processed image to display.
float getLanePosition(Mat& img, Mat& mask, unsigned char *display, int& detectLinePos, LaneType lType) {
    Mat imgClone = img.clone();
    Mat maskClone = mask.clone();
    
    cvtColor(maskClone, maskClone, COLOR_GRAY2BGR);
    bitwise_and(imgClone, maskClone, imgClone);
    threshold(imgClone, maskClone, 1, 255, THRESH_BINARY_INV);

    cvtColor(imgClone, imgClone, COLOR_RGB2GRAY);

    vector<int> dots;
    uchar *     row = imgClone.ptr(detectLinePos);
    for (int i = 0; i < sizeSmall.width; i++)
        if (row[i]) { dots.push_back(i); }
    update(dots, lType);
    // cout << laneL << "," << laneR << endl;

    // Restore colorspace
    cvtColor(imgClone, imgClone, COLOR_GRAY2BGR);

    row = imgClone.ptr(detectLinePos);
    for (int i = 0; i < sizeSmall.width; i++) {
        if (row[i * 3]) {
            row[i * 3]     = 0;
            row[i * 3 + 1] = 0;
        } else {
            row[i * 3] = 255;
        }
    }

    LaneAnalysis& analy = lAnalysis[lType];

    if (analy.laneL >= 0 || analy.laneL < sizeSmall.width) {
        row[analy.laneL * 3 + 0] = 0;
        row[analy.laneL * 3 + 1] = 255;
        row[analy.laneL * 3 + 2] = 255;
    }

    if (analy.laneR >= 0 || analy.laneR < sizeSmall.width) {
        row[analy.laneR * 3 + 0] = 255;
        row[analy.laneR * 3 + 1] = 255;
        row[analy.laneR * 3 + 2] = 0;
    }

    // Copy processed image to display
    if (display) {
        // Restore size
        resize(imgClone, imgClone, sizeOrigin, INTER_NEAREST);
        copy(imgClone.data, imgClone.data + W * H * 3, display);
    }

    return -(analy.laneL + analy.laneR - sizeSmall.width);
}

void detectLane(recog_arg *arg, vector_lane *result) {
    static bool init = true;
    static Mat  M, mask;
    static int  detectLinePos, detectLinePosWithWhite;
    if (init) {
        init = false;
        cout << sizeOrigin << "," << sizeSmall << endl;
        Point2f pts[4];
        getRoiPerspectiveTransform(&M, pts);
        detectLinePos           = sizeSmall.height * detectLineRatio;
        detectLinePosWithWhite  = sizeSmall.height * detectLineRatio;
    }

    // Copy image and wrap raw data with Mat object
    copy(arg->camera_output, arg->camera_output + W * H * 3, raw);
    Mat img(H, W, CV_8UC3, raw);

    // Convert to small perspective small size image
    warpPerspective(img, img, M, Size(W, H));
    resize(img, img, sizeSmall, INTER_NEAREST);

    // Now I can do complicated tasks because image is very small.

    // Convert color to hsv
    cvtColor(img, img, COLOR_BGR2HSV);
    vector<Mat> hsv;
    split(img, hsv);
    int hc = 30;
    int he = 20;
    bitwise_and(hsv[0] > hc - he, hsv[0] < hc + he, mask);
    bitwise_and(hsv[1] >= 50, mask, mask);

    // Detect white lanes
    Mat whiteMask = hsv[0] >= 0;
    bitwise_and(hsv[1] >= 0, hsv[1] <= 40, whiteMask);
    bitwise_and(hsv[2] > 200, whiteMask, whiteMask);
    bitwise_and(hsv[2] <= 255, whiteMask, whiteMask);
    bitwise_or(whiteMask, mask, whiteMask);

    result->position = getLanePosition(img, mask, nullptr, detectLinePos, ONLY_WITH_YELLOW);
    result->position_with_white = getLanePosition(img, whiteMask, arg->display_input, detectLinePosWithWhite, WITH_YELLOW_AND_WHITE);
}