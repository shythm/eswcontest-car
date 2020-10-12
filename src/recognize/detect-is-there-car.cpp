#include "detect-is-there-car.h"
#include "recognize-lib.h"
#include <math.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

vector_car_pos detectIsThereCar(recog_arg *arg) {
    unsigned char *      cam_data = arg->camera_output;
    static unsigned char src_data[VPE_OUTPUT_W * VPE_OUTPUT_H * 3];
    copy(cam_data, cam_data + sizeof(src_data), src_data);
    Mat src(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, src_data);

    Mat temp;
    cvtColor(src, temp, COLOR_BGR2GRAY);

#define ROI_ENTER   30
#define ROI_EXIT    100
#define ROI_INTER   10
#define ROI_SLOPE   1.75f
#define ROI_SLOPE_C 1.25f

#define LANE_DIST  160
#define DIFF_SHIFT 4

    int    i = 0;
    uchar *row;
    int    leftScore, rightScore, centerScore;
    leftScore = rightScore = centerScore = 0;
    for (; i < ROI_ENTER; i++) {
        row = temp.ptr(i);
        for (int j = 0; j < VPE_OUTPUT_W; j++) { row[j] = 0; }
    }
    int thr1, thr2, thr3, thr4;

    for (; i < ROI_EXIT; i++) {
        row  = temp.ptr(i);
        thr1 = VPE_OUTPUT_W / 2 - (i + ROI_INTER) * ROI_SLOPE;
        thr2 = VPE_OUTPUT_W / 2 - (i + ROI_INTER) * ROI_SLOPE_C;
        thr3 = VPE_OUTPUT_W / 2 + (i + ROI_INTER) * ROI_SLOPE_C;
        thr4 = VPE_OUTPUT_W / 2 + (i + ROI_INTER) * ROI_SLOPE;
        // row[thr1] = row[thr2] = 255;
        for (int j = 0; j < VPE_OUTPUT_W - 1; j++) {
            int diff = abs(row[j] - row[j + 1]) >> DIFF_SHIFT;
            if (diff > 0) diff = 255;
            if (j < thr1) {
                leftScore += diff;
                row[j] = diff;
            } else if (j > thr4) {
                rightScore += diff;
                row[j] = diff;
            } else if (thr2 < j && j < thr3) {
                centerScore += diff;
                row[j] = diff;
            } else {
                row[j] = 128;
            }
        }
    }
    for (; i < VPE_OUTPUT_H; i++) {
        row = temp.ptr(i);
        for (int j = 0; j < VPE_OUTPUT_W; j++) { row[j] = 0; }
    }
    cvtColor(temp, temp, COLOR_GRAY2BGR);

    // Car position
    vector_car_pos ret;
    ret.left   = false;
    ret.center = false;
    ret.right  = false;

    if ((leftScore < rightScore) && (leftScore < centerScore)) {
        // If left is minimum
        ret.left = true;
        putText(temp, "LEFT", Point(10, 50), FONT_HERSHEY_PLAIN, 1,
                Scalar(0, 0, 255));
    } else if ((rightScore < centerScore) && (rightScore < leftScore)) {
        // If right is minimum
        ret.right = true;
        putText(temp, "RIGHT", Point(10, 50), FONT_HERSHEY_PLAIN, 1,
                Scalar(0, 0, 255));
    } else {
        // If center is minimum
        ret.center = true;
        putText(temp, "STRAIGHT", Point(10, 50), FONT_HERSHEY_PLAIN, 1,
                Scalar(0, 0, 255));
    }

    // show image on display
    Mat car_disp_img(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, arg->display_input);
    temp.copyTo(car_disp_img);

    return ret;
}