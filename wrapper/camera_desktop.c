/*
<package name="Camera_Desktop">
	<target n="win32">
		<dll n="opencv/build/x86/vc10/bin/msvcp100.dll"/>
		<dll n="opencv/build/x86/vc10/bin/msvcr100.dll"/>
		<dll n="opencv/build/x86/vc10/bin/opencv_core249.dll"/>
		<dll n="opencv/build/x86/vc10/bin/opencv_highgui249.dll"/>
		<lib n="opencv/build/x86/vc10/lib/opencv_core249.lib"/>
		<lib n="opencv/build/x86/vc10/lib/opencv_highgui249.lib"/>
	</target>
	<target n="win64">
		<dll n="opencv/build/x64/vc10/bin/msvcp100.dll"/>
		<dll n="opencv/build/x64/vc10/bin/msvcr100.dll"/>
		<dll n="opencv/build/x64/vc10/bin/opencv_core249.dll"/>
		<dll n="opencv/build/x64/vc10/bin/opencv_highgui249.dll"/>
		<lib n="opencv/build/x64/vc10/lib/opencv_core249.lib"/>
		<lib n="opencv/build/x64/vc10/lib/opencv_highgui249.lib"/>
	</target>
</package>
*/
#include <string.h>
#include <malloc.h>
#include "wrapper_defines.h"
#define MAX_CAMERAS 8
#define CV_CAP_PROP_FRAME_WIDTH    3
#define CV_CAP_PROP_FRAME_HEIGHT   4
#define CV_CAP_PROP_FPS            5

typedef struct _IplImage{
	int	 nSize;				/* sizeof(IplImage) */
	int	 ID;				/* version (=0)*/
	int	 nChannels;			/* Most of OpenCV functions support 1,2,3 or 4 channels */
	int	 alphaChannel;		/* Ignored by OpenCV */
	int	 depth;				/* Pixel depth in bits: IPL_DEPTH_8U, IPL_DEPTH_8S, IPL_DEPTH_16S,
							   IPL_DEPTH_32S, IPL_DEPTH_32F and IPL_DEPTH_64F are supported.  */
	char colorModel[4];		/* Ignored by OpenCV */
	char channelSeq[4];		/* ditto */
	int	 dataOrder;			/* 0 - interleaved color channels, 1 - separate color channels.
							   cvCreateImage can only create interleaved images */
	int	 origin;			/* 0 - top-left origin,
							   1 - bottom-left origin (Windows bitmaps style).	*/
	int	 align;				/* Alignment of image rows (4 or 8).
							   OpenCV ignores it and uses widthStep instead.	*/
	int	 width;				/* Image width in pixels.							*/
	int	 height;			/* Image height in pixels.							*/
	void*roi;	 /* Image ROI. If NULL, the whole image is selected. */
	void*maskROI;	   /* Must be NULL. */
	void  *imageId;					/* "		   " */
	void*tileInfo;	/* "		   " */
	int	 imageSize;			/* Image data size in bytes
							   (==image->height*image->widthStep
							   in case of interleaved data)*/
	char *imageData;		/* Pointer to aligned image data.		  */
	int	 widthStep;			/* Size of aligned image row in bytes.	  */
	int	BorderMode[4];		/* Ignored by OpenCV.					  */
	int	BorderConst[4];	/* Ditto.								  */
	char *imageDataOrigin;	/* Pointer to very origin of image data
							   (not necessarily aligned) -
							   needed for correct deallocation */
}IplImage;

typedef void* PCvCapture;
typedef IplImage* PIplImage;

extern PCvCapture __cdecl cvCreateCameraCapture( int index );
extern PIplImage __cdecl cvQueryFrame( PCvCapture capture );
extern void __cdecl cvReleaseCapture( PCvCapture* capture );
extern int __cdecl cvSetCaptureProperty( PCvCapture capture, int property_id, double value );

typedef unsigned char u8;
static PCvCapture g_capture_handles[MAX_CAMERAS]={NULL};
static int* g_the_image=NULL;
static int g_buf_w=0,g_buf_h=0;
EXPORT int* osal_GetCameraImage(int cam_id, int* pw,int* ph){
	if((unsigned int)cam_id>=MAX_CAMERAS||!g_capture_handles[cam_id])return NULL;
	{
		PCvCapture cvcap=g_capture_handles[cam_id];
		PIplImage img=cvQueryFrame(cvcap);
		if(!img)return NULL;
		if(g_buf_w!=img->width||g_buf_h!=img->height){
			if(g_the_image){free(g_the_image);g_the_image=NULL;}
			g_buf_w=img->width;
			g_buf_h=img->height;
		}
		if(!g_the_image){g_the_image=(int*)malloc(g_buf_w*g_buf_h*sizeof(int));}
		*pw=img->width;
		*ph=img->height;
		{
			u8* src=img->imageData;
			int* tar=g_the_image;
			int w=g_buf_w;
			int h=g_buf_h;
			int sz=w*h;
			int i;
			 for(i=0;i<sz;i++){
				*tar=0xff000000+((int)src[0]<<16)+((int)src[1]<<8)+(int)src[2];
				src+=3;
				tar++;
			}
		}
		//return img->imageData;
		return g_the_image;
	}
}
EXPORT int osal_TurnOnCamera(int cam_id,int w,int h,int fps){
	if((unsigned int)cam_id>=MAX_CAMERAS)return 0;
	if(g_capture_handles[cam_id])return 1;
	g_capture_handles[cam_id]=cvCreateCameraCapture(cam_id);
	if(!g_capture_handles[cam_id])return 0;
	if(w){cvSetCaptureProperty(g_capture_handles[cam_id],CV_CAP_PROP_FRAME_WIDTH,(double)w);}
	if(h){cvSetCaptureProperty(g_capture_handles[cam_id],CV_CAP_PROP_FRAME_HEIGHT,(double)h);}
	if(fps){cvSetCaptureProperty(g_capture_handles[cam_id],CV_CAP_PROP_FPS,(double)fps);}
	return 1;
}
EXPORT int osal_TurnOffCamera(int cam_id){
	if((unsigned int)cam_id>=MAX_CAMERAS)return 0;
	if(!g_capture_handles[cam_id])return 1;
	cvReleaseCapture(&g_capture_handles[cam_id]);
	g_capture_handles[cam_id]=NULL;
	return 1;
}
EXPORT int osal_GetFrontCameraId(){
	return 0;
}
EXPORT int osal_GetBackCameraId(){
	return -1;
}
