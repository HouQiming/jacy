
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


static double MAX_ISO, MIN_ISO, MAX_DURATION, MIN_DURATION, target_iso = -1;
static double MAX_M_ISO, MIN_M_ISO, MAX_M_DURATION, MIN_M_DURATION, target_duration = -1;
static float MIN_T_INV = -1;

@implementation ExposureManager
- (void)initializeCamera:(AVCaptureDevice *)device {
	// lock configuration
	NSError * error = nil;
	CGPoint pointOfInterest = CGPointMake(0.5, 0.5); // initialize to focus in the center
	if ([device lockForConfiguration: &error]) {
		// config focus mode
		// config point
		if ([device isFocusPointOfInterestSupported]) {
			[device setFocusPointOfInterest: pointOfInterest];
		}
		if ([device isFocusModeSupported:AVCaptureFocusModeContinuousAutoFocus]) {
			[device setFocusMode:AVCaptureFocusModeContinuousAutoFocus];
		}
		else if ([device isFocusModeSupported:AVCaptureFocusModeAutoFocus]) {
			[device setFocusMode:AVCaptureFocusModeAutoFocus];
		}
		else if ([device isFocusModeSupported:AVCaptureFocusModeLocked]) {
			[device setFocusMode:AVCaptureFocusModeLocked];
		}
		// config auto focus restriction
		// asume the face is near the camera
		
		if ([device isAutoFocusRangeRestrictionSupported]) {
			device.autoFocusRangeRestriction = AVCaptureAutoFocusRangeRestrictionNear;
		}
		
		// config exposure mode
		// config exposure point
		if ([device isExposurePointOfInterestSupported]) {
			[device setExposurePointOfInterest: pointOfInterest];
		}
		if ([device isExposureModeSupported:AVCaptureExposureModeContinuousAutoExposure]) {
			[device setExposureMode:AVCaptureExposureModeContinuousAutoExposure];
		}
		else if ([device isExposureModeSupported:AVCaptureExposureModeAutoExpose]) {
			[device setExposureMode:AVCaptureExposureModeAutoExpose];
		}
		else if ([device isExposureModeSupported:AVCaptureExposureModeLocked]) {
			[device setExposureMode:AVCaptureExposureModeLocked];
		}

		// config whitebalance mode
		if ([device isWhiteBalanceModeSupported:AVCaptureWhiteBalanceModeContinuousAutoWhiteBalance]) {
			[device setWhiteBalanceMode:AVCaptureWhiteBalanceModeContinuousAutoWhiteBalance];
		}
		else if ([device isWhiteBalanceModeSupported:AVCaptureWhiteBalanceModeAutoWhiteBalance]) {
			[device setWhiteBalanceMode:AVCaptureWhiteBalanceModeAutoWhiteBalance];
		}
		else if ([device isWhiteBalanceModeSupported:AVCaptureWhiteBalanceModeLocked]) {
			[device setWhiteBalanceMode:AVCaptureWhiteBalanceModeLocked];
		}

		// turn on the low light boost
		if (device.isLowLightBoostSupported) {
			device.automaticallyEnablesLowLightBoostWhenAvailable = YES;
		}
		
		// unlock configuration
		[device unlockForConfiguration];
	}
	else {
		// process error
	}
	return;
}

- (void)initializeVar {
	target_iso = -1;
	target_duration = -1;
	return;
}

- (void)getSuitableTandISO:(TCamera *)cam {
	if (cam->m_expect_T <= 0) return;
	// set Range
	MAX_ISO=cam->m_device.activeFormat.maxISO;
	MIN_ISO=cam->m_device.activeFormat.minISO;
	MAX_DURATION=CMTimeGetSeconds(cam->m_device.activeFormat.maxExposureDuration);
	MIN_DURATION=CMTimeGetSeconds(cam->m_device.activeFormat.minExposureDuration);
	if (MAX_DURATION>0.10) MAX_DURATION=0.10;
	if (MIN_DURATION<0.000001) MIN_DURATION=0.000001;
	if (MIN_T_INV < 0) {
		MIN_T_INV = (10.0 / MIN_DURATION);
	}
	// the small range
	MAX_M_ISO=MAX_ISO;
	MIN_M_ISO=MIN_ISO;
	MAX_M_DURATION=MAX_DURATION;
	MIN_M_DURATION=MIN_DURATION;
	if (MAX_M_ISO > 1000) MAX_M_ISO=1000;
	if (MAX_M_DURATION > 0.06) MAX_M_DURATION=0.06;
	
	// 1. set T as 0.04s
	double newT=CMTimeGetSeconds(cam->m_device.exposureDuration);
	if (newT > 0.04) newT = 0.04;
	double newISO;
	double scale = cam->m_expect_T / newT;
	// 2. calculate ISO and T
	newISO = cam->m_device.ISO * scale;
	if (newISO > MAX_M_ISO || newISO < MIN_M_ISO) {
		if (newISO > MAX_M_ISO) {
			// too large
			newISO = MAX_M_ISO;
			newT = newT * (scale * cam->m_device.ISO / newISO);	
			if (newT>MAX_M_DURATION) {
				newT=MAX_M_DURATION;
				scale=cam->m_expect_T/newT;
				newISO=cam->m_device.ISO*scale;
				if (newISO > MAX_ISO) {
					newISO = MAX_ISO;
					newT = newT * (scale / (newISO / cam->m_device.ISO));
					if (newT > MAX_DURATION) {
						newT = MAX_DURATION;
					}
				}
			}
		}
		else {
			// too small
			newISO = MIN_ISO;
			newT = newT * (scale * cam->m_device.ISO / newISO);
			if (newT < MIN_DURATION) {
				newT = MIN_DURATION;
			}
		}
	}

	// set target iso and duration
	target_iso = newISO;
	target_duration = newT;
	return;
}

- (void)setTandISO:(TCamera *)cam {
	if (target_iso < 0 || target_duration < 0) return;
	///////////////
	double newT = target_duration;
	double newISO = target_iso;
	double nowT = CMTimeGetSeconds(cam->m_device.exposureDuration);
	double nowISO = cam->m_device.ISO;
	// set new Duration and ISO
	newT = (float)newT * (int)MIN_T_INV + 0.5;
	CMTime newDuration = CMTimeMake((int)newT, (int)MIN_T_INV);
	NSError* error = nil;
	if ([cam->m_device lockForConfiguration:&error]) {
		[cam->m_device setExposureModeCustomWithDuration: newDuration ISO: newISO completionHandler:nil];
		[cam->m_device unlockForConfiguration];
	}
	else {
		return;
	}
	return;
}

@end