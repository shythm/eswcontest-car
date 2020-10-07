#include "detect-slope.h"
#include <algorithm>
#include <opencv2/opencv.hpp>

using namespace cv;

extern "C" {

bool detectSlope(recog_arg *arg) {
    unsigned char *cam_out = new unsigned char[VPE_OUTPUT_IMG_SIZE];
    std::copy(arg->camera_output, arg->camera_output + VPE_OUTPUT_IMG_SIZE,
              cam_out);
    cv::Mat frame(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, cam_out);
    cv::Mat disp(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, arg->display_input);
    int     ret = 0;

    // HSV로 변환
    cv::cvtColor(frame, frame, cv::COLOR_BGR2HSV);

    // 노란색 추출
    cv::Scalar lowerb(20, 50, 0);
    cv::Scalar upperb(40, 255, 255);
    cv::inRange(frame, lowerb, upperb, frame);

    // Canny Edge Detection
    cv::Canny(frame, frame, 50, 150);

#if 0
    cv::vector<cv::Vec2f> lines;
    cv::HoughLines(frame, lines, 1, CV_PI / 180, 80);
    cv::cvtColor(frame, frame, cv::COLOR_GRAY2BGR);

    for (size_t i = 0; i < lines.size(); i++) {
        float  r = lines[i][0], t = lines[i][1];
        double cos_t = cos(t), sin_t = sin(t);
        double x0 = r * cos_t, y0 = r * sin_t;
        double alpha = 1000;

        Point pt1(cvRound(x0 + alpha * (-sin_t)), cvRound(y0 + alpha * cos_t));
        Point pt2(cvRound(x0 - alpha * (-sin_t)), cvRound(y0 - alpha * cos_t));
        line(frame, pt1, pt2, Scalar(0, 0, 255), 2);
    }
#else
    vector<Vec4i> lines;
    HoughLinesP(frame, lines, 1, CV_PI / 180, 60, 30, 10);
    cvtColor(frame, frame, COLOR_GRAY2BGR);

    for (Vec4i l : lines) {
        line(frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 2);
    }
#endif

    frame.copyTo(disp);
    return ret;
}
}