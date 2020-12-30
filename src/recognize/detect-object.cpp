#include "recognize-lib.h"
#include <algorithm>
#include <numeric>
#include <opencv2/opencv.hpp>

struct TrafficLights {
    bool red, yellow, green, left, right;
};
struct Point {
    int x;
    int y;
};
struct StopObstacle {
    bool         exist;
    struct Point center;
    float        area;
};

enum Shape : int {
    Circle    = 0b1,
    Rectangle = 0b10,
    Left      = 0b100,
    Right     = 0b1000,
    Undefined = 0b10000
};
typedef std::vector<cv::Point> Contour;

// OpenCV HSV Value Range
// H: 0-179, S: 0-255, V: 0-255
cv::Mat maskImage(cv::Mat &frame, int hStart, int hEnd, int sMin, int sMax,
                  int vMin, int vMax) {
    cv::Mat hsvImage;
    cv::cvtColor(frame, hsvImage, cv::COLOR_BGR2HSV);

    std::vector<cv::Mat> channels;
    cv::split(hsvImage, channels);

    if (hStart < hEnd)
        cv::bitwise_and(hStart <= channels[0], channels[0] <= hEnd,
                        channels[0]);
    else
        cv::bitwise_or(hStart <= channels[0], channels[0] <= hEnd, channels[0]);

    cv::bitwise_and(sMin <= channels[1], channels[1] <= sMax, channels[1]);
    cv::bitwise_and(vMin <= channels[2], channels[2] <= vMax, channels[2]);

    cv::Mat mask = channels[0];
    for (int i = 1; i < 3; ++i) cv::bitwise_and(channels[i], mask, mask);

    cv::Mat grey = cv::Mat::zeros(mask.rows, mask.cols, CV_8U);
    for (int row = 0; row < mask.rows; ++row) {
        for (int col = 0; col < mask.cols; ++col) {
            uchar v1 = channels[0].data[row * mask.cols + col];
            uchar v2 = channels[1].data[row * mask.cols + col];
            uchar v3 = channels[2].data[row * mask.cols + col];
            grey.data[row * mask.cols + col] = (v1 && v2 && v3 ? 255 : 0);
        }
    }

    return grey;
}

Shape labelPolygon(Contour &c) {
    double  peri = cv::arcLength(c, true);
    Contour approx;
    cv::approxPolyDP(c, approx, 0.02 * peri, true);
    bool isConvex = cv::isContourConvex(approx);

    if ((int)approx.size() == 4 && isConvex) return Rectangle;

    if ((int)approx.size() == 7 && !isConvex) {
        int center =
            std::accumulate(approx.begin(), approx.end(), cv::Point(0, 0)).x /
            7;
        int leftCount, rightCount;
        leftCount = rightCount = 0;

        for (int i = 0; i < 7; ++i) {
            if (approx[i].x - center >= 0) ++rightCount;
            else
                ++leftCount;
        }

        if (leftCount >= rightCount) return Left;
        else
            return Right;
    }

    if (approx.size() > 7 && isConvex) return Circle;

    return Undefined;
}

std::string shapeToString(Shape s) {
    switch (s) {
    case Circle:
        return "Circle";

    case Left:
        return "Left";

    case Right:
        return "Right";

    case Rectangle:
        return "Rectangle";

    case Undefined:
        return "Undefined";
    }

    return "Error";
}

std::vector<Contour> findShapes(Shape shapeToFind, cv::Mat &grey, int minArea,
                                int maxArea) {
    std::vector<Contour> contours;
    cv::findContours(grey, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    std::vector<Contour> found;

    for (int i = 0; i < (int)contours.size(); ++i) {
        Contour &c    = contours[i];
        double   area = cv::contourArea(c);

        if (area != 0 && minArea <= area && area <= maxArea) {
            Shape shape = labelPolygon(c);
            if (shape & shapeToFind) found.push_back(c);
        }
    }

    return found;
}

void putTextAtCenter(cv::Mat &frame, std::string text, cv::Scalar color) {
    int      baseline = 0;
    cv::Size textSize =
        cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 1, 1, &baseline);

    int x = (frame.cols - textSize.width) / 2;
    int y = (frame.rows - textSize.height) / 2;

    cv::putText(frame, text, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 1,
                color, 1);
}

// < 신호등 원의 크기에 대하여 >
// 바퀴가 흰색 정지선에 있을 때: 크기 280정도 나옴.
// 앞바퀴가 흰색 정지선을 완전히 넘었을 때: 300정도 나옴.
// 앞바퀴가 흰색 정지선의 뒤에 있을 때: 250정도 나옴.
TrafficLights detectLights(cv::Mat &frame, cv::Mat *drawBoard, int minArea,
                           int maxArea) {
    cv::Mat blackMasked = maskImage(frame, 0, 179, 0, 255, 0, 50);

    std::vector<Contour> circles =
        findShapes(Circle, blackMasked, minArea, maxArea);

    if (circles.size() >= 4) {
        printf("Circles Count: %d\n", (int)circles.size());
        for (const Contour &c : circles) {
            cv::Moments m  = cv::moments(c);
            double      cX = m.m10 / m.m00;
            double      cY = m.m01 / m.m00;
            printf("%lf, (%lf, %lf)\n", cv::contourArea(c), cX, cY);
        }
        printf("\n");
    }

    if (drawBoard) {
        cv::drawContours(*drawBoard, circles, -1, cv::Scalar(255, 0, 0), 2);
    }

    // cv::Mat gray;
    // cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    // cv::GaussianBlur(gray, gray, cv::Size(3, 3), 0);

    // std::vector<cv::Vec3f> circles;
    // // 감지하려는 원의 최소 반지름이 r일 때, 원의 중심들간 최소
    // 거리(minDist)는
    // // 2r이다.
    // cv::HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, 2 * minRadius, 50,
    // 50,
    //                  minRadius, maxRadius);

    // printf("Circles Count: %d\n", (int)circles.size());
    // for (const cv::Vec3f &c : circles)
    //     printf("center: (%f, %f), radius: %f\n", c[0], c[1], c[2]);
    // printf("\n");
    // sleep(1);

    TrafficLights result;
    result.red    = false;
    result.yellow = false;
    result.green  = false;
    result.left   = false;
    result.right  = false;
    return result;
}

#define IMG_H VPE_OUTPUT_H
#define IMG_W VPE_OUTPUT_W

struct TrafficLights detectLights(recog_arg *arg) {
    unsigned char *srcBuf = arg->camera_output;
    unsigned char *outBuf = arg->display_input;

    static unsigned char srcCopied[IMG_H * IMG_W * 3];
    std::copy(srcBuf, srcBuf + IMG_H * IMG_W * 3, srcCopied);
    cv::Mat       srcRGB(IMG_H, IMG_W, CV_8UC3, srcCopied);
    TrafficLights result;

    if (outBuf) {
        cv::Mat dstRGB(IMG_H, IMG_W, CV_8UC3, outBuf);
        result = detectLights(srcRGB, &dstRGB, 1, 100000);
    } else
        result = detectLights(srcRGB, NULL, 1, 100000);

    return result;
}

struct StopObstacle detectStopObstacle(cv::Mat &frame, cv::Mat *drawBoard,
                                       int minArea, int maxArea) {
    cv::Mat redMasked = maskImage(frame, -15, 15, 90, 255, 60, 255);
    std::vector<Contour> rectFound =
        findShapes(Rectangle, redMasked, minArea, maxArea);

    if (rectFound.size() > 0) {
        if (drawBoard) {
            std::vector<Contour> toDraw;
            toDraw.push_back(rectFound[0]);
            cv::drawContours(*drawBoard, toDraw, -1, cv::Scalar(0, 0, 255), 2);
            putTextAtCenter(*drawBoard, "Stop!", cv::Scalar(0, 0, 255));
        }

        StopObstacle result;
        result.exist  = true;
        cv::Moments m = cv::moments(rectFound[0]);
        result.area   = (float)m.m00;
        result.center = {(int)(m.m10 / m.m00), (int)(m.m01 / m.m00)};
        return result;
    }

    StopObstacle result;
    result.exist  = false;
    result.area   = 0;
    result.center = {0, 0};
    return result;
}

struct StopObstacle detectStopObstacle(recog_arg *arg) {
    unsigned char *srcBuf = arg->camera_output;
    unsigned char *outBuf = arg->display_input;

    static unsigned char srcCopied[IMG_W * IMG_H * 3];
    std::copy(srcBuf, srcBuf + IMG_W * IMG_H * 3, srcCopied);
    cv::Mat      srcRGB(IMG_H, IMG_W, CV_8UC3, srcCopied);
    StopObstacle result;

    if (outBuf) {
        cv::Mat dstRGB(IMG_H, IMG_W, CV_8UC3, outBuf);
        result = detectStopObstacle(srcRGB, &dstRGB, 100, 100000);
    } else
        result = detectStopObstacle(srcRGB, NULL, 100, 100000);

    return result;
}

// *********************************************************
// THESE FUNCTIONS ARE FOR UPDATE recog_result STRUCTURE.
// *********************************************************
extern "C" recog_stop_obstacle_t get_stop_obstacle(recog_arg *arg) {
    static recog_stop_obstacle_t result;

    struct StopObstacle detected = detectStopObstacle(arg);
    if (detected.exist) {
        result.area  = detected.area;
        result.pos_x = detected.center.x;
        result.pos_y = detected.center.y;
    } else {
        result.area  = 0;
        result.pos_x = -1;
        result.pos_y = -1;
    }

    return result;
}
extern "C" recog_traffic_light_t get_traffic_light(recog_arg *arg) {
    struct TrafficLights detected = detectLights(arg);
    int                  result   = TL_NONE;

    if (detected.green) result |= TL_GREEN;
    if (detected.yellow) result |= TL_YELLOW;
    if (detected.left) result |= TL_LEFT;
    if (detected.red) result |= TL_RED;

    return (recog_traffic_light_t)result;
}