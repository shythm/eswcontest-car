#include "detect.h"
#include <algorithm>
#include <opencv2/opencv.hpp>

#define IMG_W    VPE_OUTPUT_W
#define IMG_WC   IMG_W / 2
#define IMG_H    VPE_OUTPUT_H
#define IMG_HC   IMG_H / 2
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

bool intersection(Point2f o1, Point2f p1, Point2f o2, Point2f p2, Point2f &r) {
    Point2f x  = o2 - o1;
    Point2f d1 = p1 - o1;
    Point2f d2 = p2 - o2;

    float cross = d1.x * d2.y - d1.y * d2.x;
    if (abs(cross) < 1e-8) return false;

    double t1 = (x.x * d2.y - x.y * d2.x) / cross;
    r         = o1 + d1 * t1;
    return true;
}

double getLineSlope(Point2f p1, Point2f p2) {
    double w = p2.x - p1.x;
    double h = p1.y - p2.y;

    return h / w;
}

bool detectSlope(recog_arg *arg) {
    static unsigned char cam_out[IMG_SIZE];
    copy(arg->camera_output, arg->camera_output + IMG_SIZE, cam_out);
    Mat  frame(IMG_H, IMG_W, CV_8UC3, cam_out);
    Mat  disp(IMG_H, IMG_W, CV_8UC3, arg->display_input);
    bool ret = false;

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
        makellMask(mask, IMG_W, IMG_H, 0, 40);
        init_mask = false;
    }

    // ROI 설정
    bitwise_and(frame, mmask, frame);

    // Canny 방식 윤곽선 검출
    Canny(frame, frame, 50, 150);

    // Hough 선 검출 (마지막 세 인자: threshold, minLineLength, maxLineGap)
    vector<Vec4i> lines;
    HoughLinesP(frame, lines, 1, CV_PI / 180, 20, 30, 3);

    // 가장 아래에 있는 두 직선 검출
    Vec4i line_l(0, 0, 0, 0);
    Vec4i line_r(0, 0, 0, 0);
    for (Vec4i l : lines) {
        if (l[0] < IMG_WC && l[2] < IMG_WC) { // 왼쪽 차선
            if (min(line_l[1], line_l[3]) < min(l[1], l[3])) line_l = l;
        } else if (l[0] >= IMG_WC && l[2] >= IMG_WC) { // 오른쪽 차선
            if (min(line_r[1], line_r[3]) < min(l[1], l[3])) line_r = l;
        }
    }

    // 두 직선의 교점 구하기
    Vec4i   vi(1, 1, 1, 1);
    Point2f r(0, 0);
    if (line_l.dot(vi) && line_r.dot(vi)) { // 두 직선이 유효하면
        Point2f o1(line_l[0], line_l[1]);
        Point2f p1(line_l[2], line_l[3]);
        Point2f o2(line_r[0], line_r[1]);
        Point2f p2(line_r[2], line_r[3]);
        intersection(o1, p1, o2, p2, r);

        if (getLineSlope(o1, p1) * getLineSlope(o2, p2) < 0) {
            // 두 직선의 기울기의 곱이 음수일 때 (커브길 판단 제외, 커브길에는
            // 두 직선의 기울기의 곱이 양수임)
            if (r.y > 0) {
                // 교점 y 좌표의 절댓값이 0보다 크면 경사로로 판단
                // (일반적인 차선에서는 교점 y좌표의 절댓값이 0보다 작음)
                ret = true;
            }
        }
    }

    // 검출 정보 화면 출력
    cvtColor(frame, frame, COLOR_GRAY2BGR);
#if 0
    if (r.dot(Point2f(1, 1))) {
        putText(frame, to_string(r.x), Point(25, 25), FONT_HERSHEY_SIMPLEX, 1,
                Scalar(255, 255, 255), 2);
        putText(frame, to_string(r.y), Point(25, 60), FONT_HERSHEY_SIMPLEX, 1,
                Scalar(255, 255, 255), 2);
        line(frame, r, r, Scalar(0, 0, 255), 10);
    }

    if (ret) {
        putText(frame, "slope!", Point(25, 95), FONT_HERSHEY_SIMPLEX, 1,
                Scalar(255, 255, 255), 2);
    }

    if (line_l.dot(Vec4i(1, 1, 1, 1))) {
        line(frame, Point(line_l[0], line_l[1]), Point(line_l[2], line_l[3]),
             Scalar(0, 0, 255), 2);
    }

    if (line_r.dot(Vec4i(1, 1, 1, 1))) {
        line(frame, Point(line_r[0], line_r[1]), Point(line_r[2], line_r[3]),
             Scalar(0, 0, 255), 2);
    }
#endif
    frame.copyTo(disp);

    return ret;
}