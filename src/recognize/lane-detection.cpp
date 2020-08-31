#include "lane-detection.h"
#include "recognize-update.h"
#include <opencv2/opencv.hpp>
#include <stdio.h>

#define W VPE_OUTPUT_W
#define H VPE_OUTPUT_H

void detect_lane(recog_arg *arg, vector_lane *result)
{
    // Copy image
    unsigned char *raw = new unsigned char[W * H * 3];
    std::copy(arg->camera_output, arg->camera_output + W * H * 3, raw);
    cv::Mat src(H, W, CV_8UC3, raw);

    // Convert image to grayscale
    cv::cvtColor(src, src, cv::COLOR_BGR2GRAY);

    int index = 0;
    for (int i = 0; i < src.cols; i++)
    {
        index = W * i + 10;
        src.data[index] = 0;
    }

    // Convert grayscale image back to RGB image and display
    cv::cvtColor(src, src, cv::COLOR_GRAY2BGR);
    std::copy(src.data, src.data + W * H * 3, arg->display_input);
}