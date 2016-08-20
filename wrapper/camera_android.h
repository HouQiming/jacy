#ifndef _CAMERA_ANDROID_H_
#define _CAMERA_ANDROID_H_
#include "sdl.h"
#include <jni.h>
#include <math.h>
#include <string.h>
#include <android/log.h>
#include "wrapper_defines.h"
#include "camera_android.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef struct _TCamera{
	int m_inited;
	jobject m_cam_object;
	////////
	int m_is_on;
	SDL_mutex* m_cam_mutex;
	////////
	u32* m_image_front;
	u32* m_image_back;
	int m_w,m_h,m_image_ready;
	int m_android_texid;
	int m_android_is_texid_valid;

	float m_ev_luminance;
	double m_expected_EV;
}TCamera;

#endif 