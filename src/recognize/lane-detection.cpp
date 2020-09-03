#include "lane-detection.h"
#include "recognize-update.h"
#include <opencv2/opencv.hpp>
#include <stdio.h>

#define W VPE_OUTPUT_W
#define H VPE_OUTPUT_H

// This is possible because W(=VPE_OUT_W) and H(=VPE_OUTPUT_H) are actually constants defined with preprocessor.
// Therefore, dynamic memory allocation is not requried.
unsigned char raw[W * H * 3];
cv::Size size_origin = cv::Size(VPE_OUTPUT_W, VPE_OUTPUT_H);
cv::Size size_small = cv::Size(VPE_OUTPUT_W / 2, VPE_OUTPUT_H / 2);

int vanish = 0;
int range = 130;

cv::Mat getPerspectiveTransform()
{
    cv::Point2f src[4];
    cv::Point2f dst[4];

    src[0] = cv::Point2f(vanish, 0);
    src[1] = cv::Point2f(vanish, 0);
    src[2] = cv::Point2f(-range, H);
    src[2] = cv::Point2f(W + range, H);

    dst[0] = cv::Point2f(0, 0);
    dst[1] = cv::Point2f(W, 0);
    dst[2] = cv::Point2f(0, H);
    dst[3] = cv::Point2d(W, H);

    return cv::getPerspectiveTransform(src, dst);
}

void detect_lane(recog_arg *arg, vector_lane *result)
{
    static bool init = true;
    if (init)
    {
        init = false;
        std::cout << size_origin << "," << size_small << std::endl;
    }

    // Copy image
    std::copy(arg->camera_output, arg->camera_output + W * H * 3, raw);
    // Just wrap raw data with object
    cv::Mat src(H, W, CV_8UC3, raw);

    // Convert image to grayscale
    cv::cvtColor(src, src, cv::COLOR_BGR2GRAY);
    // cv::resize(src, src, size_small, 0, 0, CV_INTER_NN);

    // cv::adaptiveThreshold(
    //     src, src, 255,
    //     CV_ADAPTIVE_THRESH_MEAN_C,
    //     CV_THRESH_BINARY, 3, -10);

    // Convert grayscale image back to RGB image
    // cv::resize(src, src, size_origin, 0, 0, CV_INTER_NN);
    cv::cvtColor(src, src, cv::COLOR_GRAY2BGR);

    // cv::line(src,
    //          cv::Point(W / 2, vanish),
    //          cv::Point(-range, H),
    //          cv::Scalar(255, 0, 0));

    // cv::line(src,
    //          cv::Point(W / 2, vanish),
    //          cv::Point(W + range, H),
    //          cv::Scalar(0, 0, 255));

    cv::

        // Copy processed image to display
        std::copy(src.data, src.data + W * H * 3, arg->display_input);
}