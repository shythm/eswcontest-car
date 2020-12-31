#include "recognize-lib.h"
#include <algorithm>
#include <numeric>
#include <opencv2/opencv.hpp>

struct Point {
    int x;
    int y;

    int operator-(const Point &p) const { return abs(p.x - x) + abs(p.y - y); }

    bool operator<(const Point &p) const { return x < p.x; }
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

#define LIGHTS_COUNT 4
class CenterKeeper {
  private:
    Point centers[LIGHTS_COUNT] = {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}};

    // centers 집합에 recogPoint를 추가했다면 true
    bool update(const Point &recogPoint) {
        for (int i = 0; i < LIGHTS_COUNT; ++i) {
            if (centers[i].x == -1) {
                centers[i] = recogPoint;
                return true;
            } else if (centers[i] - recogPoint < 10) {
                return false;
            }
        }

        return false;
    }

  public:
    // 주어진 recognized 중 적어도 하나를
    // centers 집합에 추가했다면 true,
    // 그렇지 않다면 false.
    bool update(const std::vector<Point> &recognized) {
        bool added = false;
        for (const Point &p : recognized) {
            if (centers[LIGHTS_COUNT - 1].x ==
                -1) { // centers가 다 채워지지 않은 상태
                added |= update(p);
            }
        }
        return added;
    }

    void clear() {
        for (int i = 0; i < LIGHTS_COUNT; ++i) centers[i].x = centers[i].y = -1;
    }

    bool isReady() {
        // centers 집합에 존재하는 점의 개수가 LIGHTS_COUNT와 같은지 확인
        for (int i = 0; i < LIGHTS_COUNT; ++i)
            if (centers[i].x == -1) return false;

        std::sort(centers, centers + LIGHTS_COUNT);
        int dx = (centers[LIGHTS_COUNT - 1].x - centers[0].x) / 3;
        int dy = (centers[LIGHTS_COUNT - 1].y - centers[0].y) / 3;

        // 모든 점들이 어떤 일차함수 위에 있으며 서로 이웃한 점들끼리
        // 일정한 벡터만큼 떨어져 있는지 확인.
        for (int i = 0; i < LIGHTS_COUNT; ++i) {
            int  predX = dx * i + centers[0].x;
            int  predY = dy * i + centers[0].y;
            bool satisfied =
                predX - 10 < centers[i].x && centers[i].x < predX + 10 &&
                predY - 10 < centers[i].y && centers[i].y < predY + 10;

            if (!satisfied) {
                // 만약 조건을 만족하지 못했다면 애초에 잘못된 점을 받았을 수
                // 있으므로, clear하여 다시 받도록 함.
                clear();
                return false;
            }
        }

        return true;
    }

    Point &operator[](const int index) { return centers[index]; }
};

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

bool isSimilar(const std::vector<Point> &v1, const std::vector<Point> &v2) {
    if (v1.size() != v2.size()) return false;

    for (int i = 0; i < (int)v1.size(); ++i) {
        int dx = abs(v1[i].x - v2[i].x);
        int dy = abs(v1[i].y - v2[i].y);
        if (dx > 10 && dy > 10) return false;
    }

    return true;
}

bool isBlack(cv::Mat &blackMasked, int x, int y, int error = 1) {
    int blackCount = 0;
    for (int i = x - error; i <= x + error; ++i)
        for (int j = y - error; j <= y + error; ++j)
            if (blackMasked.at<char>(j, i)) ++blackCount;

    return 2 * blackCount > (2 * error + 1) * (2 * error + 1);
}

// < 신호등 원의 크기에 대하여 >
// 바퀴가 흰색 정지선에 있을 때: 크기 280정도 나옴.
// 앞바퀴가 흰색 정지선을 완전히 넘었을 때: 300정도 나옴.
// 앞바퀴가 흰색 정지선의 뒤에 있을 때: 250정도 나옴.
recog_traffic_light_t detectLights(cv::Mat &frame, cv::Mat *drawBoard,
                                   int minArea, int maxArea) {
    static std::vector<Point>   prevCenters;
    static int                  hitCount = 0;
    static CenterKeeper         keeper;
    const recog_traffic_light_t ORDER[LIGHTS_COUNT] = {TL_RED, TL_YELLOW,
                                                       TL_LEFT, TL_GREEN};

    recog_traffic_light_t result = TL_NONE;

    cv::Mat              blackMasked = maskImage(frame, 0, 179, 0, 255, 0, 50);
    std::vector<Contour> circles =
        findShapes(Circle, blackMasked, minArea, maxArea);

    if (circles.size() == 3) {
        std::vector<Point> centers(3);

        printf("Saw: ");
        for (int i = 0; i < (int)circles.size(); ++i) {
            cv::Moments m = cv::moments(circles[i]);
            centers[i].x  = m.m10 / m.m00;
            centers[i].y  = m.m01 / m.m00;
            printf("(%d, %d) ", centers[i].x, centers[i].y);
        }
        printf("\n");
        std::sort(centers.begin(), centers.end());

        if (isSimilar(prevCenters, centers)) {
            printf("Similar\n");
            ++hitCount;
            if (hitCount > 10) {
                keeper.update(centers);
                if (keeper.isReady()) {
                    for (int i = 0; i < LIGHTS_COUNT; ++i) {
                        cv::Point c = {keeper[i].x, keeper[i].y};

                        if (!isBlack(blackMasked, c.x, c.y))
                            result = (recog_traffic_light_t)(result | ORDER[i]);
                    }
                    printf("\n");
                }
            }
        } else {
            printf("Not Similar\n");
            hitCount = 0;
        }
        prevCenters = centers;

    } else {
        hitCount = 0;
        prevCenters.clear();
    }

    // for debug
    printf("%s\n", (keeper.isReady() ? "Ready" : "Not Ready"));
    for (int i = 0; i < LIGHTS_COUNT && keeper.isReady(); ++i)
        printf("(%d, %d) ", keeper[i].x, keeper[i].y);
    printf("\n");
    printf("Result: %d\n", (int)result);

    if (drawBoard) { blackMasked.copyTo(*drawBoard); }

    return result;
}

#define IMG_H VPE_OUTPUT_H
#define IMG_W VPE_OUTPUT_W

recog_traffic_light_t detectLights(recog_arg *arg) {
    unsigned char *srcBuf = arg->camera_output;
    unsigned char *outBuf = arg->display_input;

    static unsigned char srcCopied[IMG_H * IMG_W * 3];
    std::copy(srcBuf, srcBuf + IMG_H * IMG_W * 3, srcCopied);
    cv::Mat               srcRGB(IMG_H, IMG_W, CV_8UC3, srcCopied);
    recog_traffic_light_t result;

    if (outBuf) {
        cv::Mat dstRGB(IMG_H, IMG_W, CV_8UC3, outBuf);
        result = detectLights(srcRGB, &dstRGB, 200, 500);
    } else
        result = detectLights(srcRGB, NULL, 200, 500);

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
        result = detectStopObstacle(srcRGB, &dstRGB, 1, 100000);
    } else
        result = detectStopObstacle(srcRGB, NULL, 1, 100000);

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
    return detectLights(arg);
}