#ifndef __OSAL_CAMERA_H
#define __OSAL_CAMERA_H

int* osal_GetCameraImage(int cam_id, int* pw,int* ph);
int osal_TurnOnCamera(int cam_id,int w,int h,int fps);
int osal_TurnOffCamera(int cam_id);
int osal_GetFrontCameraId();
int osal_GetBackCameraId();

int osal_IOSAutoAdjustCameraExposure(int cam_id,int face,int* random_points);

#if defined(ANDROID)||defined(__ANDROID__)
	void osal_AndroidSetCameraPreviewTexture(int cam_id,int texid);
	void osal_AndroidCallUpdateTexImage(int cam_id);
	int osal_AndroidAutoAdjustCameraExposure(int cam_id,int face,int* random_points);
	int osal_AndroidSetNV21Mode(int mode);
#endif

#endif
