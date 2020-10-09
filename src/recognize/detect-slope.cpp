#include "detect-slope.h"
#include <algorithm>
#include <opencv2/opencv.hpp>

#define IMG_W    VPE_OUTPUT_W
#define IMG_H    VPE_OUTPUT_H
#define IMG_SIZE VPE_OUTPUT_IMG_SIZE

using namespace cv;
using namespace std;

void makellMask(unsigned char *mask, int w, int h, int offset, int size) {
    int a1 = offset;
    int a2 = a1 + size;
    int b2 = w - offset;
    int b1 = b2 - size;

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            if ((j > a1 && j < a2) || (j > b1 && j < b2)) {
                mask[w * i + j] = 255;
                continue;
            }
            mask[w * i + j] = 0;
        }
    }
}

bool detectSlope(recog_arg *arg) {
    unsigned char *cam_out = new unsigned char[IMG_SIZE];
    copy(arg->camera_output, arg->camera_output + IMG_SIZE, cam_out);
    Mat frame(IMG_H, IMG_W, CV_8UC3, cam_out);
    Mat disp(IMG_H, IMG_W, CV_8UC3, arg->display_input);
    int ret = 0;

    // HSV로 변환
    cvtColor(frame, frame, cv::COLOR_BGR2HSV);

    // 노란색(40 +-20) 추출
    Scalar lowerb(20, 63, 168);
    Scalar upperb(60, 255, 255);
    inRange(frame, lowerb, upperb, frame);

    // 마스킹 이미지 만들기
    static unsigned char mask[IMG_W * IMG_H];
    static bool          init_mask = true;
    static Mat           mmask(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC1, mask);
    if (init_mask) {
        makellMask(mask, IMG_W, IMG_H, 10, 40);
        init_mask = false;
    }

    // ROI 설정
    bitwise_and(frame, mmask, frame);

    // Canny 방식 윤곽선 검출
    Canny(frame, frame, 50, 150);

    // Hough 선 검출 (마지막 세 인자: threshold, minLineLength, maxLineGap)
    vector<Vec4i> lines;
    HoughLinesP(frame, lines, 1, CV_PI / 180, 20, 30, 3);

    // 검출 정보 화면 출력
    cvtColor(frame, frame, COLOR_GRAY2BGR);
    if (lines.size()) {
        float slope_l_sum = 0, slope_r_sum = 0;
        int   slope_l_cnt = 0, slope_r_cnt = 0;

        for (Vec4i l : lines) {
            line(frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255),
                 2);

            float w = l[0] - l[2];
            float h = l[3] - l[1];
            if (w) {
                float s = (double)h / w;
                if ((l[0] < IMG_W / 2) && (l[2] < IMG_W / 2)) { // 왼쪽 차선
                    slope_l_sum += s;
                    slope_l_cnt++;
                } else if ((l[0] >= IMG_W / 2) &&
                           ((l[0] >= IMG_W / 2))) { // 오른쪽 차선
                    slope_r_sum += s;
                    slope_r_cnt++;
                }
            }
        }

        if (slope_l_cnt) {
            putText(frame, "L: " + std::to_string(slope_l_sum / slope_l_cnt),
                    Point(25, 25), FONT_HERSHEY_SIMPLEX, 1,
                    Scalar(255, 255, 255), 2);
        }
        if (slope_r_cnt) {
            putText(frame, "R: " + std::to_string(slope_r_sum / slope_r_cnt),
                    Point(25, 60), FONT_HERSHEY_SIMPLEX, 1,
                    Scalar(255, 255, 255), 2);
        }
    }

    // cvtColor(frame, frame, COLOR_GRAY2BGR);
    frame.copyTo(disp);

    delete[] cam_out;
    return ret;
}