#include "detect-is-there-car.h"
#include "recognize-lib.h"
#include <math.h>
#include <opencv2/opencv.hpp>

#define ABS(x) (((x) > 0) ? (x) : -(x))

using namespace cv;
using namespace std;

vector_car_pos detectIsThereCar(recog_arg *arg) {
    unsigned char *cam_data  = arg->camera_output;
    unsigned char *disp_data = arg->display_input;

    // Copy cam data and convert it to Mat
    static unsigned char cam_copied[VPE_OUTPUT_W * VPE_OUTPUT_H * 3];
    copy(cam_data, cam_data + sizeof(cam_copied), cam_copied);
    Mat src_img(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, cam_copied);

    // Convert to grayscale image
    Mat gray_img = Mat::zeros(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC1);
    cvtColor(src_img, gray_img, COLOR_BGR2GRAY);

    Mat gray_roi1 = gray_img.clone();

    // Apply mask on ROI image
    for (int y = gray_img.rows / 5; y < (int)(gray_img.rows * 3.f / 5.f); y++) {
        for (int x = 0; x < gray_img.cols; x++) {
            gray_roi1.at<uchar>(y, x) = 0;
        }
    }

    Mat temp;

    // Make boa function
    unsigned char boa[100];
    for (int i = 0; i < 100; i++) boa[i] = -ABS((255.f / 50) * (i - 50)) + 255;

    Mat  boa_mat(1, 100, CV_8UC1, boa);
    long boa_l = 0, boa_c = 0, boa_r = 0;
    Mat  gray_l =
        gray_img(Range(gray_img.rows / 5, (int)(gray_img.rows * 3.f / 5.f)),
                 Range(0, 99));
    Mat gray_c =
        gray_img(Range(gray_img.rows / 5, (int)(gray_img.rows * 3.f / 5.f)),
                 Range(110, 209));
    Mat gray_r =
        gray_img(Range(gray_img.rows / 5, (int)(gray_img.rows * 3.f / 5.f)),
                 Range(220, 319));

    for (int i = 0; i < gray_l.rows; i++) {
        boa_l += boa_mat.dot(gray_l.row(i));
        boa_c += boa_mat.dot(gray_c.row(i));
        boa_r += boa_mat.dot(gray_r.row(i));
    }
    printf("L: %ld, C: %ld, R: %ld\n", boa_l, boa_c, boa_r);

    vector_car_pos ret;

    // Car position
    vector_car_pos ret;
    ret.left   = false;
    ret.center = false;
    ret.right  = false;

    if ((boa_l < boa_r) && (boa_l < boa_c)) {
        // If left is minimum
        ret.left = true;
        putText(temp, "LEFT", Point(10, 50), FONT_HERSHEY_PLAIN, 1,
                Scalar(0, 0, 255));
    } else if ((boa_r < boa_c) && (boa_r < boa_l)) {
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