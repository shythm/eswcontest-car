#include "lane-detection.h"
#include "recognize-update.h"
#include <opencv2/opencv.hpp>
#include <stdio.h>

#define W VPE_OUTPUT_W
#define H VPE_OUTPUT_H

// This is possible because W(=VPE_OUT_W) and H(=VPE_OUTPUT_H) are actually constants defined with preprocessor.
// Therefore, dynamic memory allocation is not requried.
unsigned char raw[W * H * 3];

void detect_lane(recog_arg *arg, vector_lane *result)
{
    // Copy image
    std::copy(arg->camera_output, arg->camera_output + W * H * 3, raw);
    // Just wrap raw data with object
    cv::Mat src(H, W, CV_8UC3, raw);

    // Convert image to grayscale
    cv::cvtColor(src, src, cv::COLOR_BGR2GRAY);

    // Do image processing
    // someComplicatedMagincalImageProcessing(src,result)

    // Convert grayscale image back to RGB image
    cv::cvtColor(src, src, cv::COLOR_GRAY2BGR);

    // Copy processed image to display
    std::copy(src.data, src.data + W * H * 3, arg->display_input);
}