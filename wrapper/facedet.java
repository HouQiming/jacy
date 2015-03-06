package com.spap.wrapper;

import android.graphics.Bitmap;
import android.graphics.PointF;
import android.media.FaceDetector;
import android.util.Log;

public class facedet{
	public static FaceDetector fd;
	public static int m_w;
	public static int m_h;
	public static int m_max_faces;
	public static Bitmap m_bmp;
	public static FaceDetector.Face[] faces;
	public static final int run(int[] img,int w,int h,int[] pfaces,int max_faces){
		if(m_w!=w||m_h!=h||m_max_faces!=max_faces||fd==null){
			fd=new FaceDetector(w,h,max_faces);
			m_w=w;
			m_h=h;
			m_max_faces=max_faces;
			m_bmp=null;
			faces=new FaceDetector.Face[max_faces];
		}
		if(m_bmp==null){
			m_bmp=Bitmap.createBitmap(w,h,Bitmap.Config.RGB_565);
		}
		m_bmp.setPixels(img,0,w,0,0,w,h);
		int n=fd.findFaces(m_bmp,faces);
		//if(n>0)Log.v("STDOUT","findFaces returns n="+n+" img[0]="+img[0]+" max_faces="+max_faces);
		if(n==0)return 0;
		if(n>max_faces)n=max_faces;
		int i;
		for(i=0;i<n;i++){
			PointF pt=new PointF();
			faces[i].getMidPoint(pt);
			float r=faces[i].eyesDistance()*1.8f;
			pfaces[i*4+0]=(int)(pt.x-r);
			pfaces[i*4+1]=(int)((float)(h-1)-(pt.y+r));
			pfaces[i*4+2]=(int)(pt.x+r);
			pfaces[i*4+3]=(int)((float)(h-1)-(pt.y-r));
		}
		//System.gc();
		return n;
	}
};
