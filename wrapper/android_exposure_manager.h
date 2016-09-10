#ifndef _ANDROID_EXPOSURE_MANAGER_H_
#define _ANDROID_EXPOSURE_MANAGER_H_

#include "camera_android.h"

int setCompensation(TCamera *cam, double delta_EV);
void imageEnhancementByLinear(u8 * src, u8 * dst, int w, int h);
void process_luminance(int *, u8* out, u8* img_nv21, TCamera *cam, int w, int h, float total_luminance);

#endif