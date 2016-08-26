#include "sdl.h"
#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "wrapper_defines.h"
#include "spaprt_portable.h"

static void* g_res_zip=NULL;
static iptr g_res_size=0;
EXPORT void* osal_mmap_res_zip(iptr* psize){
	if(!g_res_size){
		JNIEnv* env=(JNIEnv*)SDL_AndroidGetJNIEnv();
		jobject activity=(jobject)SDL_AndroidGetActivity();
		jclass activity_class=(*env)->GetObjectClass(env,activity);
		jmethodID activity_class_getAssets=(*env)->GetMethodID(env,activity_class,"getAssets","()Landroid/content/res/AssetManager;");
		jobject asset_manager=(*env)->CallObjectMethod(env,activity,activity_class_getAssets);
		jobject am_object=(*env)->NewGlobalRef(env,asset_manager);
		AAssetManager* am_real=AAssetManager_fromJava(env,am_object);
		AAsset* asset=AAssetManager_open(am_real,"reszip.mp3",AASSET_MODE_BUFFER);
		if(!asset){g_res_size=-1;}else{
			g_res_size=AAsset_getLength(asset);
			g_res_zip=(void*)AAsset_getBuffer(asset);
		}
		//__android_log_print(ANDROID_LOG_ERROR,"STDOUT","asset=%p, g_res_zip=%p, g_res_size=%d",asset,g_res_zip,g_res_size);
	}
	if(g_res_size<0){*psize=0;}else{*psize=g_res_size;}
	return g_res_zip;
}
