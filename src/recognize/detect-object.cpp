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

cv::Mat maskImage(cv::Mat &frame, int h, int error, int sMin, int vMin) {
    cv::Mat hsvImage;
    cv::cvtColor(frame, hsvImage, cv::COLOR_BGR2HSV);
    int lowH  = (h - error >= 0) ? h - error : h - error + 180;
    int highH = (h + error <= 180) ? h + error : h + error - 180;

    std::vector<cv::Mat> channels;
    cv::split(hsvImage, channels);
    if (lowH < highH)
        cv::bitwise_and(lowH <= channels[0], channels[0] <= highH, channels[0]);
    else
        cv::bitwise_or(lowH <= channels[0], channels[0] <= highH, channels[0]);

    channels[1] = channels[1] > sMin;
    channels[2] = channels[2] > vMin;

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

        if (leftCount > rightCount) return Left;
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

TrafficLights detectLights(cv::Mat &frame, cv::Mat *drawBoard, int minArea,
                           int maxArea) {
    cv::Mat redMasked    = maskImage(frame, 0, 15, 90, 60);
    cv::Mat yellowMasked = maskImage(frame, 30, 20, 120, 60);
    cv::Mat greenMasked  = maskImage(frame, 100, 45, 90, 40);
    cv::Mat greenInverse = 255 - greenMasked;

    const static std::string captions[] = {"Red Light!", "Yellow Light!",
                                           "Green Light!", "Left Direction!",
                                           "Right Direction!"};
    const static cv::Scalar  colors[]   = {
        cv::Scalar(0, 0, 255), cv::Scalar(131, 232, 252), cv::Scalar(0, 255, 0),
        cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 0)};

    std::vector<Contour> found[] = {
        findShapes(Circle, redMasked, minArea, maxArea),
        findShapes(Circle, yellowMasked, minArea, maxArea),
        findShapes(Circle, greenMasked, minArea, maxArea),
        findShapes(Left, greenInverse, minArea, maxArea),
        findShapes(Right, greenInverse, minArea, maxArea)};

    if (drawBoard) {
        for (int i = 0; i < 5; ++i) {
            if (!found[i].empty()) {
                cv::drawContours(*drawBoard, found[i], -1, colors[i], 2);
                putTextAtCenter(*drawBoard, captions[i], colors[i]);
            }
        }
    }

    TrafficLights result;
    result.red    = !found[0].empty();
    result.yellow = !found[1].empty();
    result.green  = !found[2].empty();
    result.left   = !found[3].empty();
    result.right  = !found[4].empty();
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
        result = detectLights(srcRGB, &dstRGB, 50, 100000);
    } else
        result = detectLights(srcRGB, NULL, 50, 100000);

    return result;
}

struct StopObstacle detectStopObstacle(cv::Mat &frame, cv::Mat *drawBoard,
                                       int minArea, int maxArea) {
    cv::Mat              redMasked = maskImage(frame, 0, 15, 90, 60);
    std::vector<Contour> rectFound =
        findShapes(Rectangle, redMasked, minArea, maxArea);
    std::vector<Contour> circFound =
        findShapes(Circle, redMasked, minArea, maxArea);

    for (int r = 0; r < (int)rectFound.size(); ++r) {
        for (int c1 = 0; c1 < (int)circFound.size(); ++c1) {
            for (int c2 = 0; c2 < (int)circFound.size(); ++c2) {
                Contour &rect = rectFound[r];
                Contour &cir1 = circFound[c1];
                Contour &cir2 = circFound[c2];

                if (cv::pointPolygonTest(rect, cir1[0], false) > 0 &&
                    cv::pointPolygonTest(cir1, cir2[0], false) > 0) {
                    if (drawBoard) {
                        std::vector<Contour> toDraw;
                        toDraw.push_back(rect);
                        cv::drawContours(*drawBoard, toDraw, -1,
                                         cv::Scalar(0, 0, 255), 2);
                        putTextAtCenter(*drawBoard, "Stop!",
                                        cv::Scalar(0, 0, 255));
                    }

                    StopObstacle result;
                    result.exist  = true;
                    cv::Moments m = cv::moments(rect);
                    result.area   = (float)m.m00;
                    result.center = {(int)(m.m10 / m.m00),
                                     (int)(m.m01 / m.m00)};
                    return result;
                }
            }
        }
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
        result = detectStopObstacle(srcRGB, &dstRGB, 50, 100000);
    } else
        result = detectStopObstacle(srcRGB, NULL, 50, 100000);

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

    if (detected.green) return TL_GREEN;
    if (detected.yellow) return TL_YELLOW;
    if (detected.left) return TL_LEFT;
    if (detected.red) return TL_RED;

    return TL_NONE;
}