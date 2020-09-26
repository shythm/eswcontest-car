#include "detect-end-zone.h"
// #include "draw-txt.h"
#include "recognize-update.h"
#include <algorithm>
#include <numeric>
#include <opencv2/opencv.hpp>

#define CUT_START  150
#define CUT_END    179
#define REAL_ROW   177
#define HUE_YELLOW 30 // opencv에서는 색조 범위 0~180, 노란색이 원래는 60
#define HUE_ERROR  20
#define SAT_MIN    40
#define VAL_MIN    40

using namespace std;
using namespace cv;

bool detectEndZone(recog_arg *arg) {
    unsigned char *cam_data  = arg->camera_output;
    unsigned char *disp_data = arg->display_input;

    //이미지 받아와서 mat 클래스로 감싼다
    unsigned char *cam_copied =
        new unsigned char[VPE_OUTPUT_W * VPE_OUTPUT_H * 3];
    copy(cam_data, cam_data + VPE_OUTPUT_W * VPE_OUTPUT_H * 3, cam_copied);
    Mat src_img(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, cam_copied);

    //출력하는 거에서 roi빼고 나머지는 검은색으로 바꿈
    for (int i = 0; i < VPE_OUTPUT_H; i++) {
        if (CUT_START <= i && i <= CUT_END) continue;
        uchar *row = src_img.ptr<uchar>(i);
        for (int j = 0; j < VPE_OUTPUT_W * 3; j++) {
            row[j] = 0;
        }
    }

    //노란색으로 이미지 마스킹하기
    Mat hsvImage;
    cvtColor(src_img, hsvImage, COLOR_BGR2HSV);
    vector<Mat> channels;
    split(hsvImage, channels); //채널 분리
    threshold(channels[0], channels[0], HUE_YELLOW + HUE_ERROR, 255,
              THRESH_BINARY_INV); //범위보다 높은거 0으로 해서 덮어쓰기
    threshold(channels[0], channels[0], HUE_YELLOW - HUE_ERROR, 255,
              THRESH_BINARY); //범위보기 낮은거 0으로 해서 덮어쓰기
    channels[1] =
        channels[1] > SAT_MIN; //채도가 너무 낮으면 0으로, 멀쩡하면 255로
    channels[2] =
        channels[2] > VAL_MIN; //명도가 너무 낮으면 0으로, 멀쩡하면 255로
    Mat yellow_masked = channels[0];
    for (int i = 1; i < 3; ++i) //채도 명도 낮은 픽셀은 없애기
        bitwise_and(channels[i], yellow_masked, yellow_masked);

    //이제부터 이진화해서 3채널로 바꾼다.
    Mat output_img = Mat::zeros(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3);
    yellow_masked =
        (yellow_masked > 0); //값을 가지고 있는 친구는 전부 255가 되는 연산
    adaptiveThreshold(yellow_masked, yellow_masked, 255, ADAPTIVE_THRESH_MEAN_C,
                      THRESH_BINARY, 9, -10); //외부에 대한 민감도 줄이기
    cvtColor(yellow_masked, output_img, COLOR_GRAY2BGR);

    //판단 부분
    uchar *row = output_img.ptr<uchar>(
        REAL_ROW); // 최종적으로 보고싶은 한줄의 첫번째 주소값을 받아온다.
    uint rising_edge_count = 0;
    for (int i = 0; i < VPE_OUTPUT_W * 3 - 1; i++) {
        if (row[i + 1] > row[i]) rising_edge_count++;
    }
    if (rising_edge_count >= 4) return true;
    else
        return false;
    /* 잘되는거 확인하기 위해 디스플레이에 띄우는 부분
    //출력
    Mat car_disp_img(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, disp_data);
    output_img.copyTo(car_disp_img);

    // REAL ROW에 대해 rising edge 개수 세서 판단하기
    uchar *row = output_img.ptr<uchar>(
        REAL_ROW); // 최종적으로 보고싶은 한줄의 첫번째 주소값을 받아온다.
    uint rising_edge_count = 0;
    for (int i = 0; i < VPE_OUTPUT_W * 3 - 1; i++) {
        if (row[i + 1] > row[i]) rising_edge_count++;
    }
    static char edge[6];
    sprintf(edge, "%d", rising_edge_count);

    drawTxt(edge, disp_data, 10, 50, VPE_OUTPUT_W, VPE_OUTPUT_H);
    */
}
