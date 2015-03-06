/*
<package name="SDL Image">
	<target n="win32">
		<dll n="lib/x86/libjpeg-9.dll"/>
		<dll n="lib/x86/libpng16-16.dll"/>
		<dll n="lib/x86/libtiff-5.dll"/>
		<dll n="lib/x86/libwebp-4.dll"/>
		<dll n="lib/x86/sdl2.dll"/>
		<dll n="lib/x86/sdl2_image.dll"/>
		<dll n="lib/x86/zlib1.dll"/>
		<lib n="lib/x86/sdl2.lib"/>
		<lib n="lib/x86/sdl2_image.lib"/>
	</target>
	<target n="win64">
		<dll n="lib/x64/libjpeg-9.dll"/>
		<dll n="lib/x64/libpng16-16.dll"/>
		<dll n="lib/x64/libtiff-5.dll"/>
		<dll n="lib/x64/libwebp-4.dll"/>
		<dll n="lib/x64/sdl2.dll"/>
		<dll n="lib/x64/sdl2_image.dll"/>
		<dll n="lib/x64/zlib1.dll"/>
		<lib n="lib/x64/sdl2.lib"/>
		<lib n="lib/x64/sdl2_image.lib"/>
	</target>
</package>
*/
#include "SDL.h"
#include "SDL_Image.h"
#ifdef PM_RELEASE
#define EXPORT
#else
#define EXPORT __declspec(dllexport)
#endif

EXPORT SDL_Surface* SDLW_LoadRGBA8(void* pbase, size_t filesize, 
		void** ppixels,int* pw,int *ph,int* ppitch){
	SDL_RWops* rw=SDL_RWFromMem(pbase, filesize);
	SDL_Surface* ss=IMG_Load_RW(rw,0);
	SDL_Surface* ss2;
	SDL_FreeRW(rw);
	if(!ss){return NULL;}
	ss2=ss;
	ss=SDL_ConvertSurfaceFormat(ss2,SDL_PIXELFORMAT_ABGR8888,SDL_SWSURFACE);
	SDL_FreeSurface(ss2);
	SDL_LockSurface(ss);
	*ppixels=ss->pixels;
	*pw=ss->w;
	*ph=ss->h;
	*ppitch=ss->pitch;
	return ss;
}

EXPORT void SDLW_Free(SDL_Surface* ss){
	SDL_UnlockSurface(ss);
	SDL_FreeSurface(ss);
}

EXPORT void SDLW_SavePNG(char* fn, int* img,int w,int h){
	SDL_Surface* ss=SDL_CreateRGBSurfaceFrom(img,w,h,32,w*sizeof(int),0x0000ff,0x00ff00,0xff0000,0xff000000);
	IMG_SavePNG(ss,fn);
	SDL_FreeSurface(ss);
}
