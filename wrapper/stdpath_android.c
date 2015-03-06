/*
<package name="stdpaths_android">
	<target n="android">
	</target>
</package>
*/
#include "sdl.h"
#include <jni.h>
#include <android/log.h>
#include <string.h>
#include "wrapper_defines.h"

static char* g_the_path=NULL;

EXPORT char* osal_getStoragePath(){
	if(!g_the_path){
		JNIEnv* env=(JNIEnv*)SDL_AndroidGetJNIEnv();
		jobject activity=(jobject)SDL_AndroidGetActivity();
		jclass activity_class=(*env)->GetObjectClass(env,activity);
		jmethodID activity_class_getFilesDir=(*env)->GetMethodID(env,activity_class,"getFilesDir","()Ljava/io/File;");
		jobject filedir=(*env)->CallObjectMethod(env,activity,activity_class_getFilesDir);
		jclass file_class=(*env)->GetObjectClass(env,filedir);
		jmethodID file_class_tostring=(*env)->GetMethodID(env,file_class,"toString","()Ljava/lang/String;");
		jobject str_fname=(*env)->CallObjectMethod(env,filedir,file_class_tostring);
		//int _dum2=__android_log_print(ANDROID_LOG_ERROR,"STDOUT","str_fname=%08x",str_fname);
		const char* pchar_fname=(*env)->GetStringUTFChars(env,str_fname,NULL);
		g_the_path=strdup(pchar_fname);
	}
	return g_the_path;
}
