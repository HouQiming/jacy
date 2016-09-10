#include "android_exposure_manager.h"


static int average_luminance = 0, luminance_count = 0;
static int frame_count = 0;
int setCompensation(TCamera *cam, double delta_EV) {
	JNIEnv* env=(JNIEnv*)SDL_AndroidGetJNIEnv();
	jmethodID method_id;
	jvalue args[1];
	int ret;
	jclass clazz=(*env)->FindClass(env,"com/spap/wrapper/camera");
	SDL_LockMutex(cam->m_cam_mutex);
	method_id=(*env)->GetMethodID(env,clazz,"set_compensation","(D)I");
	args[0].d=delta_EV;
	ret=(*env)->CallIntMethodA(env,cam->m_cam_object,method_id,args);
	SDL_UnlockMutex(cam->m_cam_mutex);
	return ret;
}

void imageEnhancementByLinear(u8 * src, u8 * dst, int w, int h) {
	u8 dataMax = 0;
	u8 dataMin = 255;
	int len = w * h;
	for (int i = 0; i < len; i++) {
		if (src[i] > dataMax) dataMax = src[i];
		if (src[i] < dataMin) dataMin = src[i];
	}
	double scale = (double)(256 - 1)  / (double)(dataMax - dataMin);
	for (int i = 0; i < len; i++) {
		u8 v = src[i];
		dst[i] = (u8)((v - dataMin) * scale);
	}
	return;
}

void process_luminance(int* luminance_sample_points, u8* out, u8* img_nv21, TCamera *cam, int w, int h, float total_luminance) {
	// calculate the expected EV
	cam->m_ev_luminance = 0;
	cam->m_expected_EV = 0;
	luminance_count = 0;
	if (luminance_sample_points[0]>=0) {
		for (int i = 0; luminance_sample_points[i] >= 0; i+=2) {
			int x = luminance_sample_points[i];
			int y = luminance_sample_points[i + 1];
			if ((y>=0&&y<h)&&(x>=0&&x<w)) {
				cam->m_ev_luminance += out[y * w + x];
				luminance_count++;
			}
		}
	}
	else {
		int target_half_w=w / 4;
		int target_half_h=h / 4;
		for(int i= h / 2 - target_half_h; i < h / 2 + target_half_h; i++){
			for(int j = w / 2 - target_half_w; j < w / 2 + target_half_w; j++){
				cam->m_ev_luminance += out[i * w + j];
				luminance_count++;
			}
		}
	}
	// calculate the compensation
	if (luminance_count > 0) {
		cam->m_ev_luminance = cam->m_ev_luminance / (double)(luminance_count);
	}
	if (cam->m_ev_luminance > 1) {
		int target_luminance = 90;
		double delta_log = log(cam->m_ev_luminance) - log(target_luminance);
		cam->m_expected_EV = -delta_log;
	}
	average_luminance = (int)cam->m_ev_luminance;
	// set Compensation
	frame_count = (frame_count + 1) % 5;
	if (!frame_count) {
		setCompensation(cam, cam->m_expected_EV);
	}

	total_luminance /= (float)(w * h);
	if (total_luminance < 50) {
		imageEnhancementByLinear(img_nv21, out + w * h * 3, w, h);
	}
	else {
		memcpy(out + w * h * 3, out, w * h);
	}
	return;
}