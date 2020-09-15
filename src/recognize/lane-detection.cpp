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

float detectLineRatio = 0.3;

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

bool init     = true;
int  laneL    = 0;
int  laneR    = sizeSmall.width - 1;
int  dist     = 24;
int  ScoreMin = -16;

void update(vector<int> dots) {
    int lScoreMax = -9999;
    int rScoreMax = -9999;
    int lScore, rScore, nl, nr;
    for (int dot : dots) {
        lScore = -abs(dot - laneL);
        rScore = -abs(dot - laneR);
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

    bool detectL = lScoreMax > ScoreMin;
    bool detectR = rScoreMax > ScoreMin;

    if (init) {
        if (detectL && detectR) { ScoreMin++; }
        if (ScoreMin = -8) init = false;
    }
    if (!detectL && !detectR) {
        nl = laneL;
        nr = laneR;
    }
    if (!detectL || !detectR) {
        if (detectR) { nl = nr - dist; }
        if (detectL) { nr = nl + dist; }
    }

    laneL = nl;
    laneR = nr;
}

void detectLane(recog_arg *arg, vector_lane *result) {
    static bool init = true;
    static Mat  M, mask;
    static int  detectLinePos;
    if (init) {
        init = false;
        cout << sizeOrigin << "," << sizeSmall << endl;
        Point2f pts[4];
        getRoiPerspectiveTransform(&M, pts);
        detectLinePos = sizeSmall.height * detectLineRatio;
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

    cvtColor(mask, mask, COLOR_GRAY2BGR);
    bitwise_and(img, mask, img);
    threshold(img, mask, 1, 255, THRESH_BINARY_INV);

    cvtColor(img, img, COLOR_RGB2GRAY);

    vector<int> dots;
    uchar *     row = img.ptr(detectLinePos);
    for (int i = 0; i < sizeSmall.width; i++)
        if (row[i]) { dots.push_back(i); }
    update(dots);
    // cout << laneL << "," << laneR << endl;

    // Restore colorspace
    cvtColor(img, img, COLOR_GRAY2BGR);

    row = img.ptr(detectLinePos);
    for (int i = 0; i < sizeSmall.width; i++) {
        if (row[i * 3]) {
            row[i * 3]     = 0;
            row[i * 3 + 1] = 0;
        } else {
            row[i * 3] = 255;
        }
    }
    if (laneL >= 0 || laneL < sizeSmall.width) {
        row[laneL * 3 + 0] = 0;
        row[laneL * 3 + 1] = 255;
        row[laneL * 3 + 2] = 255;
    }

    if (laneR >= 0 || laneR < sizeSmall.width) {
        row[laneR * 3 + 0] = 255;
        row[laneR * 3 + 1] = 255;
        row[laneR * 3 + 2] = 0;
    }

    // Restore size
    resize(img, img, sizeOrigin, INTER_NEAREST);

    // Copy processed image to display
    copy(img.data, img.data + W * H * 3, arg->display_input);

    result->position = -(laneL + laneR - sizeSmall.width);
}