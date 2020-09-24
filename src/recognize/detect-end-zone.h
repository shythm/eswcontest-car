#ifndef DETECT_DETECT_END_ZONE_H_
#define DETECT_DETECT_END_ZONE_H_

#ifdef __cplusplus
extern "C" {
#endif

bool detectEndZone(unsigned char *cam_data, int cam_w, int cam_h,
                   unsigned char *disp_data, int disp_w, int disp_h);

#ifdef __cplusplus
}
#endif

#endif