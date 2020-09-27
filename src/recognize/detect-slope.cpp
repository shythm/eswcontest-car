#include "detect-slope.h"
#include <algorithm>
#include <opencv2/opencv.hpp>

extern "C" {

bool detectSlope(recog_arg *arg) {
    unsigned char *cam_out = new unsigned char[VPE_OUTPUT_IMG_SIZE];
    std::copy(arg->camera_output, arg->camera_output + VPE_OUTPUT_IMG_SIZE,
              cam_out);
    cv::Mat frame(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, cam_out);
    cv::Mat disp(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, arg->display_input);
    int     ret = 0;

    frame = ~frame;
    frame.copyTo(disp);

    return ret;
}
}