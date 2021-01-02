#include "recognize.h"
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

bool getIntersection(Point2i o1, Point2i p1, Point2i o2, Point2i p2,
                     Point2i &r) {
    Point2i x  = o2 - o1;
    Point2i d1 = p1 - o1;
    Point2i d2 = p2 - o2;

    int cross = d1.x * d2.y - d1.y * d2.x;
    if (cross == 0) return false;

    int t1 = (x.x * d2.y - x.y * d2.x) / cross;
    r      = o1 + d1 * t1;

    return true;
}

float getLineSlope(Point2f p1, Point2f p2) {
    float w = p2.x - p1.x;
    float h = p1.y - p2.y;

    return (float)w / h;
}

#define MAX_SLOPE_DIST 1.0f
#define MIN_INTER_Y    IMG_H / 3

recog_slope_t detectSlope(recog_arg *arg) {
    static uchar cam_out[IMG_SIZE];

    static const Vec4i v4i(1, 1, 1, 1); // vector four i
    static const Vec2i v2i(1, 1);       // vector two i

    recog_slope_t ret = SLOPE_NONE;

    // 카메라 데이터 복사 (원본 데이터 보존)
    copy(arg->camera_output, arg->camera_output + IMG_SIZE, cam_out);
    Mat frame(IMG_H, IMG_W, CV_8UC3, cam_out);

    // HSV로 변환
    cvtColor(frame, frame, cv::COLOR_BGR2HSV);

    // 노란색(40 +-20) 추출
    static Scalar lowerb(20, 60, 0);
    static Scalar upperb(60, 255, 255);
    inRange(frame, lowerb, upperb, frame);

    // 마스킹 이미지 만들고 ROI 설정
    static uchar mask[IMG_W * IMG_H];
    static Mat   mmask(IMG_H, IMG_W, CV_8UC1, mask);
    static bool  init_mask = true;
    if (init_mask) {
        makellMask(mask, IMG_W, IMG_H, 0, 60);
        init_mask = false;
    }
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

    // 두 직선의 교점과 기울기 구하기
    Point2i intersection(0, 0); // intersection result (x, y)
    Vec2f   slope(0, 0);        // slope (left, right)
    float   slope_dist = 0.0f;  // slope distance

    if (line_l.dot(v4i) && line_r.dot(v4i)) { // 두 직선이 유효하면
        Point2i o1(line_l[0], line_l[1]);
        Point2i p1(line_l[2], line_l[3]);
        Point2i o2(line_r[0], line_r[1]);
        Point2i p2(line_r[2], line_r[3]);

        slope[0]   = getLineSlope(o1, p1);
        slope[1]   = getLineSlope(o2, p2);
        slope_dist = abs(slope.dot(v2i));

        if (slope_dist < MAX_SLOPE_DIST) {
            // 두 직선의 기울기 합의 절댓값이 일정 수준 미만일 때
            // (커브길 판단 제외, 커브길에는 slope_dist가 크다.)
            if (getIntersection(o1, p1, o2, p2, intersection)) {
                // 두 직선의 교점의 y좌표가 크다는 것은 화면 상에서 y좌표가
                // 아래에 위치한다는 뜻이므로 경사가 급하다고 볼 수 있다.
                // y좌표가 크다는 것은 미리 지정된 값(MIN_INTER_Y)를 이용해
                // 결정한다.
                if (intersection.y > MIN_INTER_Y) { ret = SLOPE_DOWNHILL; }
                if (intersection.y < -MIN_INTER_Y) { ret = SLOPE_UPHILL; }
            }
        }
    }

    // 검출 정보 화면 출력
#if 0
    cvtColor(frame, frame, COLOR_GRAY2BGR);

    if (intersection.dot(v2i)) { // 교점이 존재할 때 기본 정보 표시
        putText(frame,
                "x: " + to_string(intersection.x) +
                    ", y: " + to_string(intersection.y),
                Point(25, 25), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255),
                2);
        putText(frame, "sd: " + to_string(slope_dist), Point(25, 60),
                FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
        line(frame, intersection, intersection, Scalar(0, 0, 255), 10);
    }
    string str_res = "";
    if (ret == SLOPE_UPHILL) {
        str_res = "UPHILL!";
    } else if (ret == SLOPE_DOWNHILL) {
        str_res = "DOWNHILL!";
    }
    putText(frame, str_res, Point(25, 95), FONT_HERSHEY_SIMPLEX, 1,
            Scalar(255, 255, 255), 2);
    if (line_l.dot(Vec4i(1, 1, 1, 1))) { // 왼쪽 차선 존재하면 그리기
        line(frame, Point(line_l[0], line_l[1]), Point(line_l[2], line_l[3]),
             Scalar(0, 0, 255), 2);
    }
    if (line_r.dot(Vec4i(1, 1, 1, 1))) { // 오른쪽 차선 존재하면 그리기
        line(frame, Point(line_r[0], line_r[1]), Point(line_r[2], line_r[3]),
             Scalar(255, 0, 0), 2);
    }

    Mat disp(IMG_H, IMG_W, CV_8UC3, arg->display_input);
    frame.copyTo(disp);
#endif

    return ret;
}

// *********************************************************
// THESE FUNCTIONS ARE FOR UPDATE recog_result STRUCTURE.
// *********************************************************
extern "C" recog_slope_t get_slope(recog_arg *arg) { return detectSlope(arg); }