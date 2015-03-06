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
	//static{System.loadLibrary("main");}
	Camera my_camera;
	SurfaceTexture st;
	int my_id;
	int m_need_up;
	Camera.Size my_size;
	public void onPreviewFrame(byte[] data,Camera camera){
		if(m_need_up>0){
			try{
				//this shit discards the damn buffer queue on SamSung
				//we must make sure the update fails, though...
				st.updateTexImage();
			}catch(Exception e){}
			m_need_up=0;
		}
		sendresult(my_id,data,my_size.width,my_size.height);
	}
	public int turn_on(int id,int w,int h,int fps){
		//Log.v("STDOUT","turn_on");
		if(my_camera==null){
			//Log.v("STDOUT","Open camera");
			my_camera=Camera.open(id);
			my_id=id;
			//Log.v("STDOUT","Open camera done");
		}
		if(my_camera==null)return 0;
		//Log.v("STDOUT","Open camera successful");
		Camera.Parameters params;
		try{
			if(st==null){st=new SurfaceTexture(1);}
			my_camera.setPreviewCallback(this);
			//my_camera.setPreviewDisplay(null);
			st.setOnFrameAvailableListener(this);
			my_camera.setPreviewTexture(st);
			params=my_camera.getParameters();
			params.setPreviewFormat(ImageFormat.NV21);
			if(w!=0&&h!=0){params.setPreviewSize(w,h);}
			if(fps!=0){params.setPreviewFrameRate(fps);}//the new api sucks, use the existing one
			//if(fps!=0){params.setPreviewFpsRange(fps*1000,fps*1000);}
			my_camera.setParameters(params);
			my_camera.startPreview();
		}catch(Exception e){return 0;}
		params=my_camera.getParameters();
		my_size=params.getPreviewSize();
		//Log.v("STDOUT","reached the end");
		return 1;
	}
	public int turn_off(){
		if(my_camera==null)return 0;
		my_camera.stopPreview();
		my_camera.release();
		my_camera=null;
		return 1;
	}
	///////////////////////////////////////////////
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
	////////////////
	public void onFrameAvailable(SurfaceTexture surfaceTexture){
		m_need_up=1;
	}
};
