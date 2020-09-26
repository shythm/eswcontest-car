#include "detect-end-zone.h"
#include "recognize-update.h"
#include <algorithm>
#include <numeric>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define CUT_START  90
#define CUT_END    135
#define HUE_YELLOW 30 // opencv에서는 색조 범위 0~180
#define HUE_ERROR  20
#define SAT_MIN    40
#define VAL_MIN    40

bool detectEndZone(recog_arg *arg) {

    unsigned char *cam_data = arg->camera_output;
    // int            cam_w     = VPE_OUTPUT_W;
    // int            cam_h     = VPE_OUTPUT_H;
    unsigned char *disp_data = arg->display_input;
    // int            disp_w    = VPE_OUTPUT_W;
    // int            disp_h    = VPE_OUTPUT_H;

    //이미지 받아와서 mat 클래스로 감싼다
    unsigned char *cam_copied =
        new unsigned char[VPE_OUTPUT_W * VPE_OUTPUT_H * 3];
    copy(cam_data, cam_data + VPE_OUTPUT_W * VPE_OUTPUT_H * 3, cam_copied);
    Mat src_img(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, cam_copied);

    // region of interst 영역을 자른다.
    Mat cut_img =
        src_img(Range(CUT_START, CUT_END), Range(0, VPE_OUTPUT_W)).clone();

    // // test용 출력 부분
    // for (int i = 0; i < VPE_OUTPUT_H; i++) {
    //     if (CUT_START <= i && i <= CUT_END) continue;
    //     uchar *row = src_img.ptr<uchar>(i);
    //     for (int j = 0; j < VPE_OUTPUT_W * 3; j++) {
    //         row[j] = 0;
    //     }
    // }

    //노란색으로 이미지 마스킹하기
    Mat hsvImage;
    cvtColor(src_img, hsvImage, COLOR_BGR2HSV);
    // cvtColor(cut_img, hsvImage, COLOR_BGR2HSV); // hsv로 변환
    vector<Mat> channels;
    split(hsvImage, channels); //채널 분리
    // Mat mask1;
    threshold(channels[0], channels[0], HUE_YELLOW + HUE_ERROR, 255,
              THRESH_BINARY_INV); //범위보다 높은거 0
    // Mat mask2;
    threshold(channels[0], channels[0], HUE_YELLOW - HUE_ERROR, 255,
              THRESH_BINARY); //범위보다 낮은거 0
    // Mat hueMask = mask1 & mask2;
    channels[1] =
        channels[1] > SAT_MIN; //채도가 너무 낮으면 0으로, 멀쩡하면 1로
    channels[2] =
        channels[2] > VAL_MIN; //명도가 너무 낮으면 0으로, 멀쩡하면 1로
    Mat yellow_masked = channels[0];
    for (int i = 1; i < 3; ++i)
        bitwise_and(channels[i], yellow_masked, yellow_masked);
    yellow_masked =
        (yellow_masked > 0); //값을 가지고 있는 친구는 전부 255가 되는 연산
    adaptiveThreshold(yellow_masked, yellow_masked, 255, ADAPTIVE_THRESH_MEAN_C,
                      THRESH_BINARY, 9, -10);

    //이진화 해보자
    Mat binarization = Mat::zeros(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3);
    cvtColor(yellow_masked, binarization, COLOR_GRAY2BGR);
    //출력
    Mat car_disp_img(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, disp_data);
    binarization.copyTo(car_disp_img);
    // for(int row=0;row<yellow_masked.rows;++row) {
    //     for(int col=0;col<yellow_masked.cols;++col) {
    //         if( !channels[0].data[row*yellow_masked.cols + col] ) continue;
    //         binarization
    //     }
    // }

    // Mat binarization = Mat::zeros(yellow_masked.rows, yellow_masked.cols,
    // CV_8U); for(int row=0;row<yellow_masked.rows;++row) {
    //     for(int col=0;col<yellow_masked.cols;++col) {
    //         uchar v1 = channels[0].data[row*yellow_masked.cols + col];
    //         uchar v2 = channels[1].data[row*yellow_masked.cols + col];
    //         uchar v3 = channels[2].data[row*yellow_masked.cols + col];
    //         binarization.data[row*yellow_masked.cols + col] = (v1 && v2 && v3
    //         ? 255 : 0);
    //     }
    // }

    // threshold(channels[1], channels[1], SAT_MIN, 255, THRESH_BINARY);
    // threshold(channels[2], channels[2], VAL_MIN, 255, THRESH_BINARY);

    // int lowH = (HUE_YELLOW - HUE_ERROR >= 0) ? HUE_YELLOW - HUE_ERROR
    //                                          : HUE_YELLOW - HUE_ERROR +
    //                                          180;
    // int hingH = (HUE_YELLOW + HUE_ERROR <= 180) ? HUE_YELLOW + HUE_ERROR
    //                                             : HUE_YELLOW + HUE_ERROR
    //                                             - 180;

    // binarization(이진화)

    // salt와 pepper 없애기

    //한 row에 대해 rising edge 개수 세서 판단하기
    return false;
}