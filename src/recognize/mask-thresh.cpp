#include "mask-thresh.h"
#include <opencv2/opencv.hpp>

double otsu_8u_without_zero(const cv::Mat1b src) {
    const int N = 256;
    int       M = 0;
    int       i, j, h[N] = {0};
    for (i = 0; i < src.rows; i++) {
        const uchar *psrc = src.ptr(i);
        for (j = 0; j < src.cols; j++) {
            if (psrc[j]) {
                h[psrc[j]]++;
                ++M;
            }
        }
    }

    double mu = 0, scale = 1. / (M);
    for (i = 0; i < N; i++)
        mu += i * (double)h[i];

    mu *= scale;
    double mu1 = 0, q1 = 0;

    double max_sigma = 0, max_val = 0;

    for (i = 0; i < N; i++) {
        double p_i, q2, mu2, sigma;

        p_i = h[i] * scale;
        mu1 *= q1;
        q1 += p_i;
        q2 = 1. - q1;

        if (std::min(q1, q2) < FLT_EPSILON ||
            std::max(q1, q2) > 1. - FLT_EPSILON)
            continue;

        mu1   = (mu1 + i * p_i) / q1;
        mu2   = (mu - q1 * mu1) / q2;
        sigma = q1 * q2 * (mu1 - mu2) * (mu1 - mu2);
        if (sigma > max_sigma) {
            max_sigma = sigma;
            max_val   = i;
        }
    }
    return max_val;
}

double threshold_without_zero(cv::Mat &src, cv::Mat &dst) {
    double thresh = otsu_8u_without_zero(src);
    thresh        = cv::threshold(src, dst, thresh, 255, CV_THRESH_BINARY);
    src.copyTo(dst);
    return thresh;
}