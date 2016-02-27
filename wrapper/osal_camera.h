#ifndef __OSAL_CAMERA_H
#define __OSAL_CAMERA_H

int* osal_GetCameraImage(int cam_id, int* pw,int* ph);
int osal_TurnOnCamera(int cam_id,int w,int h,int fps);
int osal_TurnOffCamera(int cam_id);
int osal_GetFrontCameraId();
int osal_GetBackCameraId();

#endif
