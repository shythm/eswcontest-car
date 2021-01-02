#ifndef _RECOGNIZE_H_
#define _RECOGNIZE_H_

#define CAPTURE_IMG_W      1280
#define CAPTURE_IMG_H      720
#define CAPTURE_IMG_SIZE   (CAPTURE_IMG_W * CAPTURE_IMG_H * 2) // YUYU : 16bpp
#define CAPTURE_IMG_FORMAT "uyvy"

#define VPE_OUTPUT_W        320
#define VPE_OUTPUT_H        180
#define VPE_OUTPUT_IMG_SIZE (VPE_OUTPUT_W * VPE_OUTPUT_H * 3)
#define VPE_OUTPUT_FORMAT   "bgr24"

#define OVERLAY_DISP_FORCC FOURCC('A', 'R', '2', '4')
#define OVERLAY_DISP_W     480
#define OVERLAY_DISP_H     272

#include "recognize-shm.h"
#include "util.h"

typedef struct {
    external_data *pext_data;
    unsigned char  camera_output[VPE_OUTPUT_IMG_SIZE];
    unsigned char *display_input;
} recog_arg;

#endif