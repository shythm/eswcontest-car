#include "lane-detection.h"
#include "recognize-update.h"
#include <opencv2/opencv.hpp>
#include <stdio.h>

#define W VPE_OUTPUT_W
#define H VPE_OUTPUT_H

using namespace cv;
using namespace std;

// This is possible because W(=VPE_OUT_W) and H(=VPE_OUTPUT_H) are actually constants defined with preprocessor.
// Therefore, dynamic memory allocation is not requried.
unsigned char raw[W * H * 3];
Size size_origin = Size(VPE_OUTPUT_W, VPE_OUTPUT_H);
Size size_small = Size(VPE_OUTPUT_W / 2, VPE_OUTPUT_H / 2);

double vanish = 0;      // Y position of vanish point
double range = 300;     //
double viewRange = 0.6; // Region of interest

Scalar blue(255, 0, 0);
Scalar red(0, 0, 255);
Scalar green(0, 255, 0);
Scalar yellow(0, 255, 255);

int cent = 100, ran = 30;
Scalar hsvLow(cent - ran, 100, 0);
Scalar hsvHigh(cent + ran, 255, 255);

void _getPerspectiveTransform(Mat *M, Point2f *src)
{
    // Vanish와 range가 주어질 때, y좌표에 따른 x좌표를 계산해보자.
    // 자명히 (y,x)=(vanish,W/2)와 (y,x)=(H,W+range)를 지난다.
    // 그러므로 dy = H-vanish, dx = W+range-W/2 = W/2+range이다.
    // 그러므로 직선의 방정식은
    // x1 = (W/2+range)*(y-vanish)/(H-vanish)+W/2
    // x2 = W/2-(W/2+range)*(y-vanish)/(H-vanish)

    // I don't know why but getPerspeciveTransform does not work properly when all Point2f positions are integer.
    double wh = W / 2 + 0.0001;
    double y = viewRange * H;
    double xDelta = (wh + range) * (y - vanish) / (H - vanish);

    src[0] = Point2f(wh - xDelta, y);
    src[1] = Point2f(wh + xDelta, y);
    src[2] = Point2f(W + range, H);
    src[3] = Point2f(-range, H);

    Point2f dst[4];
    dst[0] = Point2f(0, 0);
    dst[1] = Point2f(W, 0);
    dst[2] = Point2d(W, H);
    dst[3] = Point2f(0, H);

    *M = getPerspectiveTransform(src, dst);
}

void detect_lane(recog_arg *arg, vector_lane *result)
{
    static bool init = true;
    static Mat M, mask;
    static vector<vector<Point>> roi;
    static int npt[] = {4};
    if (init)
    {
        init = false;
        cout << size_origin << "," << size_small << endl;
        Point2f pts[4];
        vector<Point> _roi;
        _getPerspectiveTransform(&M, pts);
        for (int i = 0; i < 4; i++)
            _roi.push_back(pts[i]);
        roi.push_back(_roi);
        cout << M << endl;
    }

    // Copy image and wrap raw data with Mat object
    copy(arg->camera_output, arg->camera_output + W * H * 3, raw);
    Mat img(H, W, CV_8UC3, raw);

    // Draw roi
    // polylines(img, roi, true, red, 3);

    cvtColor(img, img, COLOR_BGR2GRAY);

    warpPerspective(img, img, M, Size(W, H));

    threshold(img, mask, 1, 128, THRESH_BINARY_INV);

    add(img, mask, img);

    adaptiveThreshold(
        img, img, 255,
        CV_ADAPTIVE_THRESH_MEAN_C,
        CV_THRESH_BINARY, 31, -30);

    int
        left = 0,
        right = 0,
        left_n = 0,
        right_n = 0;

    for (int i = 0; i < H; i++)
    {
        uchar *pointer_input = img.ptr<uchar>(i);
        for (int j = 0; j < W / 2; j++)
        {
            if (pointer_input[j] > 128)
            {
                left += j;
                left_n++;
            }
        }
        for (int j = W / 2; j < W; j++)
        {
            if (pointer_input[j] > 128)
            {
                right += j;
                right_n++;
            }
        }
    }

    if (left_n > 0)
        left /= left_n;

    if (right_n > 0)
        right /= right_n;
    else
        right = W - 1;

    static float center = 0;
    static float gain_irr = 0.9;
    int weight = left_n + right_n;
    float temp;
    if (weight == 0)
        temp = 0;
    else
        temp = (left * left_n + right * right_n) / weight;
    center = center * gain_irr + temp * (1 - gain_irr);

    // resize(img, img, size_small, 0, 0, CV_INTER_NN);

    // Threshold frame
    // inRange(img, hsvLow, hsvHigh, mask);

    // static int counter = 0;
    // if (counter % 10 == 0)
    // {
    //     string fileName = "sc_" + to_string(counter + 10) + ".png";
    //     cout << fileName << endl;
    //     imwrite(fileName, img);
    // }
    // counter++;

    // Get nonzero values
    // Mat locations;
    // findNonZero(img, locations);
    // cout << locations << endl;

    // img.copyTo(img, mask);
    cvtColor(img, img, COLOR_GRAY2BGR);
    // resize(img, img, size_origin, 0, 0, CV_INTER_NN);

    // Draw center line
    line(img, Point(W / 2, 0), Point(W / 2, H), blue);
    line(img, Point(left, 0), Point(left, H), red);
    line(img, Point(right, 0), Point(right, H), green);
    line(img, Point(center, 0), Point(center, H), yellow);

    // Copy processed image to display
    copy(img.data, img.data + W * H * 3, arg->display_input);

    result->position = center - W / 2;
}