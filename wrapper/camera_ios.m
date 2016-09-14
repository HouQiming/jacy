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
#import <math.h>
#include <stdlib.h>
#include "wrapper_defines.h"
#include "SDL.h"
#include "TargetConditionals.h"
#include "camera_ios.h"
#include "ios_exposure_manager.h"

#define IOS_CAMERA_FRONT 0
#define IOS_CAMERA_BACK 1

static TCamera g_cameras[2]={{0}};
static float OptimalLuminance = 100;
static ExposureManager * exposure_manager;
static int luminance_sample_points[80001]; 
static int frame_count = 0;

EXPORT int osal_IOSAutoAdjustCameraExposure(int cam_id, int face, int* random_points){
	if(cam_id!=IOS_CAMERA_FRONT&&cam_id!=IOS_CAMERA_BACK){return -1;}
	TCamera* cam=&g_cameras[cam_id];
	if(!cam->m_is_on){return -1;}

	int i;
	for (i=0;random_points[i]>=0;i++){
		luminance_sample_points[i]=random_points[i];
	}
	luminance_sample_points[i++]=-1;

	return face;
}

EXPORT int osal_TurnOnCamera(int cam_id,int w,int h,int fps){
	if(cam_id!=IOS_CAMERA_FRONT&&cam_id!=IOS_CAMERA_BACK){return 0;}
	TCamera* cam=&g_cameras[cam_id];
	if(cam->m_is_on){return 1;}
	//////////////////

	exposure_manager = [[ExposureManager alloc]init];
	[exposure_manager initializeVar];
	luminance_sample_points[0]=-1;
	frame_count = 0;

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
		}else if(h<1080){
			preset=AVCaptureSessionPresetHigh;
		}else{
			preset=AVCaptureSessionPresetPhoto;
		}
	}
	preset=AVCaptureSessionPresetPhoto;//todo
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

		cam->m_expect_T = -1;
	}
	if(!cam->m_session){
		[exposure_manager initializeCamera: cam->m_device];
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
		memcpy(tar, src, w * 4);
	}

	double ev_luminance = 0.0;
	int luminance_count = 0;

	if (luminance_sample_points[0]>=0) {
		// face detected
		for(int s=0;luminance_sample_points[s]>=0;s+=2){
			int j=luminance_sample_points[s];
			int i=luminance_sample_points[s+1];
			if ((i>=0&&i<h)&&(j>=0&&j<w)){
				int* src=pimg+(stride>>2)*i;
				u32 bgra=src[j];
				u32 r = ((bgra>>16)&0xffu);
				u32 g = ((bgra>>8)&0xffu);
				u32 b = ((bgra)&0xffu);
				u32 l = (r + r + r + g + g + g + g + b)>>3;
				ev_luminance += l;
				luminance_count ++;	
			}	
		}
		if (luminance_count > 0)
			ev_luminance /= (double)luminance_count;
	}
	else {
		// no face deteceted
		// lock in the center of the screen
		int target_half_w=w/4;
		int target_half_h=h/4;
		for(int i=h/2-target_half_h;i<h/2+target_half_h;i++){
			int* src=pimg+(stride>>2)*i;
			for(int j=w/2-target_half_w;j<w/2+target_half_w;j++){
				u32 bgra=src[j];
				u32 r = ((bgra>>16)&0xffu);
				u32 g = ((bgra>>8)&0xffu);
				u32 b = ((bgra)&0xffu);
				u32 l = (r + r + r + g + g + g + g + b)>>3;
				ev_luminance += l;
				luminance_count ++;
			}
		}

		if (luminance_count > 0)
			ev_luminance /= (double)luminance_count;
	}
	// calculate luminance and expected exposure duration
	frame_count = (frame_count + 1) % 5;
	if (frame_count == 0) {
		cam->m_ev_luminance = ev_luminance;
		if (cam->m_ev_luminance > 1) {
			CMTime duration = cam->m_device.exposureDuration;
			double dura = CMTimeGetSeconds(duration);
			int target_luminance = 90;
			double delta_log = log(cam->m_ev_luminance) - log(target_luminance);
			double expect_log_t = log(dura) - delta_log;
			cam->m_expect_T = exp(expect_log_t);
			[exposure_manager getSuitableTandISO: cam];
		}
	}
	[exposure_manager setTandISO: cam];
	////////////////
	SDL_LockMutex(cam->m_cam_mutex);
	cam->m_image_ready=1;
	SDL_UnlockMutex(cam->m_cam_mutex);
	CVPixelBufferUnlockBaseAddress(imageBuffer,0);
	SDL_UnlockMutex(cam->m_cam_mutex_2);
	{
		//ignore camera id, just send something
		SDL_Event a;
		memset(&a,0,sizeof(a));
		a.type=SDL_USEREVENT;
		a.user.code=3;
		SDL_PushEvent(&a);
	}
	//CVImageBufferRelease(imageBuffer);
}
@end

EXPORT int* osal_GetCameraImage(int cam_id, int* pw,int* ph){
	if(cam_id!=IOS_CAMERA_FRONT&&cam_id!=IOS_CAMERA_BACK){return NULL;}
	TCamera* cam=&g_cameras[cam_id];
	SDL_LockMutex(cam->m_cam_mutex);
	if(cam->m_image_ready){
		u32* ret=cam->m_image_back;
		cam->m_image_back=cam->m_image_front;
		cam->m_image_front=ret;
		cam->m_image_ready=0;
		SDL_UnlockMutex(cam->m_cam_mutex);
		*pw=cam->m_w;
		*ph=cam->m_h;
		return ret;
	}else{
		SDL_UnlockMutex(cam->m_cam_mutex);
		return NULL;
	}
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
