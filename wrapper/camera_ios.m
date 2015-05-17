/*
<package name="Camera_ios">
	<target n="ios">
		<array a="ios_frameworks" n="System/Library/Frameworks/AVFoundation.framework"/>
		<array a="ios_frameworks" n="System/Library/Frameworks/CoreMedia.framework"/>
		<array a="ios_frameworks" n="System/Library/Frameworks/CoreVideo.framework"/>
	</target>
	<target n="mac">
		<array a="mac_frameworks" n="System/Library/Frameworks/AVFoundation.framework"/>
		<array a="mac_frameworks" n="System/Library/Frameworks/CoreMedia.framework"/>
		<array a="mac_frameworks" n="System/Library/Frameworks/CoreVideo.framework"/>
	</target>
</package>
*/
//[[UIDevice currentDevice] orientation]
//#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#include <stdlib.h>
#include "wrapper_defines.h"
#include "SDL.h"
#include "TargetConditionals.h"

#define IOS_CAMERA_FRONT 0
#define IOS_CAMERA_BACK 1

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
	SDL_sem* m_camdat_ready_event;
	////////
	u32* m_image_front;
	u32* m_image_back;
	int m_w,m_h,m_is_waiting,m_image_ready;
}TCamera;

static TCamera g_cameras[2]={{0}};

EXPORT int osal_TurnOnCamera(int cam_id,int w,int h,int fps){
	if(cam_id!=IOS_CAMERA_FRONT&&cam_id!=IOS_CAMERA_BACK){return 0;}
	TCamera* cam=&g_cameras[cam_id];
	if(cam->m_is_on){return 1;}
	//////////////////
	if(!cam->m_cam_mutex){
		cam->m_cam_mutex=SDL_CreateMutex();
		cam->m_cam_mutex_2=SDL_CreateMutex();
		//SDL_UnlockMutex(cam->m_cam_mutex);
		//SDL_UnlockMutex(cam->m_cam_mutex_2);
		//printf("create mutex %p\n",cam->m_cam_mutex_2);
	}
	if(cam->w_prev!=w||cam->h_prev!=h||cam->fps_prev!=fps){
		cam->w_prev=w;cam->h_prev=h;cam->fps_prev=fps;
		if(cam->m_session){
			[cam->m_session release];
		}
		cam->m_session=NULL;
	}
	//////////////////
	NSString* preset=NULL;
	//if(w==320&&h==240){preset=AVCaptureSessionPreset320x240;}
	if(w==352&&h==288){preset=AVCaptureSessionPreset352x288;}
	if(w==640&&h==480){preset=AVCaptureSessionPreset640x480;}
	//if(w==960&&h==540){preset=AVCaptureSessionPreset960x540;}
	if(w==1280&&h==720){preset=AVCaptureSessionPreset1280x720;}
	if(!preset){
		if(h<480){
			preset=AVCaptureSessionPresetLow;
		}else if(h<720){
			preset=AVCaptureSessionPresetMedium;
		}else{
			preset=AVCaptureSessionPresetHigh;
		}
	}
	if(!cam->m_device){
		NSArray *devices = [AVCaptureDevice devices];
		AVCaptureDevice *cam_mine=NULL;
		for (AVCaptureDevice *device in devices) {
			if ([device hasMediaType:AVMediaTypeVideo]) {
				if ([device position] == AVCaptureDevicePositionBack) {
					if(cam_id==IOS_CAMERA_BACK){cam_mine = device;}
				} else {
					if(cam_id==IOS_CAMERA_FRONT){cam_mine = device;}
				}
			}
		}
		if(!cam_mine){return 0;}
		cam->m_device=cam_mine;
		cam->m_callback=[[CIOSCamCallBack alloc] initWithId:cam_id];
	}
	if(!cam->m_session){
		AVCaptureDeviceInput *captureInput=
			[AVCaptureDeviceInput 
				deviceInputWithDevice:cam->m_device
				error:nil];
		AVCaptureVideoDataOutput *captureOutput = [[AVCaptureVideoDataOutput alloc] init];
		CIOSCamCallBack* callback=[[CIOSCamCallBack alloc] init];
		captureOutput.alwaysDiscardsLateVideoFrames = YES;
		#if !TARGET_OS_MAC
			if(fps){captureOutput.minFrameDuration = CMTimeMake(1,fps);}
		#endif
		/*We create a serial queue to handle the processing of our frames*/
		dispatch_queue_t queue;
		queue = dispatch_queue_create("cameraQueue", NULL);
		[captureOutput setSampleBufferDelegate:cam->m_callback queue:queue];
		dispatch_release(queue);
		// Set the video output to store frame in BGRA (It is supposed to be faster)
		NSString* key = (NSString*)kCVPixelBufferPixelFormatTypeKey; 
		NSNumber* value = [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA];
		//NSNumber* value = [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32RGBA];
		NSDictionary* videoSettings = [NSDictionary dictionaryWithObject:value forKey:key];
		[captureOutput setVideoSettings:videoSettings]; 
		/*And we create a capture session*/
		AVCaptureSession* captureSession = [[AVCaptureSession alloc] init];
		/*We add input and output*/
		[captureSession addInput:captureInput];
		[captureSession addOutput:captureOutput];
		/*We use medium quality, ont the iPhone 4 this demo would be laging too much, the conversion in UIImage and CGImage demands too much ressources for a 720p resolution.*/
		cam->m_session=captureSession;
	}
	cam->m_is_on=1;
	[cam->m_session setSessionPreset:preset];
	[cam->m_session startRunning];
	return 1;
}

@implementation CIOSCamCallBack

- (id)initWithId:(int)param_variable_cam_id {
	self->m_cam_id=param_variable_cam_id;
	return self;
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput 
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
	   fromConnection:(AVCaptureConnection *)connection
{
	TCamera* cam=&g_cameras[self->m_cam_id];
	//printf("frame coming...\n");
	if(SDL_TryLockMutex(cam->m_cam_mutex_2)!=0){
		//printf("mutex failure %p\n",cam->m_cam_mutex_2);
		return;
	}
	//NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
	/*Lock the image buffer*/
	CVPixelBufferLockBaseAddress(imageBuffer,0); 
	/*Get information about the image*/
	u32* pimg = (u32 *)CVPixelBufferGetBaseAddress(imageBuffer);
	size_t stride = CVPixelBufferGetBytesPerRow(imageBuffer);
	size_t w = CVPixelBufferGetWidth(imageBuffer);
	size_t h = CVPixelBufferGetHeight(imageBuffer);
	//printf("pimg=%08x\n,",pimg);
	//make use of the image, memcpy out for now
	int need_post=0;
	//__android_log_print(ANDROID_LOG_ERROR,"STDOUT","image coming");
	SDL_LockMutex(cam->m_cam_mutex);
	if(!cam->m_is_on){
		SDL_UnlockMutex(cam->m_cam_mutex);
		SDL_UnlockMutex(cam->m_cam_mutex_2);
		//printf("cam not on\n");
		return;
	}
	if(cam->m_w!=w||cam->m_h!=h){
		if(cam->m_image_back){
			free(cam->m_image_back);
			cam->m_image_back=NULL;
		}
		if(cam->m_image_front){
			free(cam->m_image_front);
			cam->m_image_front=NULL;
		}
		cam->m_w=w;
		cam->m_h=h;
	}
	if(!cam->m_image_back){
		cam->m_image_back=malloc(4*w*h);
	}
	u32* pback=(u32*)cam->m_image_back;
	//int w3=w*3;
	SDL_UnlockMutex(cam->m_cam_mutex);
	//BGRA to RGBA
	//printf("%08x %d %d %08x\n",cam->m_image_back,w,h,pimg);
	for(int i=0;i<h;i++){
		int* tar=pback+w*i;
		int* src=pimg+(stride>>2)*i;
		for(int j=0;j<w;j++){
			u32 bgra=src[j];
			tar[j]=(bgra&0xff00ff00u)+((bgra>>16)&0xffu)+((bgra<<16)&0xff0000u);
		}
	}
	SDL_LockMutex(cam->m_cam_mutex);
	need_post=cam->m_is_waiting;
	cam->m_image_ready=1;
	cam->m_is_waiting=0;
	SDL_UnlockMutex(cam->m_cam_mutex);
	if(need_post){
		SDL_SemPost(cam->m_camdat_ready_event);
	}
	CVPixelBufferUnlockBaseAddress(imageBuffer,0);
	SDL_UnlockMutex(cam->m_cam_mutex_2);
	//CVImageBufferRelease(imageBuffer);
}
@end

EXPORT int* osal_GetCameraImage(int cam_id, int* pw,int* ph){
	if(cam_id!=IOS_CAMERA_FRONT&&cam_id!=IOS_CAMERA_BACK){return NULL;}
	TCamera* cam=&g_cameras[cam_id];
	int* ret=NULL;
	for(;;){
		SDL_LockMutex(cam->m_cam_mutex);
		if(cam->m_image_ready){
			ret=(int*)cam->m_image_back;
			cam->m_image_back=cam->m_image_front;
			cam->m_image_front=(u32*)ret;
			cam->m_image_ready=0;
			SDL_UnlockMutex(cam->m_cam_mutex);
			break;
		}else{
			cam->m_is_waiting=1;
			SDL_UnlockMutex(cam->m_cam_mutex);
			//__android_log_print(ANDROID_LOG_ERROR,"STDOUT","SDL_SemWait(cam->m_camdat_ready_event)");
			SDL_SemWait(cam->m_camdat_ready_event);
		}
	}
	*pw=cam->m_w;
	*ph=cam->m_h;
	return ret;
}

EXPORT int osal_TurnOffCamera(int cam_id){
	if(cam_id!=IOS_CAMERA_FRONT&&cam_id!=IOS_CAMERA_BACK){return 0;}
	TCamera* cam=&g_cameras[cam_id];
	int ret;
	if(!cam->m_is_on)return 1;
	SDL_LockMutex(cam->m_cam_mutex);
	if(cam->m_image_back){
		free(cam->m_image_back);
		cam->m_image_back=NULL;
	}
	if(cam->m_image_front){
		free(cam->m_image_front);
		cam->m_image_front=NULL;
	}
	cam->m_is_on=0;
	SDL_UnlockMutex(cam->m_cam_mutex);
	[cam->m_session stopRunning];
	return ret;
}

EXPORT int osal_GetFrontCameraId(){
	return IOS_CAMERA_FRONT;
}

EXPORT int osal_GetBackCameraId(){
	return IOS_CAMERA_BACK;
}
