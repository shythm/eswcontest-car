#include "recognize-lib.h"
#include <opencv2/opencv.hpp>

#define W VPE_OUTPUT_W
#define H VPE_OUTPUT_H

using namespace cv;
using namespace std;

const Size sizeOrigin = Size(W, H);
const Size sizeSmall  = Size(W / 8, H / 8);

void getValidY(Mat &img, vector<int> &out) {
    uchar *row;
    int    i, j, cnt;

    for (i = 0; i < img.size().height; i++) {
        row = img.ptr(i);
        cnt = 0;

        for (j = 0; j < img.size().width; j++) {
            if (row[j]) { cnt++; }
        }

        if (cnt > img.size().width / 2) { out.push_back(i); }
    }
}

float detectStopLine(recog_arg *arg) {
    static uchar raw[W * H * 3];

    copy(arg->camera_output, arg->camera_output + W * H * 3, raw);
    Mat img(H, W, CV_8UC3, raw);

    resize(img, img, sizeSmall);
    cvtColor(img, img, COLOR_BGR2HSV);

    static const Scalar l(0, 0, 200);    // lower white
    static const Scalar u(255, 48, 255); // upper white
    inRange(img, l, u, img);

    vector<int> validY;
    getValidY(img, validY);

    float ret = -1;

    int yCnt = validY.size();
    if (yCnt) {
        int yPos = 0;
        for (int y : validY) { yPos += y; }
        yPos /= yCnt;
        ret = (float)yPos / (float)sizeSmall.height;
    }

#if 0
    // Restore for copying to display
    cvtColor(img, img, COLOR_GRAY2BGR);
    resize(img, img, sizeOrigin, INTER_NEAREST);
    putText(img, "pos: " + to_string(ret), Point(25, 30), FONT_HERSHEY_SIMPLEX,
            1, Scalar(0, 255, 0), 2);

    copy(img.data, img.data + W * H * 3, arg->display_input);
#endif

    return ret;
}

// *********************************************************
// FOR UPDATE recog_result STRUCTURE.
// *********************************************************
extern "C" float get_stop_line(recog_arg *arg) { return detectStopLine(arg); }