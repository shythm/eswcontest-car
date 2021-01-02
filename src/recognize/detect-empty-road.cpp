#include "recognize.h"
#include <math.h>
#include <opencv2/opencv.hpp>

#define W                  VPE_OUTPUT_W
#define H                  VPE_OUTPUT_H
#define DISPLAY_EMPTY_ROAD 1

using namespace cv;
using namespace std;

void putTextAtTop(cv::Mat &frame, std::string text, cv::Scalar color) {
    int  baseline = 0;
    Size textSize =
        getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 1, 1, &baseline);

    int x = (frame.cols - textSize.width) / 2;
    int y = textSize.height + 20;

    putText(frame, text, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 1, color,
            1);
}

int decide_deriction(Mat *img) {
    const int h1 = H / 2, h2 = H - 10;
    int       countL = 0, countR = 0;
    for (int h = h1; h < h2; h++) {
        int left, right;
        int tempCountL = 0, tempCountR = 0;
        for (left = 0; left < W / 2; left++) {
            if ((*img).at<Vec3b>(h, left)[2] != 0) { // Red component is not 0
                if ((*img).at<Vec3b>(h, left)[0] !=
                    0) { // Blue component is not 0
                    // white == edge
                    tempCountL++;
                } else {
                    // red == end of ROI
                    break;
                }
            } // else: black
        }
        for (right = W; W / 2 < right; right--) {
            if ((*img).at<Vec3b>(h, right)[2] != 0) { // Red component is not 0
                if ((*img).at<Vec3b>(h, right)[0] !=
                    0) { // Blue component is not 0
                    // white == edge
                    tempCountR++;
                } else {
                    // red == end of ROI
                    break;
                }
            } // else: black
        }
        if (left < W / 2) // if left found red line
            countL += tempCountL;
        if (W / 2 < right) // if right found red line
            countR += tempCountR;
    }
    if (countL > countR) return 1; // empty right
    else if (countL == countR)
        return 0;
    else
        return -1;
}

int get_empty_road_t(recog_arg *arg) {
    static unsigned char raw_[W * H * 3];

    // read camera image
    copy(arg->camera_output, arg->camera_output + W * H * 3, raw_);
    Mat img(H, W, CV_8UC3, raw_);
    Mat img_copy;

    // paint the top part black
    img(Rect(Point(0, 0), Point(W, H / 2 - 10))) = Scalar(0, 0, 0);

    // img_copy for decide_direction() and display
    img.copyTo(img_copy);
    cvtColor(img_copy, img_copy, COLOR_BGR2GRAY);
    // Canny(img_copy, img_copy, 100, 170);
    adaptiveThreshold(img_copy, img_copy, 255, ADAPTIVE_THRESH_MEAN_C,
                      THRESH_BINARY, 3, -2);
    cvtColor(img_copy, img_copy, COLOR_GRAY2BGR);

    cvtColor(img, img, COLOR_BGR2HSV);
    // extract color
    static Scalar lowerb(0, 0, 195);
    static Scalar upperb(255, 48, 255);
    inRange(img, lowerb, upperb, img);
    Canny(img, img, 70, 140);
    vector<Vec2f> lines;
    HoughLines(img, lines, 1, CV_PI / 180, 20);

    cvtColor(img, img, COLOR_GRAY2BGR);
    // cvtColor(img, img, COLOR_GRAY2BGR);
    for (size_t i = 0; i < lines.size(); i++) {
        float rho = lines[i][0], theta = lines[i][1];
        // if (theta <= 10.f * CV_PI / 180.f || 170.f * CV_PI / 180.f <= theta)
        //     continue;
        if (70.f * CV_PI / 180.f <= theta && theta <= 110.f * CV_PI / 180.f)
            continue;
        Point  pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        pt1.x = cvRound(x0 + 1000 * (-b));
        pt1.y = cvRound(y0 + 1000 * (a));
        pt2.x = cvRound(x0 - 1000 * (-b));
        pt2.y = cvRound(y0 - 1000 * (a));
        line(img_copy, pt1, pt2, Scalar(0, 0, 255), 2, 8);
        // line(img, pt1, pt2, Scalar(0, 0, 255), 2, 8);
    }

    int ret_var = decide_deriction(&img_copy);

#if DISPLAY_EMPTY_ROAD
    img_copy(Rect(Point(0, 0), Point(W, H / 2))) = Scalar(0, 0, 0);
    if (ret_var == 0) putTextAtTop(img_copy, "NONE", Scalar(0, 255, 0));
    else if (ret_var == -1)
        putTextAtTop(img_copy, "LEFT", Scalar(0, 255, 0));
    else if (ret_var == 1)
        putTextAtTop(img_copy, "RIGHT", Scalar(0, 255, 0));
    copy(img_copy.data, img_copy.data + W * H * 3, arg->display_input);

    // copy(img.data, img.data + W * H * 3, arg->display_input);
#endif
    return ret_var;
}

// *********************************************************
// THESE FUNCTIONS ARE FOR UPDATE recog_result STRUCTURE.
// *********************************************************
extern "C" int get_empty_road(recog_arg *arg) { return get_empty_road_t(arg); }
