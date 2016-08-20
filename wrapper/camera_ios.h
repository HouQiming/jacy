#ifndef _CAMERA_IOS_H_
#define _CAMERA_IOS_H_

#include <stdlib.h>
#include "wrapper_defines.h"
#include "SDL.h"
#include "TargetConditionals.h"

@interface CIOSCamCallBack: NSObject <AVCaptureVideoDataOutputSampleBufferDelegate> {
	int m_cam_id;
}
@end


typedef unsigned char u8;
typedef unsigned int u32;
typedef struct{
	//AVCaptureDeviceInput* m_input;
	int w_prev,h_prev,fps_prev;
	AVCaptureDevice* m_device;
	AVCaptureSession* m_session;
	CIOSCamCallBack* m_callback;
	////////
	int m_is_on;
	SDL_mutex* m_cam_mutex;
	SDL_mutex* m_cam_mutex_2;
	////////
	u32* m_image_front;
	u32* m_image_back;
	int m_w,m_h,m_image_ready;

	float m_ev_luminance;
	double m_expect_T;
}TCamera;


#endif