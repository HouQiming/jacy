#include <stdlib.h>
#include <emscripten.h>
#include "wrapper_defines.h"
#include "SDL.h"

typedef unsigned int u32;
static int g_camera_is_on=0,g_w=0,g_h=0,g_image_ready=0;
static u32* g_camera_buffer=NULL;
EXPORT unsigned char* cam_get_buffer_pointer(){
	return (unsigned char*)g_camera_buffer;
}

EXPORT void cam_callback(){
	//ignore camera id, just send something
	SDL_Event a;
	memset(&a,0,sizeof(a));
	a.type=SDL_USEREVENT;
	a.user.code=3;
	SDL_PushEvent(&a);
	g_image_ready=1;
}

EXPORT int osal_TurnOnCamera(int cam_id,int w,int h,int fps){
	if(g_camera_is_on){
		return 1;
	}
	g_camera_is_on=1;
	g_camera_buffer=(u32*)calloc(sizeof(u32),w*h);
	g_w=w;g_h=h;
	int ret=EM_ASM_INT({
		try{
			var data_canvas = document.getElementById('webcam_data');
			data_canvas.width = $0;
			data_canvas.height = $1;
			var img_width=$0;
			var img_height=$1;
			
			var video = document.getElementById('webcam_video');
			
			navigator.getMedia = ( navigator.getUserMedia ||
			               navigator.webkitGetUserMedia ||
			               navigator.mozGetUserMedia ||
			               navigator.msGetUserMedia);
			
			navigator.getMedia( { video: true, audio: false },
			  function(stream) {
			    if (navigator.mozGetUserMedia) {
			      video.mozSrcObject = stream;
			    } else {
			      var vendorURL = window.URL || window.webkitURL;
			      video.src = vendorURL.createObjectURL(stream);
			    }
			    video.play();
			  },function(err) {
			    console.log("An error occured! " + err);
			    window.alert("This page needs a camera to work, please connect one and refresh the page. If you're sure you have a camera, please enable it for this website.");
			  }
			);
			
			var ctx = data_canvas.getContext('2d');
			var ptr = Module['cwrap']('cam_get_buffer_pointer', 'number', [])();
			var FPS = $2;
			
			function get_image() {
			  try {
			    ctx.drawImage(video, 0, 0, video.videoWidth, video.videoHeight, 0, 0, img_width, img_height);
			    var data = ctx.getImageData(0, 0, img_width, img_height).data;
			    var src_off = 0;
			    var dest = ptr|0;
			    for(var i = 0, l = (img_width * img_height*4)|0; i < l; ++i) {
			      HEAP8[dest++] = (data[src_off++]&255);
			    }
			    Module['cwrap']('cam_callback', null, [])();
			  } catch(e) { }
			  setTimeout(get_image, 1.0/FPS);
			}
			video.addEventListener('canplay', function() {
			  setTimeout(get_image, 1.0/FPS);
			});
			return 1;
		}catch(e){
			return 0;
		}
	},w,h,fps);
	return ret;
}

EXPORT int osal_TurnOffCamera(int cam_id){
	if(!g_camera_is_on){return 1;}
	return 0;
}

EXPORT int osal_GetFrontCameraId(){
	return 0;
}

EXPORT int osal_GetBackCameraId(){
	return 0;
}

EXPORT u32* osal_GetCameraImage(int cam_id, int* pw,int* ph){
	if(!g_image_ready)return NULL;
	*pw=g_w;
	*ph=g_h;
	return g_camera_buffer;
}
