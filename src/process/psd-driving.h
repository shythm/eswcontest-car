#ifndef __PSD_DRIVING
#define __PSD_DRIVING

#include "ctrlboard-lib.h"
#include "process.h"
#include "recognize-lib.h"

void psd_driving(mqid_ctrl ctrl, recog_result *rr, short velo, float gain_p,
                 int (*callback)());

#endif