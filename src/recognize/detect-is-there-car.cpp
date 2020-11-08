#include "detect-is-there-car.h"
#include "recognize-lib.h"
#include <opencv2/opencv.hpp>

#define ABS(x) (((x) > 0) ? (x) : -(x))

using namespace cv;
using namespace std;

vector_car_pos detectIsThereCar(recog_arg *arg) {
    unsigned char *cam_data  = arg->camera_output;
    unsigned char *disp_data = arg->display_input;
    // Mat            output_img();
    //이미지 받아와서 mat 클래스로 감싼다
    static unsigned char cam_copied[VPE_OUTPUT_W * VPE_OUTPUT_H * 3];

    copy(cam_data, cam_data + sizeof(cam_copied), cam_copied);
    Mat src_img(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, cam_copied);
    Mat gray_img = Mat::zeros(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC1);
    cvtColor(src_img, gray_img, COLOR_BGR2GRAY);
    Mat gray_roi1 = gray_img.clone();

    // mask
    for (int y = gray_img.rows / 5; y < (int)(gray_img.rows * 3.f / 5.f); y++) {
        for (int x = 0; x < gray_img.cols; x++) {
            gray_roi1.at<uchar>(y, x) = 0;
        }
    }

    // edge
    Mat           edge_img;
    vector<Vec2f> lines;
    Canny(gray_roi1, edge_img, 50, 100);
    HoughLines(edge_img, lines, 1, CV_PI / 180, 250);

    // show image on display
    Mat car_disp_img(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, disp_data);
    src_img.copyTo(car_disp_img);

    // make boa
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

    ret.left   = false;
    ret.center = false;
    ret.right  = false;

    // 함수 작성
    return ret;
}