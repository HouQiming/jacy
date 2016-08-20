package com.spap.wrapper;

import android.hardware.Camera;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.app.*;
import android.content.*;
import android.view.*;
import java.io.IOException;
import android.util.Log;

public class camera implements Camera.PreviewCallback,SurfaceTexture.OnFrameAvailableListener{

	public static native void sendresult(int camid,byte[] a,int w,int h);
	/**
	 * Camera Interface;
	 */
	//static{System.loadLibrary("main");}
	int mCameraId;
	int mNeedUp;
	int mIsTexValid;
	Camera mMyCamera;
	Camera.Size mMyCameraSize;
	SurfaceTexture mSurfaceTexture;
	float mCompensationStep = 0;
	int mMinCompensationIndex, mMaxCompensationIndex, mLastCompensationIndex = 0;
	public void onPreviewFrame(byte[] data,Camera camera){
		if(mIsTexValid==0&&mNeedUp>0){
			try{
				//this shit discards the damn buffer queue on SamSung
				//we must make sure the update fails, though...
				mSurfaceTexture.updateTexImage();
			}catch(Exception e){}
			mNeedUp=0;
		}
		sendresult(mCameraId,data,mMyCameraSize.width,mMyCameraSize.height);
	}
	public int callUpdateTexImage(){
		if(mNeedUp>0){
			//Log.v("STDOUT","callUpdateTexImage");
			try{
				mSurfaceTexture.updateTexImage();
			}catch(Exception e){}
			mNeedUp=0;
			return 1;
		}
		return 0;
	}
	public void onFrameAvailable(SurfaceTexture surfaceTexture){
		mNeedUp=1;
	}

	/**
	 * JNI Interface to operate the camera
	 */
	public int set_compensation(double compensation){
		if (compensation == 0) {
			return 0;
		}
		int index = (int)(compensation / mCompensationStep) + mMyCamera.getParameters().getExposureCompensation();
		if (index > mMaxCompensationIndex) index = mMaxCompensationIndex;
		if (index < mMinCompensationIndex) index = mMinCompensationIndex;
		Camera.Parameters m_params = mMyCamera.getParameters();
		m_params.setExposureCompensation(index);
		mMyCamera.setParameters(m_params);
		return index;
	}
	public int turn_on(int id,int w,int h,int fps,int texid,int is_tex_valid){
		//Log.v("STDOUT","turn_on");
		if(mMyCamera==null){
			//Log.v("STDOUT","Open camera");
			mMyCamera=Camera.open(id);
			mCameraId=id;
			//Log.v("STDOUT","Open camera done");
		}
		if(mMyCamera==null)return 0;
		//Log.v("STDOUT","Open camera successful");
		Camera.Parameters params;
		try{
			if(mSurfaceTexture==null){mSurfaceTexture=new SurfaceTexture(texid);}
			mMyCamera.setPreviewCallback(this);
			//mMyCamera.setPreviewDisplay(null);
			mSurfaceTexture.setOnFrameAvailableListener(this);
			mMyCamera.setPreviewTexture(mSurfaceTexture);
			params=mMyCamera.getParameters();
			params.setPreviewFormat(ImageFormat.NV21);
			//params.setRecordingHint(false);
			if(w!=0&&h!=0){params.setPreviewSize(w,h);}
			if(fps!=0){params.setPreviewFrameRate(fps);}//the new api sucks, use the existing one
			//if(fps!=0){params.setPreviewFpsRange(fps*1000,fps*1000);}
			mMyCamera.setParameters(params);
			mMyCamera.startPreview();
		}catch(Exception e){return 0;}
		params=mMyCamera.getParameters();
		mMyCameraSize=params.getPreviewSize();
		mIsTexValid=is_tex_valid;
		//Log.v("STDOUT","set texture: "+texid+" "+is_tex_valid);
		//Log.v("STDOUT","reached the end");
		// get compensation step
		mCompensationStep = mMyCamera.getParameters().getExposureCompensationStep();
		mMinCompensationIndex = mMyCamera.getParameters().getMinExposureCompensation();
		mMaxCompensationIndex = mMyCamera.getParameters().getMaxExposureCompensation();
		return 1;	
	}

	public int turn_off(){
		if(mMyCamera==null)return 0;
		mMyCamera.stopPreview();
		mMyCamera.release();
		mMyCamera=null;
		return 1;
	}
	static int front_id=-1;
	static int back_id=-1;
	static int cam_scanned;
	public static final void scan_cameras(){
		if(cam_scanned!=0)return;
		cam_scanned=1;
		int n=Camera.getNumberOfCameras();
		for(int i=0;i<n;i++){
			Camera.CameraInfo info=new Camera.CameraInfo();
			Camera.getCameraInfo(i,info);
			if(front_id<0&&info.facing==Camera.CameraInfo.CAMERA_FACING_FRONT){
				front_id=i;
			}
			if(back_id<0&&info.facing==Camera.CameraInfo.CAMERA_FACING_BACK){
				back_id=i;
			}
		}
	}
	public static final int get_front_camera_id(){
		scan_cameras();
		return front_id;
	}
	public static final int get_back_camera_id(){
		scan_cameras();
		return back_id;
	}
	
};
