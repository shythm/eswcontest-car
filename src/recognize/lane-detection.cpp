#include "lane-detection.h"
#include "recognize-update.h"
#include <opencv2/opencv.hpp>
#include <stdio.h>

#define W VPE_OUTPUT_W
#define H VPE_OUTPUT_H

using namespace cv;
using namespace std;

// This is possible because W(=VPE_OUT_W) and H(=VPE_OUTPUT_H) are actually
// constants defined with preprocessor. Therefore, dynamic memory allocation is
// not requried.
unsigned char raw[W * H * 3];

Size sizeOrigin = Size(VPE_OUTPUT_W, VPE_OUTPUT_H);
Size sizeSmall  = Size(VPE_OUTPUT_W / 2, VPE_OUTPUT_H / 2);

double vanish    = 0;   // Y position of vanish point
double range     = 300; //
double viewRange = 0.3; // Region of interest

Scalar blue(255, 0, 0);
Scalar red(0, 0, 255);
Scalar green(0, 255, 0);
Scalar yellow(0, 255, 255);

int    hAverage = 100, hRange = 30;
Scalar hsvLow(hAverage - hRange, 100, 0);
Scalar hsvHigh(hAverage + hRange, 255, 255);

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

void detectLane(recog_arg *arg, vector_lane *result) {
    static bool                  init = true;
    static Mat                   M, mask;
    static vector<vector<Point>> roi;
    static int                   npt[] = {4};
    if (init) {
        init = false;
        cout << sizeOrigin << "," << sizeSmall << endl;
        Point2f       pts[4];
        vector<Point> _roi;
        getRoiPerspectiveTransform(&M, pts);
        for (int i = 0; i < 4; i++)
            _roi.push_back(pts[i]);
        roi.push_back(_roi);
        cout << M << endl;
    }

    // Copy image and wrap raw data with Mat object
    copy(arg->camera_output, arg->camera_output + W * H * 3, raw);
    Mat img(H, W, CV_8UC3, raw);

    // Draw roi
    // polylines(org, roi, true, red, 1);

    cvtColor(img, img, COLOR_BGR2GRAY);

    warpPerspective(img, img, M, Size(W, H));

    Mat org = img.clone();

    threshold(img, mask, 1, 255, THRESH_BINARY_INV);

    add(img, mask, img);

    adaptiveThreshold(img, img, 255, CV_ADAPTIVE_THRESH_MEAN_C,
                      CV_THRESH_BINARY, 31, -20);

    bitwise_not(mask, mask);
    bitwise_and(mask, img, img);

    vector<vector<Point>> contours;
    vector<Vec4i>         hierarchy;
    findContours(img, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE,
                 Point(0, 0));

    cvtColor(img, img, COLOR_GRAY2BGR);
    cvtColor(org, org, COLOR_GRAY2BGR);

    int          i, size;
    Point        top, bottom;
    Point2f      a;
    static float centerLine = W / 2;
     float leftMost = -9999, rightMost = 9999;

    float xExt = 1.0f, aSum = 0, bSum = 0;

    bool leftSet = false, rightSet = false;
    int  aNum = 0, bNum = 0;
    for (i = 0; i < contours.size(); i++) {
        extremum(&contours[i], &size, &top, &bottom);
        if (size < 250) continue;
        lineY(top, bottom, &a);
        aSum += a.x;
        bSum += a.y;
        aNum++;
        bNum++;
        bottom = dotY(H * xExt, a);
        Scalar color;
        if (bottom.x > centerLine) {
            if (bottom.x < rightMost) {
                rightSet  = true;
                rightMost = bottom.x;
            }
            color = red;
        } else {
            if (bottom.x > leftMost) {
                leftSet  = true;
                leftMost = bottom.x;
            }
            color = green;
        }
        line(org, dotY(0, a), bottom, color);
        drawContours(org, contours, i, red, 1, 8, hierarchy, 0, Point());
    }

    float center;
    float aMean    = aSum / aNum;
    float bMean    = bSum / bNum;
    float laneDist = 190 * sqrt(1 + aMean * aMean);
    if (leftSet && rightSet) {
        center = (leftMost + rightMost - W) / 2;
    } else if (!(leftSet || rightSet)) {
        center = 0;
    } else if (leftSet) {
        rightMost = leftMost + laneDist;
        center    = (leftMost + rightMost - W) / 2;
        Point2f _a(aMean, bMean + laneDist);
        line(org, dotY(0, _a), dotY(H * xExt, _a), yellow);
    } else {
        leftMost = rightMost - laneDist;
        center   = (leftMost + rightMost - W) / 2;
        Point2f _a(aMean, bMean - laneDist);
        line(org, dotY(0, _a), dotY(H * xExt, _a), yellow);
    }

    centerLine = 0.99 * centerLine + 0.01 * (center + W / 2);

    // Draw center line
    line(org, Point(centerLine, 0), Point(centerLine, H), blue);
    line(org, Point(W / 2 + center, 0), Point(W / 2 + center, H), yellow);

    // Copy processed image to display
    copy(org.data, org.data + W * H * 3, arg->display_input);

    result->position = -center * 10;
}