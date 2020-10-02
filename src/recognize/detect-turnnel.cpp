#include "detect-turnnel.h"
#include "recognize-update.h"
#include <algorithm>
#include <numeric>
#include <opencv2/opencv.hpp>
#include <stdio.h>

#define FRAME_ERROR 30

using namespace cv;
using namespace std;

bool detectTurnnel(recog_arg *arg) {
    int        cur_frame_value_sum = 0;
    int        cur_mean_frame      = 0;
    static int total_mean_frame    = 0;

    //현재 프레임의 평균값을 구한다.
    for (int i = 0; i < VPE_OUTPUT_IMG_SIZE;
         i++) { // VPE_OUTPUT_IMG_SIZE = 가로*세로*3
        cur_frame_value_sum += arg->camera_output[i];
    }
    cur_mean_frame = cur_frame_value_sum / (VPE_OUTPUT_IMG_SIZE * 3);

    //테스트용: 현재 프레임의 평균값을 출력한다.
    unsigned char *cam_data  = arg->camera_output;
    unsigned char *disp_data = arg->display_input;

    unsigned char *cam_copied = new unsigned char[VPE_OUTPUT_IMG_SIZE];
    copy(cam_data, cam_data + VPE_OUTPUT_IMG_SIZE, cam_copied);
    Mat src_img(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, cam_copied);

    static char test_brightness[6];
    sprintf(test_brightness, "%d", cur_mean_frame);
    putText(src_img, test_brightness, Point(10, 50), CV_FONT_HERSHEY_SIMPLEX, 1,
            Scalar(0, 0, 0), 1);

    Mat disp_img(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, disp_data);
    src_img.copyTo(disp_img);

    // //이전 프레임보다 확 떨어지면
    // if (cur_mean_frame < total_mean_frame - FRAME_ERROR) { return true }
    // total_mean_frame = (total_mean_frame + cur_mean_frame) / 2;

    //평균 값보다 일정 값 이상 어두워지면 터널 진입으로 판단.
    return 1;
}