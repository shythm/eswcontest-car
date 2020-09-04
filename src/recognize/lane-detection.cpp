#include "lane-detection.h"
#include "recognize-update.h"
#include <opencv2/opencv.hpp>
#include <stdio.h>

#define W VPE_OUTPUT_W
#define H VPE_OUTPUT_H

using namespace cv;

// This is possible because W(=VPE_OUT_W) and H(=VPE_OUTPUT_H) are actually constants defined with preprocessor.
// Therefore, dynamic memory allocation is not requried.
unsigned char raw[W * H * 3];
Size size_origin = Size(VPE_OUTPUT_W, VPE_OUTPUT_H);
Size size_small = Size(VPE_OUTPUT_W / 2, VPE_OUTPUT_H / 2);

float vanish = 0;
float range = 300;
float viewRange = 0.15f;

Scalar blue(255, 0, 0);
Scalar red(0, 0, 255);
Scalar green(0, 255, 0);

void _getPerspectiveTransform(Mat *M, Point2f *src)
{
    // Vanish와 range가 주어질 때, y좌표에 따른 x좌표를 계산해보자.
    // 자명히 (y,x)=(vanish,W/2)와 (y,x)=(H,W+range)를 지난다.
    // 그러므로 dy = H-vanish, dx = W+range-W/2 = W/2+range이다.
    // 그러므로 직선의 방정식은
    // x1 = (W/2+range)*(y-vanish)/(H-vanish)+W/2
    // x2 = W/2-(W/2+range)*(y-vanish)/(H-vanish)

    float wh = W * 1.0f / 2;
    float y = H * viewRange;
    float xDelta = (wh + range) * (y - vanish) / (H - vanish);

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
    static Mat M;
    static vector<vector<Point> /**/> roi;
    static int npt[] = {4};
    if (init)
    {
        init = false;
        std::cout << size_origin << "," << size_small << std::endl;
        Point2f pts[4];
        vector<Point> _roi;
        _getPerspectiveTransform(&M, pts);
        for (int i = 0; i < 4; i++)
            _roi.push_back(pts[i]);
        roi.push_back(_roi);
        std::cout << M << std::endl;
    }

    // Copy image and wrap raw data with Mat object
    std::copy(arg->camera_output, arg->camera_output + W * H * 3, raw);
    Mat src(H, W, CV_8UC3, raw);

    // Draw roi
    polylines(src, roi, true, red, 3);

    // Perspective transform
    warpPerspective(src, src, M, Size(W, H));

    // Convert image to grayscale
    // cvtColor(src, src, COLOR_BGR2YUV);
    // resize(src, src, size_small, 0, 0, CV_INTER_NN);

    // Threshold frame
    // inRange(frame_HSV, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), frame_threshold);

    // adaptiveThreshold(
    //     src, src, 255,
    //     CV_ADAPTIVE_THRESH_MEAN_C,
    //     CV_THRESH_BINARY, 3, -10);

    // Convert grayscale image back to RGB image

    // resize(src, src, size_origin, 0, 0, CV_INTER_NN);
    // cvtColor(src, src, COLOR_YUV2BGR);

    // line(src,
    //          Point(W / 2, vanish),
    //          Point(-range, H),
    //          Scalar(255, 0, 0));

    // line(src,
    //          Point(W / 2, vanish),
    //          Point(W + range, H),
    //          Scalar(0, 0, 255));

    //

    // Copy processed image to display
    std::copy(src.data, src.data + W * H * 3, arg->display_input);
}